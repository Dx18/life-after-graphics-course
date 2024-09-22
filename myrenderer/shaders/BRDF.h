#ifndef BRDF_COMMON_H
#define BRDF_COMMON_H

#include "SurfacePointInfo.h"


// Shamelessly stolen from https://learnopengl.com/PBR/Theory

const float PI = 3.14159265359;

float distributionGgx(vec3 dirIn, vec3 dirOut, vec3 normal, float roughness)
{
  float roughness2 = roughness * roughness;

  vec3 h = normalize(dirIn + dirOut);

  float d = max(0.0, dot(normal, h));

  float denominator = d * d * (roughness2 * roughness2 - 1.0) + 1.0;
  denominator *= denominator;

  return roughness2 * roughness2 / denominator / PI;
}

float geometrySchlickGgx(vec3 v, vec3 normal, float roughness)
{
  float d = max(0.0, dot(normal, v));

  float k = (roughness + 1.0) * (roughness + 1.0) / 8;

  return d / (d * (1.0 - k) + k);
}

float geometrySmith(vec3 dirIn, vec3 dirOut, vec3 normal, float roughness)
{
  float ggx1 = geometrySchlickGgx(dirIn, normal, roughness);
  float ggx2 = geometrySchlickGgx(dirOut, normal, roughness);

  return ggx1 * ggx2;
}

vec3 fresnelSchlick(vec3 dirIn, vec3 dirOut, vec3 normal, vec3 albedo, float metalness)
{
  vec3 f0 = mix(vec3(0.04), albedo, metalness);

  vec3 h = normalize(dirIn + dirOut);

  float d = max(0.0, dot(h, dirOut));

  return f0 + (1.0 - f0) * pow(clamp(1.0 - d, 0.0, 1.0), 5.0);
}

float brdfSpecular(vec3 dirIn, vec3 dirOut, SurfacePointInfo info)
{
  float d = distributionGgx(dirIn, dirOut, info.normal, info.roughness);
  float g = geometrySmith(dirIn, dirOut, info.normal, info.roughness);

  float numerator = d * g;
  float denominator =
    4.0 * max(0.0, dot(dirIn, info.normal)) * max(0.0, dot(dirOut, info.normal)) + 0.01;

  return numerator / denominator;
}

vec3 brdfDiffuse(vec3 dirIn, vec3 dirOut, SurfacePointInfo info)
{
  return (1.0 - info.metalness) * info.albedo / PI;
}

vec3 brdf(vec3 dirIn, vec3 dirOut, SurfacePointInfo info)
{
  vec3 f = fresnelSchlick(dirIn, dirOut, info.normal, info.albedo, info.metalness);

  float specular = brdfSpecular(dirIn, dirOut, info);
  vec3 diffuse = brdfDiffuse(dirIn, dirOut, info);

  return (f * specular + (1.0 - f) * diffuse) * max(0.0, dot(dirIn, info.normal));
}

#endif
