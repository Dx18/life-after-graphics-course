#pragma once

#include <glm/glm.hpp>

#include <tiny_gltf.h>

#include <etna/BlockingTransferHelper.hpp>
#include <etna/OneShotCmdMgr.hpp>
#include <etna/Image.hpp>
#include <etna/Sampler.hpp>


enum struct ImageIndex : std::uint32_t
{
};

enum struct SamplerIndex : std::uint32_t
{
};


struct Texture
{
  ImageIndex image;
  SamplerIndex sampler;
};

enum struct TextureIndex : std::uint32_t
{
};

struct Material
{
  std::optional<TextureIndex> baseColorTexture;
  glm::vec4 baseColor{glm::vec4(1.0, 1.0, 1.0, 1.0)};

  std::optional<TextureIndex> metallicRoughnessTexture;
  float metallicFactor{0.0};
  float roughnessFactor{0.0};

  std::optional<TextureIndex> normalTexture;
  std::optional<TextureIndex> occlusionTexture;

  std::optional<TextureIndex> emissiveTexture;
  glm::vec4 emissiveFactor{0.0};
};

enum struct MaterialIndex : std::uint32_t
{
};

class SceneMaterials
{
public:
  void load(const tinygltf::Model& model, etna::OneShotCmdMgr& one_shot_cmd_mgr);

  const etna::Image& getImage(ImageIndex image_index) const;
  const etna::Sampler& getSampler(SamplerIndex sampler_index) const;
  const Texture& getTexture(TextureIndex texture_index) const;
  const Material& getMaterial(MaterialIndex material_index) const;

private:
  std::vector<etna::Image> images;
  std::vector<etna::Sampler> samplers;
  std::vector<Texture> textures;

  std::vector<Material> materials;
};
