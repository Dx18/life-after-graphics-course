#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

#include "SurfacePointInfo.h"


layout(location = 0) in VS_OUT
{
  vec3 wPos;
  vec3 wNorm;
  vec2 texCoord;
}
surf;

layout(location = 0) out vec4 out_channel0;
layout(location = 1) out vec4 out_channel1;
layout(location = 2) out vec4 out_channel2;

layout(push_constant) uniform params_t
{
  mat3x4 modelTransposed;
  vec4 baseColorMetalnessFactor;
  vec4 emissionRoughnessFactors;
}
params;

layout(binding = 0, set = 0) uniform frameParams_t
{
  mat4 viewProjection;
  vec4 cameraPosition;
}
frameParams;

layout(binding = 1, set = 0) uniform sampler2D baseColorTexture;
layout(binding = 2, set = 0) uniform sampler2D metalnessRoughnessTexture;
layout(binding = 3, set = 0) uniform sampler2D normalTexture;
layout(binding = 4, set = 0) uniform sampler2D occlusionTexture;
layout(binding = 5, set = 0) uniform sampler2D emissionTexture;

mat3 cotangentFrame(vec3 normal, vec3 position, vec2 texCoord)
{
  vec3 dPositionX = dFdx(position);
  vec3 dPositionY = dFdy(position);
  vec2 dTexCoordX = dFdx(texCoord);
  vec2 dTexCoordY = dFdy(texCoord);

  vec3 dPositionYPerp = cross(dPositionY, normal);
  vec3 dPositionXPerp = cross(normal, dPositionX);
  vec3 tangent = dPositionYPerp * dTexCoordX.x + dPositionXPerp * dTexCoordY.x;
  vec3 bitangent = dPositionYPerp * dTexCoordX.y + dPositionXPerp * dTexCoordY.y;

  float invmax = inversesqrt(max(dot(tangent, tangent), dot(bitangent, bitangent)));

  return mat3(tangent * invmax, bitangent * invmax, normal);
}

void main()
{
  vec3 albedo = texture(baseColorTexture, surf.texCoord).rgb * params.baseColorMetalnessFactor.rgb;

  vec3 normal = normalize(surf.wNorm);

  vec3 cameraPosition = frameParams.cameraPosition.xyz;

  vec3 actualNormalLocal = texture(normalTexture, surf.texCoord).xyz * 2.0 - 1.0;
  vec3 actualNormal =
    cotangentFrame(normal, surf.wPos - cameraPosition, surf.texCoord) * actualNormalLocal;

  vec3 emission = texture(emissionTexture, surf.texCoord).rgb * params.emissionRoughnessFactors.rgb;

  float occlusion = texture(occlusionTexture, surf.texCoord).r;
  vec2 metalnessRoughness = texture(metalnessRoughnessTexture, surf.texCoord).bg *
    vec2(params.baseColorMetalnessFactor.a, params.emissionRoughnessFactors.a);

  SurfacePointInfo info;

  info.materialType =
    length(emission) > 0.07 ? MATERIAL_TYPE_EMISSIVE : MATERIAL_TYPE_METALLIC;

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
