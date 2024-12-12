#include "DiffuseBakingDemoRenderer.hpp"

#include <glm/ext.hpp>

#include <stb_image.h>

#include <imgui.h>

#include <etna/GlobalContext.hpp>
#include <etna/PipelineManager.hpp>
#include <etna/RenderTargetStates.hpp>
#include <etna/BlockingTransferHelper.hpp>
#include <etna/Profiling.hpp>


#include "generated/SphereSamples.hpp"
#include "generated/SphereMesh.hpp"
#include "generated/SphericalHarmonicSamples.hpp"


struct HDRImagePixels
{
  std::uint32_t width;
  std::uint32_t height;
  std::vector<glm::vec4> data;
};

static HDRImagePixels loadImageHDR(const std::filesystem::path& path)
{
  int width;
  int height;
  int channels;
  float* dataRaw = stbi_loadf(path.c_str(), &width, &height, &channels, 4);

  if (dataRaw == nullptr)
  {
    return HDRImagePixels{
      .width = 1,
      .height = 1,
      .data = {glm::u8vec4(255, 255, 255, 255)},
    };
  }

  glm::vec4* dataRawTyped = reinterpret_cast<glm::vec4*>(dataRaw);

  std::vector<glm::vec4> data(dataRawTyped, dataRawTyped + width * height);

  stbi_image_free(dataRaw);

  return HDRImagePixels{
    .width = static_cast<std::uint32_t>(width),
    .height = static_cast<std::uint32_t>(height),
    .data = std::move(data),
  };
}

DiffuseBakingDemoRenderer::DiffuseBakingDemoRenderer() {}

void DiffuseBakingDemoRenderer::allocateResources(glm::uvec2 swapchain_resolution)
{
  resolution = swapchain_resolution;

  auto& ctx = etna::get_context();

  mainTarget = ctx.createImage(etna::Image::CreateInfo{
    .extent = vk::Extent3D{resolution.x, resolution.y, 1},
    .name = "main_target",
    .format = vk::Format::eR8G8B8A8Unorm,
    .imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
  });

  cubemap = ctx.createImage(etna::Image::CreateInfo{
    .flags = vk::ImageCreateFlagBits::eCubeCompatible,
    .extent = vk::Extent3D{512, 512, 1},
    .name = "cubemap",
    // .format = vk::Format::eR8G8B8A8Unorm,
    .format = vk::Format::eR32G32B32A32Sfloat,
    .imageUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
    .layers = 6,
  });

  {
    namespace samples = generated::sphere_samples;

    sphereSamples = ctx.createBuffer(etna::Buffer::CreateInfo{
      .size = std::span(samples::SAMPLE_DIRECTIONS).size_bytes(),
      .bufferUsage =
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
      .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
      .name = "sphere_samples",
    });
  }

  diffuseSphericalHarmonics = ctx.createBuffer(etna::Buffer::CreateInfo{
    .size = (generated::spherical_harmonic_samples::MAX_BAND + 1) *
      (generated::spherical_harmonic_samples::MAX_BAND + 1) * sizeof(glm::vec4),
    .bufferUsage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
    .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
    .name = "diffuse_spherical_harmonics",
  });

  defaultSampler = etna::Sampler(etna::Sampler::CreateInfo{
    .name = "default_sampler",
  });
}

void DiffuseBakingDemoRenderer::loadShaders()
{
  etna::create_program(
    "cubemap",
    {
      DIFFUSE_BAKING_SHADERS_ROOT "shaders/cubemap.vert.spv",
      DIFFUSE_BAKING_SHADERS_ROOT "shaders/cubemap.frag.spv",
    });

  etna::create_program(
    "dynamic_diffuse",
    {
      DIFFUSE_BAKING_SHADERS_ROOT "shaders/sphere.vert.spv",
      DIFFUSE_BAKING_SHADERS_ROOT "shaders/shading/dynamic_diffuse.frag.spv",
    });
  etna::create_program(
    "cubemap_diffuse",
    {
      DIFFUSE_BAKING_SHADERS_ROOT "shaders/sphere.vert.spv",
      DIFFUSE_BAKING_SHADERS_ROOT "shaders/shading/cubemap_diffuse.frag.spv",
    });
  etna::create_program(
    "spherical_harmonics_diffuse",
    {
      DIFFUSE_BAKING_SHADERS_ROOT "shaders/sphere.vert.spv",
      DIFFUSE_BAKING_SHADERS_ROOT "shaders/shading/spherical_harmonics_diffuse.frag.spv",
    });

  etna::create_program(
    "scene_blend",
    {
      DIFFUSE_BAKING_SHADERS_ROOT "shaders/scene_blend.vert.spv",
      DIFFUSE_BAKING_SHADERS_ROOT "shaders/scene_blend.frag.spv",
    });
}

void DiffuseBakingDemoRenderer::setupPipelines(vk::Format swapchain_format)
{
  initCubemapPipeline(swapchain_format);

  initDynamicDiffusePipeline();
  initSphericalHarmonicsDiffusePipeline();

  initSceneBlendPipeline(swapchain_format);
}

DiffuseBakingDemoRenderer::ResourcesPrepareContext::ResourcesPrepareContext(
  const etna::Image& cubemap,
  const etna::Buffer& sphere_samples,
  const etna::Buffer& diffuse_spherical_harmonics)
  : cubemap(cubemap)
  , sphereSamples(sphere_samples)
  , diffuseSphericalHarmonics(diffuse_spherical_harmonics)
{
  defaultSampler = etna::Sampler(etna::Sampler::CreateInfo{
    .name = "default_sampler",
  });
}

void DiffuseBakingDemoRenderer::ResourcesPrepareContext::prepareResources()
{
  spdlog::info("Preparing resources...");

  auto& ctx = etna::get_context();

  vk::UniqueCommandPool commandPool =
    etna::unwrap_vk_result(ctx.getDevice().createCommandPoolUnique(vk::CommandPoolCreateInfo{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = ctx.getQueueFamilyIdx(),
    }));

  vk::UniqueCommandBuffer commandBuffer = std::move(etna::unwrap_vk_result(
    ctx.getDevice().allocateCommandBuffersUnique(vk::CommandBufferAllocateInfo{
      .commandPool = commandPool.get(),
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1,
    }))[0]);

  vk::UniqueFence fence =
    etna::unwrap_vk_result(ctx.getDevice().createFenceUnique(vk::FenceCreateInfo{}));

  ETNA_CHECK_VK_RESULT(commandBuffer->begin(vk::CommandBufferBeginInfo{
    .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
  }));

  fillCubemap(commandBuffer.get());
  fillSphereSamples(commandBuffer.get());
  // bakeDiffuseCubemap(commandBuffer.get());
  bakeDiffuseSphericalHarmonics(commandBuffer.get());

  ETNA_CHECK_VK_RESULT(commandBuffer->end());

  vk::CommandBufferSubmitInfo commandBufferSubmitInfo = {
    .commandBuffer = commandBuffer.get(),
    .deviceMask = 1,
  };

  vk::SubmitInfo2 submitInfo = {
    .commandBufferInfoCount = 1,
    .pCommandBufferInfos = &commandBufferSubmitInfo,
  };

  ETNA_CHECK_VK_RESULT(ctx.getQueue().submit2({submitInfo}, fence.get()));
  ETNA_CHECK_VK_RESULT(ctx.getDevice().waitForFences(
    {fence.get()}, vk::True, std::numeric_limits<std::uint64_t>::max()));
}

void DiffuseBakingDemoRenderer::ResourcesPrepareContext::fillCubemap(
  vk::CommandBuffer command_buffer)
{
  static const std::array<const char*, 6> CUBEMAP_TEXTURE_NAMES = {
    "right",
    "left",
    "top",
    "bottom",
    "front",
    "back",
  };

  std::array<HDRImagePixels, 6> faces;
  for (std::size_t i = 0; i < 6; ++i)
  {
    faces[i] = loadImageHDR(fmt::format(
      GRAPHICS_COURSE_RESOURCES_ROOT "/cubemaps/circus/{}.png", CUBEMAP_TEXTURE_NAMES[i]));
  }

  for (std::size_t i = 1; i < 6; ++i)
  {
    ETNA_VERIFYF(faces[i].width == faces[i].height, "Cubemap faces must be square");
    ETNA_VERIFYF(faces[i].width == faces[0].width, "Cubemap faces must have equal size");
  }

  std::uint32_t faceSize = faces[0].width;

  auto& ctx = etna::get_context();

  cubemapStagingBuffer = ctx.createBuffer(etna::Buffer::CreateInfo{
    .size = faceSize * faceSize * sizeof(glm::vec4) * 6,
    .bufferUsage = vk::BufferUsageFlagBits::eTransferSrc,
    .memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    .name = "cubemap_staging_buffer",
  });

  std::byte* stagingBufferData = cubemapStagingBuffer.map();

  for (std::size_t i = 0; i < 6; ++i)
  {
    std::memcpy(
      stagingBufferData + faceSize * faceSize * sizeof(glm::vec4) * i,
      faces[i].data.data(),
      faceSize * faceSize * sizeof(glm::vec4));
  }

  etna::set_state(
    command_buffer,
    cubemap.get(),
    vk::PipelineStageFlagBits2::eTransfer,
    vk::AccessFlagBits2::eTransferWrite,
    vk::ImageLayout::eTransferDstOptimal,
    vk::ImageSubresourceRange{
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 6,
    });
  etna::flush_barriers(command_buffer);

  command_buffer.copyBufferToImage(
    cubemapStagingBuffer.get(),
    cubemap.get(),
    vk::ImageLayout::eTransferDstOptimal,
    {
      vk::BufferImageCopy{
        .bufferOffset = 0,
        .bufferRowLength = faceSize,
        .bufferImageHeight = faceSize,
        .imageSubresource =
          {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 6,
          },
        .imageOffset = {0, 0, 0},
        .imageExtent = {faceSize, faceSize, 1},
      },
    });
}

void DiffuseBakingDemoRenderer::ResourcesPrepareContext::fillSphereSamples(
  vk::CommandBuffer command_buffer)
{
  namespace samples = generated::sphere_samples;

  sphereSamplesStagingBuffer = etna::get_context().createBuffer(etna::Buffer::CreateInfo{
    .size = std::span(samples::SAMPLE_DIRECTIONS).size_bytes(),
    .bufferUsage = vk::BufferUsageFlagBits::eTransferSrc,
    .memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    .name = "sphere_samples_staging_buffer",
  });

  std::byte* data = sphereSamplesStagingBuffer.map();

  std::memcpy(
    data, samples::SAMPLE_DIRECTIONS.data(), std::span(samples::SAMPLE_DIRECTIONS).size_bytes());

  command_buffer.copyBuffer(
    sphereSamplesStagingBuffer.get(),
    sphereSamples.get(),
    {
      vk::BufferCopy{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = std::span(samples::SAMPLE_DIRECTIONS).size_bytes(),
      },
    });
}

void DiffuseBakingDemoRenderer::ResourcesPrepareContext::bakeDiffuseSphericalHarmonics(
  vk::CommandBuffer command_buffer)
{
  etna::create_program(
    "diffuse_spherical_harmonics_bake",
    {
      DIFFUSE_BAKING_SHADERS_ROOT "shaders/baking/diffuse_spherical_harmonics.comp.spv",
    });

  auto& pipelineManager = etna::get_context().getPipelineManager();

  diffuseSphericalHarmonicsBakePipeline = pipelineManager.createComputePipeline(
    "diffuse_spherical_harmonics_bake", etna::ComputePipeline::CreateInfo{});

  {
    namespace shs = generated::spherical_harmonic_samples;

    std::size_t sampleDirectionsSize = std::span(shs::SAMPLE_DIRECTIONS).size_bytes();
    std::size_t precomputedValuesSize = std::span(shs::PRECOMPUTED_VALUES).size_bytes();
    precomputedValuesSize += (4 - precomputedValuesSize % 4) % 4;

    std::size_t bufferSize = sampleDirectionsSize + precomputedValuesSize;

    diffuseSphericalHarmonicSamplesStagingBuffer =
      etna::get_context().createBuffer(etna::Buffer::CreateInfo{
        .size = bufferSize,
        .bufferUsage = vk::BufferUsageFlagBits::eTransferSrc,
        .memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        .name = "diffuse_spherical_harmonic_samples_staging_buffer",
      });
    diffuseSphericalHarmonicSamples = etna::get_context().createBuffer(etna::Buffer::CreateInfo{
      .size = bufferSize,
      .bufferUsage =
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer,
      .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
      .name = "diffuse_spherical_harmonic_samples",
    });

    std::byte* data = diffuseSphericalHarmonicSamplesStagingBuffer.map();

    std::size_t offset = 0;

    std::memcpy(
      data + offset, shs::SAMPLE_DIRECTIONS.data(), std::span(shs::SAMPLE_DIRECTIONS).size_bytes());
    offset += sampleDirectionsSize;

    std::memcpy(
      data + offset,
      shs::PRECOMPUTED_VALUES.data(),
      std::span(shs::PRECOMPUTED_VALUES).size_bytes());
    offset += precomputedValuesSize;

    command_buffer.copyBuffer(
      diffuseSphericalHarmonicSamplesStagingBuffer.get(),
      diffuseSphericalHarmonicSamples.get(),
      {
        vk::BufferCopy{
          .srcOffset = 0,
          .dstOffset = 0,
          .size = bufferSize,
        },
      });

    command_buffer.fillBuffer(
      diffuseSphericalHarmonics.get(),
      0,
      (generated::spherical_harmonic_samples::MAX_BAND + 1) *
        (generated::spherical_harmonic_samples::MAX_BAND + 1) * sizeof(glm::vec4),
      0);

    std::array<vk::BufferMemoryBarrier2, 2> barriers = {
      vk::BufferMemoryBarrier2{
        .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
        .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eComputeShader,
        .dstAccessMask = vk::AccessFlagBits2::eShaderRead,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = diffuseSphericalHarmonicSamples.get(),
        .offset = 0,
        .size = bufferSize,
      },
      vk::BufferMemoryBarrier2{
        .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
        .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eComputeShader,
        .dstAccessMask = vk::AccessFlagBits2::eShaderWrite | vk::AccessFlagBits2::eShaderRead,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = diffuseSphericalHarmonics.get(),
        .offset = 0,
        .size = (generated::spherical_harmonic_samples::MAX_BAND + 1) *
          (generated::spherical_harmonic_samples::MAX_BAND + 1) * sizeof(glm::vec4),
      },
    };

    command_buffer.pipelineBarrier2(vk::DependencyInfo{
      .dependencyFlags = vk::DependencyFlagBits::eByRegion,
      .bufferMemoryBarrierCount = barriers.size(),
      .pBufferMemoryBarriers = barriers.data(),
    });
  }

  etna::set_state(
    command_buffer,
    cubemap.get(),
    vk::PipelineStageFlagBits2::eComputeShader,
    vk::AccessFlagBits2::eShaderRead,
    vk::ImageLayout::eShaderReadOnlyOptimal,
    vk::ImageSubresourceRange{
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 6,
    });
  etna::flush_barriers(command_buffer);

  auto diffuseSphericalHarmonicsBakeInfo =
    etna::get_shader_program("diffuse_spherical_harmonics_bake");

  auto set = etna::create_descriptor_set(
    diffuseSphericalHarmonicsBakeInfo.getDescriptorLayoutId(0),
    command_buffer,
    {
      etna::Binding{
        0,
        cubemap.genBinding(
          defaultSampler.get(),
          vk::ImageLayout::eShaderReadOnlyOptimal,
          etna::Image::ViewParams{
            .viewType = vk::ImageViewType::eCube,
            .layerCount = 6,
          }),
      },
      etna::Binding{
        1,
        diffuseSphericalHarmonicSamples.genBinding(),
      },
      etna::Binding{
        2,
        diffuseSphericalHarmonics.genBinding(),
      },
    });
  etna::flush_barriers(command_buffer);

  command_buffer.bindPipeline(
    vk::PipelineBindPoint::eCompute, diffuseSphericalHarmonicsBakePipeline.getVkPipeline());

  command_buffer.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute,
    diffuseSphericalHarmonicsBakePipeline.getVkPipelineLayout(),
    0,
    {set.getVkSet()},
    {});

  command_buffer.dispatch((generated::spherical_harmonic_samples::SAMPLE_COUNT - 1) / 8 + 1, 1, 1);
}

void DiffuseBakingDemoRenderer::debugInput(const Keyboard&) {}

void DiffuseBakingDemoRenderer::update(const FramePacket& packet)
{
  ZoneScoped;

  // calc camera matrix
  {
    const float aspect = static_cast<float>(resolution.x) / static_cast<float>(resolution.y);
    viewProjection = packet.mainCam.projTm(aspect) * packet.mainCam.viewTm();

    if (packet.mainCam.position != camera.position || packet.mainCam.rotation != camera.rotation)
    {
      time = 0;
    }

    camera = packet.mainCam;
  }
}

void DiffuseBakingDemoRenderer::doCubemapPass(
  vk::CommandBuffer command_buffer, vk::Image target_image, vk::ImageView target_image_view)
{
  etna::set_state(
    command_buffer,
    cubemap.get(),
    vk::PipelineStageFlagBits2::eFragmentShader,
    vk::AccessFlagBits2::eShaderRead,
    vk::ImageLayout::eShaderReadOnlyOptimal,
    vk::ImageSubresourceRange{
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 6,
    });
  etna::flush_barriers(command_buffer);

  etna::RenderTargetState renderTargets(
    command_buffer,
    {{0, 0}, {resolution.x, resolution.y}},
    {
      {
        .image = target_image,
        .view = target_image_view,
      },
    },
    {});

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, cubemapPipeline.getVkPipeline());

  auto cubemapInfo = etna::get_shader_program("cubemap");

  auto set = etna::create_descriptor_set(
    cubemapInfo.getDescriptorLayoutId(0),
    command_buffer,
    {
      etna::Binding{
        0,
        etna::ImageBinding{
          cubemap,
          vk::DescriptorImageInfo{
            .sampler = defaultSampler.get(),
            .imageView = cubemap.getView(etna::Image::ViewParams{
              .viewType = vk::ImageViewType::eCube,
              .layerCount = 6,
            }),
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
          },
        },
      },
    });

  command_buffer.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, cubemapInfo.getPipelineLayout(), 0, {set.getVkSet()}, {});

  const float aspect = static_cast<float>(resolution.x) / static_cast<float>(resolution.y);
  glm::mat4 viewProjection = camera.projTm(aspect) * glm::mat4(glm::mat3(camera.viewTm()));

  command_buffer.pushConstants<glm::mat4>(
    cubemapPipeline.getVkPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, {viewProjection});

  command_buffer.draw(36, 1, 0, 0);
}

void DiffuseBakingDemoRenderer::doSpherePass(vk::CommandBuffer command_buffer)
{
  etna::GraphicsPipeline* pipeline = nullptr;
  std::optional<etna::ShaderProgramInfo> shaderInfo;
  std::vector<etna::Binding> bindings;
  switch (diffuseMode)
  {
  case DiffuseMode::DYNAMIC:
    pipeline = &dynamicDiffusePipeline;
    shaderInfo.emplace(etna::get_shader_program("dynamic_diffuse"));
    bindings = std::vector<etna::Binding>{
      etna::Binding{
        0,
        cubemap.genBinding(
          defaultSampler.get(),
          vk::ImageLayout::eShaderReadOnlyOptimal,
          etna::Image::ViewParams{
            .viewType = vk::ImageViewType::eCube,
            .layerCount = 6,
          }),
      },
      etna::Binding{
        1,
        sphereSamples.genBinding(),
      },
    };
    etna::set_state(
      command_buffer,
      cubemap.get(),
      vk::PipelineStageFlagBits2::eFragmentShader,
      vk::AccessFlagBits2::eShaderRead,
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::ImageSubresourceRange{
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 6,
      });
    break;
  case DiffuseMode::SPHERICAL_HARMONICS:
    pipeline = &sphericalHarmonicsDiffusePipeline;
    shaderInfo.emplace(etna::get_shader_program("spherical_harmonics_diffuse"));
    bindings = std::vector<etna::Binding>{
      etna::Binding{
        0,
        diffuseSphericalHarmonics.genBinding(),
      },
    };
    break;
  default:
    ETNA_PANIC("Unknown diffuse mode");
    break;
  }
  etna::flush_barriers(command_buffer);

  etna::RenderTargetState renderTargets(
    command_buffer,
    {{0, 0}, {resolution.x, resolution.y}},
    {
      {
        .image = mainTarget.get(),
        .view = mainTarget.getView({}),
        .loadOp = time == 0 ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad,
        .clearColorValue = std::array<float, 4>({0.0f, 0.0f, 0.0f, 0.0f}),
      },
    },
    {});

  etna::DescriptorSet set =
    etna::create_descriptor_set(shaderInfo->getDescriptorLayoutId(0), command_buffer, bindings);

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getVkPipeline());

  command_buffer.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, shaderInfo->getPipelineLayout(), 0, {set.getVkSet()}, {});

  struct PushConstants
  {
    glm::mat4 viewProjection;
    glm::vec4 cameraPositionTime;
  };

  command_buffer.pushConstants<PushConstants>(
    pipeline->getVkPipelineLayout(),
    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
    0,
    {
      PushConstants{
        .viewProjection = viewProjection,
        .cameraPositionTime = glm::vec4(camera.position, glm::uintBitsToFloat(time)),
      },
    });

  std::array<float, 4> blendConstants = {1.0f / time, 1.0f / time, 1.0f / time, 0.0};
  command_buffer.setBlendConstants(blendConstants.data());

  {
    namespace mesh = generated::sphere_mesh;

    command_buffer.draw(
      2 * mesh::LONGITUDE_RESOLUTION * 3 +
        mesh::LONGITUDE_RESOLUTION * (mesh::LATITUDE_RESOLUTION - 2) * 6,
      1,
      0,
      0);
  }
}

void DiffuseBakingDemoRenderer::doSceneBlendPass(
  vk::CommandBuffer command_buffer, vk::Image target_image, vk::ImageView target_image_view)
{
  etna::set_state(
    command_buffer,
    mainTarget.get(),
    vk::PipelineStageFlagBits2::eFragmentShader,
    vk::AccessFlagBits2::eShaderRead,
    vk::ImageLayout::eShaderReadOnlyOptimal,
    vk::ImageAspectFlagBits::eColor);
  etna::flush_barriers(command_buffer);

  etna::RenderTargetState renderTargets(
    command_buffer,
    {{0, 0}, {resolution.x, resolution.y}},
    {
      {
        .image = target_image,
        .view = target_image_view,
        .loadOp = vk::AttachmentLoadOp::eLoad,
      },
    },
    {});

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, sceneBlendPipeline.getVkPipeline());

  auto sceneBlendInfo = etna::get_shader_program("scene_blend");

  auto set = etna::create_descriptor_set(
    sceneBlendInfo.getDescriptorLayoutId(0),
    command_buffer,
    {
      etna::Binding{
        0, mainTarget.genBinding(defaultSampler.get(), vk::ImageLayout::eShaderReadOnlyOptimal)},
    });

  command_buffer.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, sceneBlendInfo.getPipelineLayout(), 0, {set.getVkSet()}, {});

  command_buffer.draw(6, 1, 0, 0);
}

void DiffuseBakingDemoRenderer::renderWorld(
  vk::CommandBuffer command_buffer, vk::Image target_image, vk::ImageView target_image_view)
{
  ETNA_PROFILE_GPU(command_buffer, renderWorld);

  if (!prepareContext.has_value())
  {
    prepareContext.emplace(cubemap, sphereSamples, diffuseSphericalHarmonics);
    prepareContext->prepareResources();
  }

  // Cubemap pass

  {
    ETNA_PROFILE_GPU(command_buffer, cubemapPass);

    doCubemapPass(command_buffer, target_image, target_image_view);
  }

  // Sphere pass

  {
    ETNA_PROFILE_GPU(command_buffer, spherePass);

    doSpherePass(command_buffer);
  }

  // Scene blend pass

  {
    ETNA_PROFILE_GPU(command_buffer, sceneBlendPass);

    doSceneBlendPass(command_buffer, target_image, target_image_view);
  }

  ++time;
}

void DiffuseBakingDemoRenderer::drawGui()
{
  ImGui::Begin("Simple render settings");

  ImGui::Text(
    "Application average %.3f ms/frame (%.1f FPS)",
    1000.0f / ImGui::GetIO().Framerate,
    ImGui::GetIO().Framerate);

  struct DiffuseModeMetadata
  {
    DiffuseMode mode;
    const char* name;
  };

  static const std::array DIFFUSE_MODE_METADATA = {
    DiffuseModeMetadata{
      .mode = DiffuseMode::DYNAMIC,
      .name = "Dynamic",
    },
    DiffuseModeMetadata{
      .mode = DiffuseMode::SPHERICAL_HARMONICS,
      .name = "Spherical harmonics",
    },
  };

  const char* preview =
    std::ranges::find(DIFFUSE_MODE_METADATA, diffuseMode, &DiffuseModeMetadata::mode)->name;
  if (ImGui::BeginCombo("Diffuse mode", preview))
  {
    for (const DiffuseModeMetadata& metadata : DIFFUSE_MODE_METADATA)
    {
      if (ImGui::Selectable(metadata.name))
      {
        diffuseMode = metadata.mode;
        time = 0;
        break;
      }
    }

    ImGui::EndCombo();
  }

  ImGui::NewLine();

  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Press 'B' to recompile and reload shaders");
  ImGui::End();
}

void DiffuseBakingDemoRenderer::initCubemapPipeline(vk::Format swapchain_format)
{
  auto& pipelineManager = etna::get_context().getPipelineManager();

  cubemapPipeline = pipelineManager.createGraphicsPipeline(
    "cubemap",
    etna::GraphicsPipeline::CreateInfo{
      .rasterizationConfig =
        vk::PipelineRasterizationStateCreateInfo{
          .polygonMode = vk::PolygonMode::eFill,
          .cullMode = vk::CullModeFlagBits::eBack,
          .frontFace = vk::FrontFace::eCounterClockwise,
          .lineWidth = 1.0f,
        },
      .blendingConfig =
        etna::GraphicsPipeline::CreateInfo::Blending{
          .attachments =
            {
              vk::PipelineColorBlendAttachmentState{
                .blendEnable = vk::False,
                .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                  vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
              },
            },
          .logicOpEnable = false,
          .logicOp = vk::LogicOp::eCopy,
        },
      .fragmentShaderOutput =
        {
          .colorAttachmentFormats =
            {
              swapchain_format,
            },
          .depthAttachmentFormat = vk::Format::eUndefined,
        },
    });
}

void DiffuseBakingDemoRenderer::initDynamicDiffusePipeline()
{
  auto& pipelineManager = etna::get_context().getPipelineManager();

  dynamicDiffusePipeline =
    pipelineManager
      .createGraphicsPipeline(
        "dynamic_diffuse",
        etna::GraphicsPipeline::CreateInfo{
          .rasterizationConfig =
            vk::PipelineRasterizationStateCreateInfo{
              .polygonMode = vk::PolygonMode::eFill,
              .cullMode = vk::CullModeFlagBits::eBack,
              .frontFace = vk::FrontFace::eCounterClockwise,
              .lineWidth = 1.0f,
            },
          .blendingConfig =
            etna::GraphicsPipeline::CreateInfo::Blending{
              .attachments =
                {
                  vk::PipelineColorBlendAttachmentState{
                    .blendEnable = vk::True,
                    .srcColorBlendFactor = vk::BlendFactor::eConstantColor,
                    .dstColorBlendFactor = vk::BlendFactor::eOneMinusConstantColor,
                    .colorBlendOp = vk::BlendOp::eAdd,
                    .srcAlphaBlendFactor = vk::BlendFactor::eOne,
                    .dstAlphaBlendFactor = vk::BlendFactor::eZero,
                    .alphaBlendOp = vk::BlendOp::eAdd,
                    .colorWriteMask = vk::ColorComponentFlagBits::eR |
                      vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                      vk::ColorComponentFlagBits::eA,
                  },
                },
              .logicOpEnable = false,
              .logicOp = vk::LogicOp::eCopy,
            },
          .fragmentShaderOutput =
            {
              .colorAttachmentFormats =
                {
                  vk::Format::eR8G8B8A8Unorm,
                },
              .depthAttachmentFormat = vk::Format::eUndefined,
            },
          .dynamicStates =
            {
              vk::DynamicState::eViewport,
              vk::DynamicState::eScissor,
              vk::DynamicState::eBlendConstants,
            },
        });
}

void DiffuseBakingDemoRenderer::initSphericalHarmonicsDiffusePipeline()
{
  auto& pipelineManager = etna::get_context().getPipelineManager();

  sphericalHarmonicsDiffusePipeline =
    pipelineManager
      .createGraphicsPipeline(
        "spherical_harmonics_diffuse",
        etna::GraphicsPipeline::CreateInfo{
          .rasterizationConfig =
            vk::PipelineRasterizationStateCreateInfo{
              .polygonMode = vk::PolygonMode::eFill,
              .cullMode = vk::CullModeFlagBits::eBack,
              .frontFace = vk::FrontFace::eCounterClockwise,
              .lineWidth = 1.0f,
            },
          .blendingConfig =
            etna::GraphicsPipeline::CreateInfo::Blending{
              .attachments =
                {
                  vk::PipelineColorBlendAttachmentState{
                    .blendEnable = vk::True,
                    .srcColorBlendFactor = vk::BlendFactor::eConstantColor,
                    .dstColorBlendFactor = vk::BlendFactor::eOneMinusConstantColor,
                    .colorBlendOp = vk::BlendOp::eAdd,
                    .srcAlphaBlendFactor = vk::BlendFactor::eOne,
                    .dstAlphaBlendFactor = vk::BlendFactor::eZero,
                    .alphaBlendOp = vk::BlendOp::eAdd,
                    .colorWriteMask = vk::ColorComponentFlagBits::eR |
                      vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                      vk::ColorComponentFlagBits::eA,
                  },
                },
              .logicOpEnable = false,
              .logicOp = vk::LogicOp::eCopy,
            },
          .fragmentShaderOutput =
            {
              .colorAttachmentFormats =
                {
                  vk::Format::eR8G8B8A8Unorm,
                },
              .depthAttachmentFormat = vk::Format::eUndefined,
            },
          .dynamicStates =
            {
              vk::DynamicState::eViewport,
              vk::DynamicState::eScissor,
              vk::DynamicState::eBlendConstants,
            },
        });
}

void DiffuseBakingDemoRenderer::initSceneBlendPipeline(vk::Format swapchain_format)
{
  auto& pipelineManager = etna::get_context().getPipelineManager();

  sceneBlendPipeline =
    pipelineManager
      .createGraphicsPipeline(
        "scene_blend",
        etna::GraphicsPipeline::CreateInfo{
          .rasterizationConfig =
            vk::PipelineRasterizationStateCreateInfo{
              .polygonMode = vk::PolygonMode::eFill,
              .cullMode = vk::CullModeFlagBits::eBack,
              .frontFace = vk::FrontFace::eCounterClockwise,
              .lineWidth = 1.0f,
            },
          .blendingConfig =
            etna::GraphicsPipeline::CreateInfo::Blending{
              .attachments =
                {
                  vk::PipelineColorBlendAttachmentState{
                    .blendEnable = vk::True,
                    .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
                    .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
                    .colorBlendOp = vk::BlendOp::eAdd,
                    .srcAlphaBlendFactor = vk::BlendFactor::eOne,
                    .dstAlphaBlendFactor = vk::BlendFactor::eZero,
                    .alphaBlendOp = vk::BlendOp::eAdd,
                    .colorWriteMask = vk::ColorComponentFlagBits::eR |
                      vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                      vk::ColorComponentFlagBits::eA,
                  },
                },
              .logicOpEnable = false,
              .logicOp = vk::LogicOp::eCopy,
            },
          .fragmentShaderOutput =
            {
              .colorAttachmentFormats =
                {
                  swapchain_format,
                },
              .depthAttachmentFormat = vk::Format::eUndefined,
            },
          .dynamicStates =
            {
              vk::DynamicState::eViewport,
              vk::DynamicState::eScissor,
            },
        });
}
