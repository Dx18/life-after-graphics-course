#pragma once

#include <glm/glm.hpp>

#include <etna/Image.hpp>
#include <etna/Sampler.hpp>
#include <etna/Buffer.hpp>
#include <etna/GraphicsPipeline.hpp>

#include "render_utils/LineRenderer.hpp"
#include "scene/SceneManager.hpp"
#include "render_utils/QuadRenderer.hpp"
#include "wsi/Keyboard.hpp"

#include "FramePacket.hpp"


/**
 * The meat of the sample. All things you see on the screen are contained within this class.
 * This what you want to change and expand between different samples.
 */
class WorldRenderer
{
public:
  WorldRenderer();

  void loadScene(std::filesystem::path path);

  void loadShaders();
  void allocateResources(glm::uvec2 swapchain_resolution);
  void setupPipelines(vk::Format swapchain_format);

  void debugInput(const Keyboard& kb);
  void update(const FramePacket& packet);
  void drawGui();
  void renderWorld(
    vk::CommandBuffer cmd_buf, vk::Image target_image, vk::ImageView target_image_view);

private:
  struct MaterialBindings
  {
    etna::ImageBinding baseColorBinding;
    etna::ImageBinding metallicRoughnessBinding;
    etna::ImageBinding normalBinding;
    etna::ImageBinding occlusionBinding;
    etna::ImageBinding emissiveBinding;
  };

  struct MaterialConstants
  {
    glm::vec4 baseColorMetallicFactor;
    glm::vec4 emissiveRoughnessFactors;
  };

  etna::DescriptorSet createMaterialBindings(
    vk::CommandBuffer command_buffer, std::optional<std::size_t> material_index) const;

  MaterialConstants getMaterialConstants(std::optional<std::size_t> material_index) const;

  void doGeometryPass(vk::CommandBuffer cmd_buf);
  void doGBufferPass(vk::CommandBuffer cmd_buf);
  void doResolvePass(
    vk::CommandBuffer cmd_buf, vk::Image target_image, vk::ImageView target_image_view);

private:
  // Scene manager

  std::unique_ptr<SceneManager> sceneMgr;

  // "Default" resources

  etna::Sampler defaultSampler;

  etna::Image defaultBaseColorImage;
  etna::Image defaultMetalnessRoughnessImage;
  etna::Image defaultNormalImage;
  etna::Image defaultOcclusionImage;
  etna::Image defaultEmissionImage;

  // Co;or attachments for light passes

  struct GBuffer
  {
    std::array<etna::Image, 3> channels;
    etna::Image depth;
  };

  GBuffer gBuffer;

  // Camera parameters

  glm::mat4 viewProjection;

  Camera camera;

  // Pipelines

  etna::GraphicsPipeline geometryPassPipeline;
  etna::GraphicsPipeline gBufferPassPipeline;
  etna::GraphicsPipeline resolvePassPipeline;

  void initGeometryPassPipeline();
  void initGBufferPassPipeline();
  void initResolvePassPipeline(vk::Format swapchain_format);

  // Debug stuff

  enum struct DebugQuadMode
  {
    kDisabled,
    kBaseColor,
    kNormal,
    kEmissive,
    kOcclusionMetallicRoughness,
    kDepth,
  };

  std::unique_ptr<QuadRenderer> quadRenderer;
  DebugQuadMode debugQuadMode{DebugQuadMode::kDisabled};

  void initQuadRenderer(vk::Format swapchain_format);

  std::unique_ptr<LineRenderer> gridRenderer;
  bool drawDebugGrid = false;

  void initGridRenderer(vk::Format swapchain_format);

  // Framebuffer size

  glm::uvec2 resolution;
};
