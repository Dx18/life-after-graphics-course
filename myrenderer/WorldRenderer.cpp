#include "WorldRenderer.hpp"

#include <glm/ext.hpp>

#include <imgui.h>

#include <etna/GlobalContext.hpp>
#include <etna/PipelineManager.hpp>
#include <etna/RenderTargetStates.hpp>
#include <etna/Profiling.hpp>


WorldRenderer::WorldRenderer()
  : sceneMgr{std::make_unique<SceneManager>()}
{
}

void WorldRenderer::allocateResources(glm::uvec2 swapchain_resolution)
{
  resolution = swapchain_resolution;

  auto& ctx = etna::get_context();

  gBuffer.channels = {{
    ctx.createImage(etna::Image::CreateInfo{
      .extent = vk::Extent3D{resolution.x, resolution.y, 1},
      .name = "channel0",
      .format = vk::Format::eR8G8B8A8Unorm,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
    }),
    ctx.createImage(etna::Image::CreateInfo{
      .extent = vk::Extent3D{resolution.x, resolution.y, 1},
      .name = "channel1",
      .format = vk::Format::eA2R10G10B10UnormPack32,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
    }),
    ctx.createImage(etna::Image::CreateInfo{
      .extent = vk::Extent3D{resolution.x, resolution.y, 1},
      .name = "channel2",
      .format = vk::Format::eR8G8B8A8Unorm,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
    }),
  }};
  gBuffer.depth = ctx.createImage(etna::Image::CreateInfo{
    .extent = vk::Extent3D{resolution.x, resolution.y, 1},
    .name = "depth",
    .format = vk::Format::eD32Sfloat,
    .imageUsage =
      vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
  });

  defaultSampler = etna::Sampler(etna::Sampler::CreateInfo{.name = "default_sampler"});

  std::unique_ptr<etna::OneShotCmdMgr> oneShotCmdMgr = ctx.createOneShotCmdMgr();

  vk::CommandBuffer commandBuffer = oneShotCmdMgr->start();

  defaultBaseColorImage = etna::create_image_from_bytes(
    etna::Image::CreateInfo{
      .extent = vk::Extent3D{1, 1, 1},
      .name = "default_base_color",
      .format = vk::Format::eR8G8B8A8Srgb,
      .imageUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
    },
    commandBuffer,
    std::array<std::uint8_t, 4>{0xFF, 0xFF, 0xFF, 0xFF}.data());
  defaultMetalnessRoughnessImage = etna::create_image_from_bytes(
    etna::Image::CreateInfo{
      .extent = vk::Extent3D{1, 1, 1},
      .name = "default_metallic_roughness",
      .format = vk::Format::eR8G8B8A8Srgb,
      .imageUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
    },
    commandBuffer,
    std::array<std::uint8_t, 4>{0x00, 0xFF, 0xFF, 0xFF}.data());
  defaultNormalImage = etna::create_image_from_bytes(
    etna::Image::CreateInfo{
      .extent = vk::Extent3D{1, 1, 1},
      .name = "default_normal",
      .format = vk::Format::eR8G8B8A8Srgb,
      .imageUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
    },
    commandBuffer,
    std::array<std::uint8_t, 4>{0x80, 0x80, 0xFF, 0xFF}.data());
  defaultOcclusionImage = etna::create_image_from_bytes(
    etna::Image::CreateInfo{
      .extent = vk::Extent3D{1, 1, 1},
      .name = "default_occlusion",
      .format = vk::Format::eR8G8B8A8Srgb,
      .imageUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
    },
    commandBuffer,
    std::array<std::uint8_t, 4>{0xFF, 0xFF, 0xFF, 0xFF}.data());
  defaultEmissionImage = etna::create_image_from_bytes(
    etna::Image::CreateInfo{
      .extent = vk::Extent3D{1, 1, 1},
      .name = "default_emissive",
      .format = vk::Format::eR8G8B8A8Srgb,
      .imageUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
    },
    commandBuffer,
    std::array<std::uint8_t, 4>{0x00, 0x00, 0x00, 0x00}.data());
}

void WorldRenderer::loadScene(std::filesystem::path path)
{
  sceneMgr->selectScene(path);
}

void WorldRenderer::loadShaders()
{
  etna::create_program(
    "geometry_pass",
    {
      MYRENDERER_SHADERS_ROOT "g_buffer_pass.vert.spv",
    });
  etna::create_program(
    "g_buffer_pass",
    {
      MYRENDERER_SHADERS_ROOT "g_buffer_pass.vert.spv",
      MYRENDERER_SHADERS_ROOT "g_buffer_pass.frag.spv",
    });
  etna::create_program(
    "resolve_pass",
    {
      MYRENDERER_SHADERS_ROOT "resolve_pass.vert.spv",
      MYRENDERER_SHADERS_ROOT "resolve_pass.frag.spv",
    });
}

void WorldRenderer::setupPipelines(vk::Format swapchain_format)
{
  initGridRenderer(swapchain_format);

  initGeometryPassPipeline();
  initGBufferPassPipeline();
  initResolvePassPipeline(swapchain_format);
}

void WorldRenderer::debugInput(const Keyboard& kb)
{
  if (kb[KeyboardKey::kG] == ButtonState::Falling)
  {
    drawDebugGrid = !drawDebugGrid;
  }
}

void WorldRenderer::update(const FramePacket& packet)
{
  ZoneScoped;

  // calc camera matrix
  {
    const float aspect = static_cast<float>(resolution.x) / static_cast<float>(resolution.y);
    viewProjection = packet.mainCam.projTm(aspect) * packet.mainCam.viewTm();
    camera = packet.mainCam;
  }
}

etna::DescriptorSet WorldRenderer::createMaterialBindings(
  vk::CommandBuffer command_buffer, std::optional<MaterialIndex> material_index) const
{
  auto createImageBinding =
    [this](std::optional<TextureIndex> texture_index, const etna::Image& fallback_image) {
      const etna::Image* image;
      const etna::Sampler* sampler;
      if (texture_index.has_value())
      {
        const SceneMaterials& materials = sceneMgr->getMaterials();

        const Texture& texture = materials.getTexture(*texture_index);
        image = &materials.getImage(texture.image);
        sampler = &materials.getSampler(texture.sampler);
      }
      else
      {
        image = &fallback_image;
        sampler = &defaultSampler;
      }

      return image->genBinding(sampler->get(), vk::ImageLayout::eShaderReadOnlyOptimal);
    };

  std::array<std::optional<TextureIndex>, 5> textureIndices;

  if (material_index.has_value())
  {
    const SceneMaterials& materials = sceneMgr->getMaterials();

    const Material& material = materials.getMaterial(*material_index);

    textureIndices[0] = material.baseColorTexture;
    textureIndices[1] = material.metallicRoughnessTexture;
    textureIndices[2] = material.normalTexture;
    textureIndices[3] = material.occlusionTexture;
    textureIndices[4] = material.emissiveTexture;
  }

  std::array<const etna::Image*, 5> kFallbackImages = {
    &defaultBaseColorImage,
    &defaultMetalnessRoughnessImage,
    &defaultNormalImage,
    &defaultOcclusionImage,
    &defaultEmissionImage,
  };

  std::vector<etna::ImageBinding> imageBindings;
  for (std::size_t i = 0; i < textureIndices.size(); ++i)
  {
    imageBindings.push_back(createImageBinding(textureIndices[i], *kFallbackImages[i]));
  }

  std::vector<etna::Binding> bindings;
  for (std::size_t i = 0; i < textureIndices.size(); ++i)
  {
    bindings.push_back(etna::Binding{static_cast<std::uint32_t>(i), imageBindings[i]});
  }

  auto gBufferPassInfo = etna::get_shader_program("g_buffer_pass");

  return etna::create_descriptor_set(
    gBufferPassInfo.getDescriptorLayoutId(0), command_buffer, std::move(bindings));
}

WorldRenderer::MaterialConstants WorldRenderer::getMaterialConstants(
  std::optional<MaterialIndex> material_index) const
{
  glm::vec4 baseColorMetallicFactor = glm::vec4(1.0, 1.0, 1.0, 1.0);
  glm::vec4 emissiveRoughnessFactors = glm::vec4(0.0, 0.0, 0.0, 1.0);

  if (material_index.has_value())
  {
    const SceneMaterials& materials = sceneMgr->getMaterials();

    const Material& material = materials.getMaterial(*material_index);

    baseColorMetallicFactor = glm::vec4(glm::vec3(material.baseColor), material.metallicFactor);
    emissiveRoughnessFactors =
      glm::vec4(glm::vec3(material.emissiveFactor), material.roughnessFactor);
  }

  return MaterialConstants{
    .baseColorMetallicFactor = baseColorMetallicFactor,
    .emissiveRoughnessFactors = emissiveRoughnessFactors,
  };
}

void WorldRenderer::doGeometryPass(vk::CommandBuffer command_buffer)
{
  etna::RenderTargetState renderTargets(
    command_buffer,
    {{0, 0}, {resolution.x, resolution.y}},
    {},
    {.image = gBuffer.depth.get(), .view = gBuffer.depth.getView({})});

  command_buffer.bindPipeline(
    vk::PipelineBindPoint::eGraphics, geometryPassPipeline.getVkPipeline());

  struct TransformConstants
  {
    glm::mat4 viewProjection;
    glm::mat4 model;
  };

  const SceneMeshes& sceneMeshes = sceneMgr->getMeshes();

  if (!sceneMeshes.getVertexBuffer())
  {
    return;
  }

  command_buffer.bindVertexBuffers(0, {sceneMeshes.getVertexBuffer()}, {0});
  command_buffer.bindIndexBuffer(sceneMeshes.getIndexBuffer(), 0, vk::IndexType::eUint32);

  TransformConstants transformConstants = {
    .viewProjection = viewProjection,
    .model = glm::mat4(1.0),
  };

  auto instanceMatrices = sceneMgr->getInstanceMatrices();

  auto meshInstances = sceneMeshes.getInstanceMeshes();

  auto meshes = sceneMeshes.getMeshes();
  auto renderElements = sceneMeshes.getRenderElements();

  for (std::size_t instIdx = 0; instIdx < meshInstances.size(); ++instIdx)
  {
    transformConstants.model = instanceMatrices[instIdx];

    command_buffer.pushConstants<TransformConstants>(
      geometryPassPipeline.getVkPipelineLayout(),
      vk::ShaderStageFlagBits::eVertex,
      0,
      {transformConstants});

    const auto meshInstance = meshInstances[instIdx];

    for (std::size_t j = 0; j < meshes[meshInstance.mesh].relemCount; ++j)
    {
      const auto relemIdx = meshes[meshInstance.mesh].firstRelem + j;
      const auto& relem = renderElements[relemIdx];

      command_buffer.drawIndexed(relem.indexCount, 1, relem.indexOffset, relem.vertexOffset, 0);
    }
  }
}

void WorldRenderer::doGBufferPass(vk::CommandBuffer command_buffer)
{
  etna::RenderTargetState renderTargets(
    command_buffer,
    {{0, 0}, {resolution.x, resolution.y}},
    {
      {
        .image = gBuffer.channels[0].get(),
        .view = gBuffer.channels[0].getView({}),
        .clearColorValue = std::array<float, 4>{0.0, 0.0, 0.0, 0.0},
      },
      {
        .image = gBuffer.channels[1].get(),
        .view = gBuffer.channels[1].getView({}),
        .clearColorValue = std::array<float, 4>{0.5, 0.5, 0.5, 0.0},
      },
      {
        .image = gBuffer.channels[2].get(),
        .view = gBuffer.channels[2].getView({}),
        .clearColorValue = std::array<float, 4>{0.2, 0.3, 0.8, 0.0},
      },
    },
    {.image = gBuffer.depth.get(), .view = gBuffer.depth.getView({})});

  command_buffer.bindPipeline(
    vk::PipelineBindPoint::eGraphics, gBufferPassPipeline.getVkPipeline());

  struct TransformConstants
  {
    glm::mat4 viewProjection;
    glm::mat4 model;
  };

  const SceneMeshes& sceneMeshes = sceneMgr->getMeshes();

  if (!sceneMeshes.getVertexBuffer())
  {
    return;
  }

  command_buffer.bindVertexBuffers(0, {sceneMeshes.getVertexBuffer()}, {0});
  command_buffer.bindIndexBuffer(sceneMeshes.getIndexBuffer(), 0, vk::IndexType::eUint32);

  TransformConstants transformConstants = {
    .viewProjection = viewProjection,
    .model = glm::mat4(1.0),
  };

  auto instanceMatrices = sceneMgr->getInstanceMatrices();

  auto meshInstances = sceneMeshes.getInstanceMeshes();

  auto meshes = sceneMeshes.getMeshes();
  auto renderElements = sceneMeshes.getRenderElements();

  for (std::size_t instIdx = 0; instIdx < meshInstances.size(); ++instIdx)
  {
    transformConstants.model = instanceMatrices[instIdx];

    command_buffer.pushConstants<TransformConstants>(
      gBufferPassPipeline.getVkPipelineLayout(),
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
      0,
      {transformConstants});

    const auto meshInstance = meshInstances[instIdx];

    for (std::size_t j = 0; j < meshes[meshInstance.mesh].relemCount; ++j)
    {
      const auto relemIdx = meshes[meshInstance.mesh].firstRelem + j;
      const auto& relem = renderElements[relemIdx];

      MaterialConstants materialConstants = getMaterialConstants(relem.material);

      auto set = createMaterialBindings(command_buffer, relem.material);

      command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        gBufferPassPipeline.getVkPipelineLayout(),
        0,
        {set.getVkSet()},
        {});

      command_buffer.pushConstants<MaterialConstants>(
        gBufferPassPipeline.getVkPipelineLayout(),
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        sizeof(transformConstants),
        {materialConstants});

      command_buffer.drawIndexed(relem.indexCount, 1, relem.indexOffset, relem.vertexOffset, 0);
    }
  }
}

void WorldRenderer::doResolvePass(
  vk::CommandBuffer cmd_buf, vk::Image target_image, vk::ImageView target_image_view)
{
  for (const etna::Image& image : gBuffer.channels)
  {
    etna::set_state(
      cmd_buf,
      image.get(),
      vk::PipelineStageFlagBits2::eFragmentShader,
      vk::AccessFlagBits2::eShaderRead,
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::ImageAspectFlagBits::eColor);
  }
  etna::set_state(
    cmd_buf,
    gBuffer.depth.get(),
    vk::PipelineStageFlagBits2::eFragmentShader,
    vk::AccessFlagBits2::eShaderRead,
    vk::ImageLayout::eShaderReadOnlyOptimal,
    vk::ImageAspectFlagBits::eDepth);
  etna::flush_barriers(cmd_buf);

  etna::RenderTargetState renderTargets(
    cmd_buf,
    {{0, 0}, {resolution.x, resolution.y}},
    {
      {.image = target_image, .view = target_image_view},
    },
    {});

  auto resolvePassInfo = etna::get_shader_program("resolve_pass");

  auto set = etna::create_descriptor_set(
    resolvePassInfo.getDescriptorLayoutId(0),
    cmd_buf,
    {
      etna::Binding{0, sceneMgr->getLights().getLightsDataBuffer().genBinding()},
      etna::Binding{
        1,
        gBuffer.channels[0].genBinding(
          defaultSampler.get(), vk::ImageLayout::eShaderReadOnlyOptimal)},
      etna::Binding{
        2,
        gBuffer.channels[1].genBinding(
          defaultSampler.get(), vk::ImageLayout::eShaderReadOnlyOptimal)},
      etna::Binding{
        3,
        gBuffer.channels[2].genBinding(
          defaultSampler.get(), vk::ImageLayout::eShaderReadOnlyOptimal)},
      etna::Binding{
        4, gBuffer.depth.genBinding(defaultSampler.get(), vk::ImageLayout::eShaderReadOnlyOptimal)},
    });

  cmd_buf.bindPipeline(vk::PipelineBindPoint::eGraphics, resolvePassPipeline.getVkPipeline());
  cmd_buf.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics,
    resolvePassPipeline.getVkPipelineLayout(),
    0,
    {set.getVkSet()},
    {});

  struct CameraConstants
  {
    glm::vec4 cameraPositionAspect;
    glm::vec4 cameraViewDirZNear;
    glm::vec4 cameraViewUpDirZFar;
  };

  CameraConstants cameraConstants = {
    .cameraPositionAspect = glm::vec4(
      camera.position, static_cast<float>(resolution.x) / static_cast<float>(resolution.y)),
    .cameraViewDirZNear = glm::vec4(camera.forward(), camera.zNear),
    .cameraViewUpDirZFar =
      glm::vec4(camera.up() * glm::tan(glm::radians(camera.fov) / 2.0f), camera.zFar),
  };

  cmd_buf.pushConstants<CameraConstants>(
    resolvePassPipeline.getVkPipelineLayout(),
    vk::ShaderStageFlagBits::eFragment,
    0,
    {cameraConstants});

  cmd_buf.draw(6, 1, 0, 0);
}

void WorldRenderer::renderWorld(
  vk::CommandBuffer cmd_buf, vk::Image target_image, vk::ImageView target_image_view)
{
  ETNA_PROFILE_GPU(cmd_buf, renderWorld);

  // G-Buffer pass

  {
    ETNA_PROFILE_GPU(cmd_buf, geometryPass);

    doGeometryPass(cmd_buf);
  }

  {
    ETNA_PROFILE_GPU(cmd_buf, gBufferPass);

    doGBufferPass(cmd_buf);
  }

  // Lighting passes

  {
    ETNA_PROFILE_GPU(cmd_buf, resolvePass);

    doResolvePass(cmd_buf, target_image, target_image_view);
  }

  // Grid pass

  if (drawDebugGrid)
  {
    gridRenderer->render(
      cmd_buf,
      {{0, 0}, {resolution.x, resolution.y}},
      viewProjection,
      target_image,
      target_image_view,
      gBuffer.depth.get(),
      gBuffer.depth.getView({}));
  }
}

void WorldRenderer::drawGui()
{
  ImGui::Begin("Simple render settings");

  ImGui::Checkbox("Draw grid", &drawDebugGrid);

  ImGui::Text(
    "Application average %.3f ms/frame (%.1f FPS)",
    1000.0f / ImGui::GetIO().Framerate,
    ImGui::GetIO().Framerate);

  ImGui::NewLine();

  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Press 'B' to recompile and reload shaders");
  ImGui::End();
}

void WorldRenderer::initGeometryPassPipeline()
{
  etna::VertexShaderInputDescription sceneVertexInputDesc{
    .bindings = {etna::VertexShaderInputDescription::Binding{
      .byteStreamDescription = sceneMgr->getMeshes().getVertexFormatDescription(),
    }},
  };

  auto& pipelineManager = etna::get_context().getPipelineManager();

  geometryPassPipeline = pipelineManager.createGraphicsPipeline(
    "geometry_pass",
    etna::GraphicsPipeline::CreateInfo{
      .vertexShaderInput = sceneVertexInputDesc,
      .rasterizationConfig =
        vk::PipelineRasterizationStateCreateInfo{
          .polygonMode = vk::PolygonMode::eFill,
          .cullMode = vk::CullModeFlagBits::eBack,
          .frontFace = vk::FrontFace::eCounterClockwise,
          .lineWidth = 1.f,
        },
      .blendingConfig =
        etna::GraphicsPipeline::CreateInfo::Blending{
          .attachments = {},
          .logicOpEnable = false,
          .logicOp = vk::LogicOp::eCopy,
        },
      .fragmentShaderOutput =
        {
          .colorAttachmentFormats = {},
          .depthAttachmentFormat = vk::Format::eD32Sfloat,
        },
    });
}

void WorldRenderer::initGBufferPassPipeline()
{
  etna::VertexShaderInputDescription sceneVertexInputDesc{
    .bindings = {etna::VertexShaderInputDescription::Binding{
      .byteStreamDescription = sceneMgr->getMeshes().getVertexFormatDescription(),
    }},
  };

  auto& pipelineManager = etna::get_context().getPipelineManager();

  gBufferPassPipeline =
    pipelineManager
      .createGraphicsPipeline(
        "g_buffer_pass",
        etna::GraphicsPipeline::CreateInfo{
          .vertexShaderInput = sceneVertexInputDesc,
          .rasterizationConfig =
            vk::PipelineRasterizationStateCreateInfo{
              .polygonMode = vk::PolygonMode::eFill,
              .cullMode = vk::CullModeFlagBits::eBack,
              .frontFace = vk::FrontFace::eCounterClockwise,
              .lineWidth = 1.f,
            },
          .blendingConfig =
            etna::GraphicsPipeline::CreateInfo::Blending{
              .attachments =
                {
                  vk::PipelineColorBlendAttachmentState{
                    .blendEnable = vk::False,
                    .colorWriteMask = vk::ColorComponentFlagBits::eR |
                      vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                      vk::ColorComponentFlagBits::eA,
                  },
                  vk::PipelineColorBlendAttachmentState{
                    .blendEnable = vk::False,
                    .colorWriteMask = vk::ColorComponentFlagBits::eR |
                      vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                      vk::ColorComponentFlagBits::eA,
                  },
                  vk::PipelineColorBlendAttachmentState{
                    .blendEnable = vk::False,
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
                  vk::Format::eA2R10G10B10UnormPack32,
                  vk::Format::eR8G8B8A8Unorm,
                },
              .depthAttachmentFormat = vk::Format::eD32Sfloat,
            },
        });
}

void WorldRenderer::initResolvePassPipeline(vk::Format swapchain_format)
{
  auto& pipelineManager = etna::get_context().getPipelineManager();

  resolvePassPipeline = pipelineManager.createGraphicsPipeline(
    "resolve_pass",
    etna::GraphicsPipeline::CreateInfo{
      .vertexShaderInput =
        etna::VertexShaderInputDescription{
          .bindings = {etna::VertexShaderInputDescription::Binding{
            .byteStreamDescription = etna::VertexByteStreamFormatDescription{}}},
        },
      .rasterizationConfig =
        vk::PipelineRasterizationStateCreateInfo{
          .polygonMode = vk::PolygonMode::eFill,
          .cullMode = vk::CullModeFlagBits::eBack,
          .frontFace = vk::FrontFace::eCounterClockwise,
          .lineWidth = 1.f,
        },
      .depthConfig =
        {
          .depthTestEnable = vk::False,
        },
      .fragmentShaderOutput =
        {
          .colorAttachmentFormats = {swapchain_format},
        },
    });
}

void WorldRenderer::initGridRenderer(vk::Format swapchain_format)
{
  std::vector<LineRenderer::Vertex> gridVertices;
  std::vector<std::uint32_t> gridIndices;

  const glm::vec3 gridExtent = glm::vec3(4.0);
  const int gridFrequency = 2;

  // Axes

  gridVertices.push_back({glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 0.0, 0.0)});
  gridVertices.push_back({glm::vec3(gridExtent.x, 0.0, 0.0), glm::vec3(1.0, 0.0, 0.0)});
  gridVertices.push_back({glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)});
  gridVertices.push_back({glm::vec3(0.0, gridExtent.y, 0.0), glm::vec3(0.0, 1.0, 0.0)});
  gridVertices.push_back({glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0)});
  gridVertices.push_back({glm::vec3(0.0, 0.0, gridExtent.z), glm::vec3(0.0, 0.0, 1.0)});

  gridIndices.push_back(0);
  gridIndices.push_back(1);
  gridIndices.push_back(2);
  gridIndices.push_back(3);
  gridIndices.push_back(4);
  gridIndices.push_back(5);

  // Lines

  for (int lineDir = 0; lineDir < 3; ++lineDir)
  {
    for (int sweepDir = 0; sweepDir < 3; ++sweepDir)
    {
      if (sweepDir == lineDir)
      {
        continue;
      }

      int bound = static_cast<int>(gridExtent[sweepDir] * gridFrequency);
      for (int i = -bound; i <= bound; ++i)
      {
        glm::vec3 color = i % gridFrequency == 0 ? glm::vec3(0.3) : glm::vec3(0.1);

        glm::vec3 position1 = glm::vec3(0.0);
        position1[lineDir] = -gridExtent[lineDir];
        position1[sweepDir] = i / static_cast<float>(gridFrequency);

        gridIndices.push_back(gridVertices.size());
        gridVertices.push_back({position1, color});

        glm::vec3 position2 = glm::vec3(0.0);
        if (i != 0)
        {
          position2[lineDir] = gridExtent[lineDir];
        }
        position2[sweepDir] = i / static_cast<float>(gridFrequency);

        gridIndices.push_back(gridVertices.size());
        gridVertices.push_back({position2, color});
      }
    }
  }

  gridRenderer = std::make_unique<LineRenderer>(LineRenderer::CreateInfo{
    .format = swapchain_format,
    .vertices = std::move(gridVertices),
    .indices = std::move(gridIndices),
  });
}
