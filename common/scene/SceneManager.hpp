#pragma once

#include <filesystem>

#include <glm/glm.hpp>
<<<<<<< HEAD

#include <tiny_gltf.h>

#include <etna/Buffer.hpp>
#include <etna/BlockingTransferHelper.hpp>
#include <etna/VertexInput.hpp>
#include <etna/Sampler.hpp>

#include "SceneLights.hpp"
#include "SceneMaterials.hpp"
#include "SceneMeshes.hpp"

=======
#include <tiny_gltf.h>
#include <etna/Buffer.hpp>
#include <etna/BlockingTransferHelper.hpp>
#include <etna/VertexInput.hpp>
#include <etna/Sampler.hpp>

#include "SceneLights.hpp"
#include "SceneMaterials.hpp"
#include "SceneMeshes.hpp"

<<<<<<< HEAD
// A single render element (relem) corresponds to a single draw call
// of a certain pipeline with specific bindings (including material data)
struct RenderElement
{
	std::uint32_t vertexOffset;
	std::uint32_t indexOffset;
	std::uint32_t indexCount;
  // Not implemented!
  // Material* material;
};

// A mesh is a collection of relems. A scene may have the same mesh
// located in several different places, so a scene consists of **instances**,
// not meshes.
struct Mesh
{
  std::uint32_t firstRelem;
  std::uint32_t relemCount;
};
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
=======
>>>>>>> f2882a9 (Added task 1)

class SceneManager
{
public:
  SceneManager();

  void selectScene(std::filesystem::path path);

<<<<<<< HEAD
<<<<<<< HEAD
  std::span<const glm::mat4x4> getInstanceMatrices() { return instanceMatrices; }

  const SceneMeshes& getMeshes() const { return meshes; }
  const SceneLights& getLights() const { return lights; }
  const SceneMaterials& getMaterials() const { return materials; }
=======
  // Every instance is a mesh drawn with a certain transform
  // NOTE: maybe you can pass some additional data through unused matrix entries?
=======
>>>>>>> f2882a9 (Added task 1)
  std::span<const glm::mat4x4> getInstanceMatrices() { return instanceMatrices; }

<<<<<<< HEAD
  // Every mesh is a collection of relems
  std::span<const Mesh> getMeshes() { return meshes; }

  // Every relem is a single draw call
  std::span<const RenderElement> getRenderElements() { return renderElements; }

  vk::Buffer getVertexBuffer() { return unifiedVbuf.get(); }
  vk::Buffer getIndexBuffer() { return unifiedIbuf.get(); }

  etna::VertexByteStreamFormatDescription getVertexFormatDescription();
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
=======
  const SceneMeshes& getMeshes() const { return meshes; }
  const SceneLights& getLights() const { return lights; }
  const SceneMaterials& getMaterials() const { return materials; }
>>>>>>> f2882a9 (Added task 1)

private:
  std::optional<tinygltf::Model> loadModel(std::filesystem::path path);

<<<<<<< HEAD
<<<<<<< HEAD
  std::vector<glm::mat4> processInstanceMatrices(const tinygltf::Model& model) const;
=======
  struct ProcessedInstances
  {
    std::vector<glm::mat4x4> matrices;
    std::vector<std::uint32_t> meshes;
  };

  ProcessedInstances processInstances(const tinygltf::Model& model) const;

  struct Vertex
  {
    // First 3 floats are position, 4th float is a packed normal
    glm::vec4 positionAndNormal;
    // First 2 floats are tex coords, 3rd is a packed tangent, 4th is padding
    glm::vec4 texCoordAndTangentAndPadding;
  };

	static_assert(sizeof(Vertex) == sizeof(float) * 8);

  struct ProcessedMeshes
  {
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<RenderElement> relems;
    std::vector<Mesh> meshes;
  };
  ProcessedMeshes processMeshes(const tinygltf::Model& model) const;
  void uploadData(std::span<const Vertex> vertices, std::span<const std::uint32_t>);
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
=======
  std::vector<glm::mat4> processInstanceMatrices(const tinygltf::Model& model) const;
>>>>>>> f2882a9 (Added task 1)

private:
  tinygltf::TinyGLTF loader;
  std::unique_ptr<etna::OneShotCmdMgr> oneShotCommands;
  etna::BlockingTransferHelper transferHelper;

<<<<<<< HEAD
<<<<<<< HEAD
  std::vector<glm::mat4> instanceMatrices;

  SceneMeshes meshes;
  SceneLights lights;
  SceneMaterials materials;
=======
	std::vector<RenderElement> renderElements;
  std::vector<Mesh> meshes;
  std::vector<glm::mat4x4> instanceMatrices;
  std::vector<std::uint32_t> instanceMeshes;

  etna::Buffer unifiedVbuf;
  etna::Buffer unifiedIbuf;
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
=======
  std::vector<glm::mat4> instanceMatrices;

  SceneMeshes meshes;
  SceneLights lights;
  SceneMaterials materials;
>>>>>>> f2882a9 (Added task 1)
};
