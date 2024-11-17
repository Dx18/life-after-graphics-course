#pragma once

#include <glm/glm.hpp>

#include <etna/Image.hpp>
#include <etna/Sampler.hpp>
#include <etna/Buffer.hpp>
#include <etna/GraphicsPipeline.hpp>
#include <etna/ComputePipeline.hpp>

#include "wsi/Keyboard.hpp"

#include "FramePacket.hpp"


/**
 * The meat of the sample. All things you see on the screen are contained within this class.
 * This what you want to change and expand between different samples.
 */
class DiffuseBakingDemoRenderer
{
public:
  DiffuseBakingDemoRenderer();

  void loadShaders();
  void allocateResources(glm::uvec2 swapchain_resolution);
  void setupPipelines(vk::Format swapchain_format);

  void debugInput(const Keyboard& kb);
  void update(const FramePacket& packet);
  void drawGui();
  void renderWorld(
    vk::CommandBuffer command_buffer, vk::Image target_image, vk::ImageView target_image_view);

private:
  void fillCubemap(vk::CommandBuffer command_buffer);
  void bakeDiffuseCubemap(vk::CommandBuffer command_buffer);

  void doCubemapPass(
    vk::CommandBuffer command_buffer, vk::Image target_image, vk::ImageView target_image_view);
  void doSpherePass(vk::CommandBuffer command_buffer);
  void doSceneBlendPass(
    vk::CommandBuffer command_buffer, vk::Image target_image, vk::ImageView target_image_view);

private:
  enum struct DiffuseMode
  {
    DYNAMIC,
    CUBEMAP,
    SPHERICAL_HARMONICS,
  };

  // Cubemap

  etna::Image cubemap;

  // Main target texture (used for temporal accummulation)

  etna::Image mainTarget;

  // Diffuse cubemap

  etna::Image diffuseCubemap;

  // Diffuse spherical harmonics

  etna::Buffer diffuseSphericalHarmonics;

  // Default sampler

  etna::Sampler defaultSampler;

  // Camera parameters

  glm::mat4 viewProjection;

  Camera camera;

  // Other parameters

  glm::uint time{0};

  DiffuseMode diffuseMode{DiffuseMode::CUBEMAP};

  // Pipelines

  etna::GraphicsPipeline cubemapPipeline;

  etna::GraphicsPipeline dynamicDiffusePipeline;
  etna::GraphicsPipeline cubemapDiffusePipeline;
  etna::GraphicsPipeline sphericalHarmonicsDiffusePipeline;

  etna::GraphicsPipeline sceneBlendPipeline;

  // Late resource initialization

  bool isFirstFrame{true};

  void initCubemapPipeline(vk::Format swapchain_format);

  void initDynamicDiffusePipeline();
  void initCubemapDiffusePipeline();
  void initSphericalHarmonicsDiffusePipeline();

  void initSceneBlendPipeline(vk::Format swapchain_format);

  // Framebuffer size

  glm::uvec2 resolution;
};
