#pragma once

#include <etna/Vulkan.hpp>
#include <etna/GraphicsPipeline.hpp>
#include <etna/Image.hpp>
#include <etna/Sampler.hpp>


/**
 * Simple class for displaying a texture on the screen for debug purposes.
 */
class QuadRenderer
{
public:
  struct CreateInfo
  {
    vk::Format format = vk::Format::eUndefined;
<<<<<<< HEAD
    vk::Rect2D rect = {};
  };

  explicit QuadRenderer(CreateInfo info);
  ~QuadRenderer() {}

  void render(
    vk::CommandBuffer cmd_buff,
    vk::Image target_image,
    vk::ImageView target_image_view,
    const etna::Image& tex_to_draw,
    const etna::Sampler& sampler);
=======
    vk::Rect2D rect   = {};
  };

  QuadRenderer(CreateInfo info);
  ~QuadRenderer() {}

  void render(vk::CommandBuffer cmd_buff, vk::Image target_image, vk::ImageView target_image_view,
                      const etna::Image &tex_to_draw, const etna::Sampler &sampler);
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)

private:
  etna::GraphicsPipeline pipeline;
  etna::ShaderProgramId programId;
<<<<<<< HEAD
  vk::Rect2D rect{};

  QuadRenderer(const QuadRenderer&) = delete;
  QuadRenderer& operator=(const QuadRenderer&) = delete;
=======
  vk::Rect2D rect {};

  QuadRenderer(const QuadRenderer &) = delete;
  QuadRenderer& operator=(const QuadRenderer &) = delete;
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
};
