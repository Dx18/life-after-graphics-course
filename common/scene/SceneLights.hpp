#pragma once

#include <tiny_gltf.h>

#include <etna/Buffer.hpp>
#include <etna/BlockingTransferHelper.hpp>

#include "scene/light/FinitePointLight.h"
#include "scene/light/DirectionalLight.h"
#include "scene/light/InfinitePointLight.h"
#include "scene/light/AmbientLight.h"


using KnownLightTypes =
  std::tuple<FinitePointLight, InfinitePointLight, DirectionalLight, AmbientLight>;

namespace detail
{

template <typename F, std::size_t... Indices>
void for_each_known_light_type_impl(F&& func, std::index_sequence<Indices...>)
{
  (func.template operator()<Indices, std::tuple_element_t<Indices, KnownLightTypes>>(), ...);
}

template <template <std::size_t, typename> typename F, typename I, typename K>
struct ForEachKnownLightTypeImpl;

template <template <std::size_t, typename> typename F, std::size_t... Indices, typename... L>
struct ForEachKnownLightTypeImpl<F, std::index_sequence<Indices...>, std::tuple<L...>>
{
  using Type = std::tuple<typename F<Indices, L>::Type...>;
};

} // namespace detail

template <typename F>
void for_each_known_light_type(F&& func)
{
  return detail::for_each_known_light_type_impl(
    std::forward<F>(func), std::make_index_sequence<std::tuple_size_v<KnownLightTypes>>{});
}

template <template <std::size_t, typename> typename F>
using ForEachKnownLightType = detail::ForEachKnownLightTypeImpl<
  F,
  std::make_index_sequence<std::tuple_size_v<KnownLightTypes>>,
  KnownLightTypes>;

class SceneLights
{
public:
  void load(
    std::span<const glm::mat4> instance_matrices,
    const tinygltf::Model& model,
    etna::BlockingTransferHelper& transfer_helper,
    etna::OneShotCmdMgr& one_shot_cmd_mgr);

  const etna::Buffer& getLightsDataBuffer() const;

private:
  etna::Buffer lightsDataBuffer;
};
