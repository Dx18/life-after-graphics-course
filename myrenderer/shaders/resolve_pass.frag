#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

#include "FinitePointLight.h"
#include "InfinitePointLight.h"
#include "DirectionalLight.h"
#include "AmbientLight.h"
#include "SurfacePointInfo.h"
#include "BRDF.h"


layout(location = 0) in VS_OUT
{
  vec2 texCoord;
}
fIn;

layout(location = 0) out vec4 out_fragColor;

layout(push_constant) uniform params_t
{
  vec4 cameraPositionAspect;
  vec4 cameraViewDirZNear;
  vec4 cameraViewUpDirZFar;
}
params;

layout(binding = 0, set = 0, std140) readonly buffer lightsData_t
{
  uint finitePointLightCount;
  uint infinitePointLightCount;
  uint directionalLightCount;
  uint ambientLightCount;

  vec4 lights[];
}
lightsData;

layout(binding = 1, set = 0) uniform sampler2D channel0;
layout(binding = 2, set = 0) uniform sampler2D channel1;
layout(binding = 3, set = 0) uniform sampler2D channel2;
layout(binding = 4, set = 0) uniform sampler2D depthTexture;

vec3 getFinitePointLightColor(uint offset, SurfacePointInfo info, vec3 cameraDir)
{
  FinitePointLight light =
    FinitePointLight(lightsData.lights[offset], lightsData.lights[offset + 1]);

  vec3 lightVec = light.positionRange.xyz - info.position;
  float lightDistance = length(lightVec);
  vec3 lightDir = normalize(lightVec);

  float attenuation = clamp(1.0 - pow(lightDistance / light.positionRange.w, 4.0), 0.0, 1.0) /
    (lightDistance * lightDistance);

  return brdf(lightDir, cameraDir, info) * attenuation * info.occlusion * light.color.rgb;
}

vec3 getInfinitePointLightColor(uint offset, SurfacePointInfo info, vec3 cameraDir)
{
  InfinitePointLight light =
    InfinitePointLight(lightsData.lights[offset], lightsData.lights[offset + 1]);

  vec3 lightVec = light.position.xyz - info.position;
  float lightDistance = length(lightVec);
  vec3 lightDir = normalize(lightVec);

  float attenuation = 1.0 / (lightDistance * lightDistance);

  return brdf(lightDir, cameraDir, info) * attenuation * info.occlusion * light.color.rgb;
}

vec3 getDirectionalLightColor(uint offset, SurfacePointInfo info, vec3 cameraDir)
{
  DirectionalLight light =
    DirectionalLight(lightsData.lights[offset], lightsData.lights[offset + 1]);

  vec3 lightDir = light.direction.xyz;

  return brdf(lightDir, cameraDir, info) * info.occlusion * light.color.rgb;
}

vec3 getAmbientLightColor(uint offset, SurfacePointInfo info, vec3 cameraDir)
{
  AmbientLight light = AmbientLight(lightsData.lights[offset]);

  return brdf(info.normal, cameraDir, info) * info.occlusion * light.color.rgb;
}

vec3 getColor(SurfacePointInfo info, vec3 cameraDir)
{
  vec3 result = vec3(0.0);

  uint offset = 0;

  for (uint i = 0; i < lightsData.finitePointLightCount; ++i)
  {
    result += getFinitePointLightColor(offset, info, cameraDir);
    offset += 2;
  }

  for (uint i = 0; i < lightsData.infinitePointLightCount; ++i)
  {
    result += getInfinitePointLightColor(offset, info, cameraDir);
    offset += 2;
  }

  for (uint i = 0; i < lightsData.directionalLightCount; ++i)
  {
    result += getDirectionalLightColor(offset, info, cameraDir);
    offset += 2;
  }

  for (uint i = 0; i < lightsData.ambientLightCount; ++i)
  {
    result += getAmbientLightColor(offset, info, cameraDir);
    ++offset;
  }

  result += info.emission * length(info.emission) / sqrt(3.0);

  return result;
}

void main()
{
  float depth = texture(depthTexture, fIn.texCoord).r;

  SurfacePointInfo info = unpackSurfacePointInfo(
    fIn.texCoord,
    vec4[](
      texture(channel0, fIn.texCoord),
      texture(channel1, fIn.texCoord),
      texture(channel2, fIn.texCoord)),
    depth,
    CameraInfo(
      params.cameraPositionAspect.xyz,
      params.cameraViewDirZNear.xyz,
      params.cameraViewUpDirZFar.xyz,
      params.cameraPositionAspect.w,
      params.cameraViewDirZNear.w,
      params.cameraViewUpDirZFar.w));

  info.normal = normalize(info.normal);

  vec3 cameraDir = normalize(params.cameraPositionAspect.xyz - info.position);

  out_fragColor = vec4(getColor(info, cameraDir), 1.0);
}
