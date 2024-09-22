#include "SceneLights.hpp"

#include <optional>

#include <etna/GlobalContext.hpp>


template <typename T>
struct GltfLightTryPack;

template <>
struct GltfLightTryPack<FinitePointLight>
{
  static std::optional<FinitePointLight> tryPack(
    glm::mat4 transform, const tinygltf::Light& gltf_light);
};

template <>
struct GltfLightTryPack<InfinitePointLight>
{
  static std::optional<InfinitePointLight> tryPack(
    glm::mat4 transform, const tinygltf::Light& gltf_light);
};

template <>
struct GltfLightTryPack<DirectionalLight>
{
  static std::optional<DirectionalLight> tryPack(
    glm::mat4 transform, const tinygltf::Light& gltf_light);
};

template <>
struct GltfLightTryPack<AmbientLight>
{
  static std::optional<AmbientLight> tryPack(
    glm::mat4 transform, const tinygltf::Light& gltf_light);
};

std::optional<FinitePointLight> GltfLightTryPack<FinitePointLight>::tryPack(
  glm::mat4 transform, const tinygltf::Light& gltf_light)
{
  if (gltf_light.type != "point" || gltf_light.range == 0.0)
  {
    return std::nullopt;
  }

  glm::vec3 position = transform * glm::vec4(glm::vec3(0.0), 1.0);
  glm::vec3 color = glm::vec3(gltf_light.color[0], gltf_light.color[1], gltf_light.color[2]);

  return FinitePointLight{
    .positionRange = glm::vec4(position, gltf_light.range),
    .colorIntensity = glm::vec4(color, gltf_light.intensity),
  };
}

std::optional<InfinitePointLight> GltfLightTryPack<InfinitePointLight>::tryPack(
  glm::mat4 transform, const tinygltf::Light& gltf_light)
{
  if (gltf_light.type != "point" || gltf_light.range != 0.0)
  {
    return std::nullopt;
  }

  glm::vec3 position = transform * glm::vec4(glm::vec3(0.0), 1.0);
  glm::vec3 color = glm::vec3(gltf_light.color[0], gltf_light.color[1], gltf_light.color[2]);

  return InfinitePointLight{
    .position = glm::vec4(position, 0.0),
    .colorIntensity = glm::vec4(color, gltf_light.intensity),
  };
}

std::optional<DirectionalLight> GltfLightTryPack<DirectionalLight>::tryPack(
  glm::mat4 transform, const tinygltf::Light& gltf_light)
{
  if (gltf_light.type != "directional")
  {
    return std::nullopt;
  }

  glm::vec3 direction = glm::mat3(transform) * glm::vec3(0.0, 0.0, -1.0);
  glm::vec3 color = glm::vec3(gltf_light.color[0], gltf_light.color[1], gltf_light.color[2]);

  return DirectionalLight{
    .direction = glm::vec4(direction, 0.0),
    .colorIntensity = glm::vec4(color, gltf_light.intensity),
  };
}

std::optional<AmbientLight> GltfLightTryPack<AmbientLight>::tryPack(
  glm::mat4, const tinygltf::Light& gltf_light)
{
  if (gltf_light.type != "ambient")
  {
    return std::nullopt;
  }

  glm::vec3 color = glm::vec3(gltf_light.color[0], gltf_light.color[1], gltf_light.color[2]);

  return AmbientLight{
    .colorIntensity = glm::vec4(color, gltf_light.intensity),
  };
}

template <std::size_t, typename L>
struct LightBufferData
{
  using Type = std::vector<L>;
};

void SceneLights::load(
  std::span<const glm::mat4> instance_matrices,
  const tinygltf::Model& model,
  etna::BlockingTransferHelper& transfer_helper,
  etna::OneShotCmdMgr& one_shot_cmd_mgr)
{
  ForEachKnownLightType<LightBufferData>::Type data;

  for (std::size_t i = 0; i < model.nodes.size(); ++i)
  {
    if (model.nodes[i].light < 0)
    {
      continue;
    }

    const tinygltf::Light& gltfLight = model.lights[model.nodes[i].light];

    for_each_known_light_type(
      [&instance_matrices, &data, i, &gltfLight]<std::size_t I, typename L>() {
        std::optional<L> lightPacked =
          GltfLightTryPack<L>::tryPack(instance_matrices[i], gltfLight);
        if (lightPacked.has_value())
        {
          std::get<I>(data).push_back(*lightPacked);
        }
      });
  }

  std::size_t bufferSize = std::tuple_size_v<KnownLightTypes> * sizeof(std::uint32_t);

  bufferSize += (16 - bufferSize % 16) % 16;

  for_each_known_light_type([&data, &bufferSize]<std::size_t I, typename L>() {
    bufferSize += std::span(std::get<I>(data)).size_bytes();
  });

  bufferSize += (16 - bufferSize % 16) % 16;

  std::vector<std::byte> lightsData(bufferSize);

  std::size_t offset = 0;

  for_each_known_light_type([&data, &lightsData, &offset]<std::size_t I, typename L>() {
    std::uint32_t* dataPtr = reinterpret_cast<std::uint32_t*>(lightsData.data() + offset);
    *dataPtr = std::get<I>(data).size();
    offset += sizeof(std::uint32_t);
  });

  offset += (16 - offset % 16) % 16;

  for_each_known_light_type([&data, &lightsData, &offset]<std::size_t I, typename L>() {
    L* dataPtr = reinterpret_cast<L*>(lightsData.data() + offset);

    std::copy(std::get<I>(data).begin(), std::get<I>(data).end(), dataPtr);

    offset += std::span(std::get<I>(data)).size_bytes();
  });

  lightsDataBuffer = etna::get_context().createBuffer(etna::Buffer::CreateInfo{
    .size = std::span(lightsData).size_bytes(),
    .bufferUsage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer,
    .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
    .name = "lights_data",
  });

  transfer_helper.uploadBuffer(one_shot_cmd_mgr, lightsDataBuffer, 0, lightsData);
}

const etna::Buffer& SceneLights::getLightsDataBuffer() const
{
  return lightsDataBuffer;
}
