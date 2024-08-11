#include "ImGuiRenderer.hpp"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include <etna/GlobalContext.hpp>
#include <etna/RenderTargetStates.hpp>
<<<<<<< HEAD
#include <etna/Profiling.hpp>
=======
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)


void ImGuiRenderer::enableImGuiForWindow(GLFWwindow* window)
{
  ImGui_ImplGlfw_InitForVulkan(window, true);
}

ImGuiRenderer::ImGuiRenderer(vk::Format target_format)
{
  createDescriptorPool();

  context = ImGui::CreateContext();
  ImGui::SetCurrentContext(context);

  initImGui(target_format);

  IMGUI_CHECKVERSION();
}

<<<<<<< HEAD
PFN_vkVoidFunction vulkan_loader_function(const char* function_name, void*)
{
  return VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(
    etna::get_context().getInstance(), function_name);
=======
PFN_vkVoidFunction vulkanLoaderFunction(const char *function_name, void *)
{
  return VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(etna::get_context().getInstance(), function_name);
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
}

void ImGuiRenderer::createDescriptorPool()
{
<<<<<<< HEAD
  using Sz = vk::DescriptorPoolSize;
  using Type = vk::DescriptorType;

  std::array descrTypes{
    Sz{Type::eSampler, 1000},
    Sz{Type::eCombinedImageSampler, 1000},
    Sz{Type::eSampledImage, 1000},
    Sz{Type::eStorageImage, 1000},
    Sz{Type::eUniformTexelBuffer, 1000},
    Sz{Type::eStorageTexelBuffer, 1000},
    Sz{Type::eUniformBuffer, 1000},
    Sz{Type::eStorageBuffer, 1000},
    Sz{Type::eUniformBufferDynamic, 1000},
    Sz{Type::eStorageBufferDynamic, 1000},
    Sz{Type::eInputAttachment, 100}};

  vk::DescriptorPoolCreateInfo info{
    .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
    .maxSets = static_cast<std::uint32_t>(descrTypes.size() * 1000),
    .poolSizeCount = static_cast<std::uint32_t>(descrTypes.size()),
    .pPoolSizes = descrTypes.data(),
  };

  descriptorPool =
    etna::unwrap_vk_result(etna::get_context().getDevice().createDescriptorPoolUnique(info));
=======
  using Sz   = vk::DescriptorPoolSize;
  using Type = vk::DescriptorType;

  std::array descrTypes{
    Sz{ Type::eSampler, 1000 },
    Sz{ Type::eCombinedImageSampler, 1000 },
    Sz{ Type::eSampledImage, 1000 },
    Sz{ Type::eStorageImage, 1000 },
    Sz{ Type::eUniformTexelBuffer, 1000 },
    Sz{ Type::eStorageTexelBuffer, 1000 },
    Sz{ Type::eUniformBuffer, 1000 },
    Sz{ Type::eStorageBuffer, 1000 },
    Sz{ Type::eUniformBufferDynamic, 1000 },
    Sz{ Type::eStorageBufferDynamic, 1000 },
    Sz{ Type::eInputAttachment, 100 }
  };

  vk::DescriptorPoolCreateInfo info{
    .flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
    .maxSets       = descrTypes.size() * 1000,
    .poolSizeCount = descrTypes.size(),
    .pPoolSizes    = descrTypes.data(),
  };

  descriptorPool = etna::unwrap_vk_result(etna::get_context().getDevice().createDescriptorPoolUnique(info));
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
}

void ImGuiRenderer::initImGui(vk::Format a_target_format)
{
<<<<<<< HEAD
  const auto& ctx = etna::get_context();

  std::array targetFormats{static_cast<VkFormat>(a_target_format)};
  ImGui_ImplVulkan_InitInfo initInfo{
    .Instance = ctx.getInstance(),
    .PhysicalDevice = ctx.getPhysicalDevice(),
    .Device = ctx.getDevice(),
    .QueueFamily = ctx.getQueueFamilyIdx(),
    .Queue = ctx.getQueue(),
    .DescriptorPool = static_cast<VkDescriptorPool>(descriptorPool.get()),
    .RenderPass = VK_NULL_HANDLE,
    // This is basically unused
    .MinImageCount = 2,
    // This is mis-named
    .ImageCount =
      std::max(static_cast<uint32_t>(ctx.getMainWorkCount().multiBufferingCount()), uint32_t{2}),
    .MSAASamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
    .PipelineCache = VK_NULL_HANDLE,
    .Subpass = 0,
    .UseDynamicRendering = true,
    .PipelineRenderingCreateInfo =
      VkPipelineRenderingCreateInfoKHR{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .viewMask = 0,
        .colorAttachmentCount = static_cast<std::uint32_t>(targetFormats.size()),
        .pColorAttachmentFormats = targetFormats.data(),
        .depthAttachmentFormat = {},
        .stencilAttachmentFormat = {},
      },
    .Allocator = nullptr,
    .CheckVkResultFn = nullptr,
    .MinAllocationSize = 0,
  };

  ImGui_ImplVulkan_LoadFunctions(&vulkan_loader_function);
  ImGui_ImplVulkan_Init(&initInfo);
=======
  const auto &ctx = etna::get_context();

  std::array targetFormats{ static_cast<VkFormat>(a_target_format) };
  ImGui_ImplVulkan_InitInfo init_info{
    .Instance                    = ctx.getInstance(),
    .PhysicalDevice              = ctx.getPhysicalDevice(),
    .Device                      = ctx.getDevice(),
    .QueueFamily                 = ctx.getQueueFamilyIdx(),
    .Queue                       = ctx.getQueue(),
    .DescriptorPool              = descriptorPool.get(),
    .RenderPass                  = VK_NULL_HANDLE,
    // This is basically unused
    .MinImageCount               = 2,
    // This is mis-named
    .ImageCount                  = std::max(static_cast<uint32_t>(ctx.getMainWorkCount().multiBufferingCount()), uint32_t{2}),
    .MSAASamples                 = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
    .PipelineCache               = VK_NULL_HANDLE,
    .Subpass                     = 0,
    .UseDynamicRendering         = true,
    .PipelineRenderingCreateInfo = VkPipelineRenderingCreateInfoKHR{
      .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .pNext                   = nullptr,
      .viewMask                = 0,
      .colorAttachmentCount    = static_cast<std::uint32_t>(targetFormats.size()),
      .pColorAttachmentFormats = targetFormats.data(),
      .depthAttachmentFormat   = {},
      .stencilAttachmentFormat = {},
    },
    .Allocator         = nullptr,
    .CheckVkResultFn   = nullptr,
    .MinAllocationSize = 0,
  };

  ImGui_ImplVulkan_LoadFunctions(vulkanLoaderFunction);
  ImGui_ImplVulkan_Init(&init_info);
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)

  // Upload GUI fonts texture
  ImGui_ImplVulkan_CreateFontsTexture();
}

void ImGuiRenderer::nextFrame()
{
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
}

<<<<<<< HEAD
void ImGuiRenderer::render(
  vk::CommandBuffer cmd_buf,
  vk::Rect2D rect,
  vk::Image image,
  vk::ImageView image_view,
  ImDrawData* im_draw_data)
{
  ETNA_PROFILE_GPU(cmd_buf, renderGui)

  etna::RenderTargetState renderTargets(
    cmd_buf,
    rect,
    {{.image = image, .view = image_view, .loadOp = vk::AttachmentLoadOp::eLoad}},
    {});
=======
void ImGuiRenderer::render(vk::CommandBuffer cmd_buf, vk::Rect2D rect, vk::Image image, vk::ImageView image_view, ImDrawData *im_draw_data)
{
  etna::RenderTargetState renderTargets(cmd_buf, rect, { { .image = image, .view = image_view, .loadOp = vk::AttachmentLoadOp::eLoad } }, {});
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)

  ImGui_ImplVulkan_RenderDrawData(im_draw_data, cmd_buf);
}

void ImGuiRenderer::cleanupImGui()
{
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
}

ImGuiRenderer::~ImGuiRenderer()
{
  cleanupImGui();
}
