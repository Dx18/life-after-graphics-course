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
  void doCubemapPass(
    vk::CommandBuffer command_buffer, vk::Image target_image, vk::ImageView target_image_view);
  void doSpherePass(vk::CommandBuffer command_buffer);
  void doSceneBlendPass(
    vk::CommandBuffer command_buffer, vk::Image target_image, vk::ImageView target_image_view);

private:
  enum struct DiffuseMode
  {
    DYNAMIC,
    SPHERICAL_HARMONICS,
  };

  // Cubemap

  etna::Image cubemap;

  // Main target texture (used for temporal accummulation)

  etna::Image mainTarget;

  // Sphere samples

  etna::Buffer sphereSamples;

  // Diffuse spherical harmonics

  etna::Buffer diffuseSphericalHarmonics;

  // Default sampler

  etna::Sampler defaultSampler;

  // Camera parameters

  glm::mat4 viewProjection;

  Camera camera;

  // Other parameters

  glm::uint time{0};

  DiffuseMode diffuseMode{DiffuseMode::SPHERICAL_HARMONICS};

  // Pipelines

  etna::GraphicsPipeline cubemapPipeline;

  etna::GraphicsPipeline dynamicDiffusePipeline;
  etna::GraphicsPipeline sphericalHarmonicsDiffusePipeline;

  etna::GraphicsPipeline sceneBlendPipeline;

  // Late resource initialization

  // bool isFirstFrame{true};

  class ResourcesPrepareContext
  {
  public:
    ResourcesPrepareContext(
      const etna::Image& cubemap,
      const etna::Buffer& sphere_samples,
      const etna::Buffer& diffuse_spherical_harmonics);

    void prepareResources();

  private:
    // Default sampler

    etna::Sampler defaultSampler;

    // Cubemap

    const etna::Image& cubemap;

    // Cubemap filling resources

    etna::Buffer cubemapStagingBuffer;

    // Sphere samples

    const etna::Buffer& sphereSamples;

    // Sphere samples filling resources

    etna::Buffer sphereSamplesStagingBuffer;

    // Diffuse spherical harmonics

    const etna::Buffer& diffuseSphericalHarmonics;

    // Diffuse spherical harmonics filling resources

    etna::ComputePipeline diffuseSphericalHarmonicsBakePipeline;

    etna::Buffer diffuseSphericalHarmonicSamplesStagingBuffer;
    etna::Buffer diffuseSphericalHarmonicSamples;

    void fillCubemap(vk::CommandBuffer command_buffer);
    void fillSphereSamples(vk::CommandBuffer command_buffer);
    void bakeDiffuseSphericalHarmonics(vk::CommandBuffer command_buffer);
  };

  std::optional<ResourcesPrepareContext> prepareContext;

  void initCubemapPipeline(vk::Format swapchain_format);

  void initDynamicDiffusePipeline();
  void initSphericalHarmonicsDiffusePipeline();

  void initSceneBlendPipeline(vk::Format swapchain_format);

  // Framebuffer size

  glm::uvec2 resolution;
};
