#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

#include "SurfacePointInfo.h"


layout(location = 0) in VS_OUT
{
  vec3 wPos;
  vec3 wNorm;
  vec3 wTangent;
  vec2 texCoord;
} surf;

layout(location = 0) out vec4 out_channel0;
layout(location = 1) out vec4 out_channel1;
layout(location = 2) out vec4 out_channel2;

layout(push_constant) uniform params_t {
  mat4 mProjView;
  mat4 mModel;
  vec4 baseColorMetalnessFactor;
  vec4 emissionRoughnessFactors;
} params;

layout(binding = 0, set = 0) uniform sampler2D baseColorTexture;
layout(binding = 1, set = 0) uniform sampler2D metalnessRoughnessTexture;
layout(binding = 2, set = 0) uniform sampler2D normalTexture;
layout(binding = 3, set = 0) uniform sampler2D occlusionTexture;
layout(binding = 4, set = 0) uniform sampler2D emissionTexture;

void main()
{
  vec3 albedo = texture(baseColorTexture, surf.texCoord).rgb * params.baseColorMetalnessFactor.rgb;

  // out_baseColor = vec4(albedo, 1.0);

  vec3 normal = normalize(surf.wNorm);
  vec3 tangent = normalize(surf.wTangent - dot(surf.wTangent, normal) * normal);
  vec3 bitangent = cross(normal, tangent);

  vec3 actualNormalLocal = texture(normalTexture, surf.texCoord).xyz * 2.0 - 1.0;
  vec3 actualNormal = mat3(tangent, bitangent, normal) * actualNormalLocal;

  // out_normal = vec4((actualNormal + 1.0) / 2.0, 1.0);

  vec3 emission = texture(emissionTexture, surf.texCoord).rgb * params.emissionRoughnessFactors.rgb;

  // out_emission = vec4(emission, 1.0);

  float occlusion = texture(occlusionTexture, surf.texCoord).r;
  vec2 metalnessRoughness = texture(metalnessRoughnessTexture, surf.texCoord).bg * vec2(params.baseColorMetalnessFactor.a, params.emissionRoughnessFactors.a);

  // out_occlusionMetalnessRoughness = vec4(occlusion, metalnessRoughness, 1.0);

  SurfacePointInfo info;

  info.albedo = albedo;
  info.normal = actualNormal;

  info.occlusion = occlusion;
  info.metalness = metalnessRoughness.r;
  info.roughness = metalnessRoughness.g;

  info.emission = emission;

  vec4 channels[3] = packSurfacePointInfo(info);

  out_channel0 = channels[0];
  out_channel1 = channels[1];
  out_channel2 = channels[2];
}
