#ifndef DIFFUSE_BAKING_ENVIRONMENT_DIFFUSE_H
#define DIFFUSE_BAKING_ENVIRONMENT_DIFFUSE_H


#include "generated/RandomTable.inc"
#include "generated/SphereSamples.inc"

const float PI = 3.14159265359;

uint randomUint(uint seed)
{
  return randomTable_values[seed % randomTable_valueCount];
}

/* mat3 cotangentFrame(vec3 normal) */
/* { */
/*   vec3 tangent = normalize(vec3(-normal.z, 0.0, normal.x)); */
/*   vec3 cotangent = cross(normal, tangent); */

/*   return mat3(tangent, cotangent, normal); */
/* } */

/* float unitIntervalRandom(uint seed) */
/* { */
/*   uint value = (randomTable[seed % 4096 / 4] >> (8 * (seed % 4))) & 0xFF; */

/*   return value / float(0xFF); */
/* } */

/* vec3 calculateEnvironmentDiffuse(vec3 normal, int sampleCount, uint seed, samplerCube cubemap) */
/* { */
/*   mat3 frame = cotangentFrame(normalize(normal)); */

/*   { */
/*     float frameAngle = unitIntervalRandom(seed * 29) * 2.0 * PI; */

/*     vec3 tangent = frame[0]; */
/*     vec3 cotangent = frame[1]; */

/*     frame[0] = tangent * cos(frameAngle) + cotangent * sin(frameAngle); */
/*     frame[1] = -tangent * sin(frameAngle) + cotangent * cos(frameAngle); */
/*   } */

/*   vec3 finalColor = vec3(0.0); */

/*   for (int i = 0; i < sampleCount; ++i) */
/*   { */
/*     float u = unitIntervalRandom(seed + 53 * i + 31); */
/*     float v = unitIntervalRandom(seed + 67 * i + 15); */

/*     float sinLatitude = sqrt(u); */
/*     float cosLatitude = sqrt(1.0 - u); */

/*     float longitude = v * 2.0 * PI; */

/*     vec3 dir = */
/*       frame * vec3(cos(longitude) * cosLatitude, sin(longitude) * cosLatitude, sinLatitude); */

/*     finalColor += texture(cubemap, dir).rgb * sinLatitude; */
/*   } */

/*   finalColor /= sampleCount; */

/*   return finalColor; */
/* } */

vec3 calculateEnvironmentDiffuse(vec3 normal, int sampleCount, uint seed, samplerCube cubemap)
{
  vec3 finalColor = vec3(0.0);

  vec3 refDir = sphereSamples_sampleDirections[randomUint(seed) % sphereSamples_sampleCount];
  vec3 axis = normalize(cross(normal, refDir));
  vec3 perp = cross(axis, normal);

  float cosAngle = dot(normal, refDir);
  float sinAngle = sqrt(1.0 - cosAngle * cosAngle);

  for (int i = 0; i < sampleCount; ++i)
  {
    vec3 dir = sphereSamples_sampleDirections
      [randomUint(seed + 53 * i + seed * seed + 31) % sphereSamples_sampleCount];

    vec3 dirLocal = vec3(dot(dir, normal), dot(dir, perp), dot(dir, axis));
    vec3 dirLocalRotated = vec3(
      dirLocal.x * cosAngle + dirLocal.y * sinAngle,
      -dirLocal.x * sinAngle + dirLocal.y * cosAngle,
      dirLocal.z);

    vec3 dirRotated = mat3(normal, perp, axis) * dirLocalRotated;

    if (dot(dirRotated, normal) < 0.0)
    {
      dirRotated *= -1.0;
    }

    finalColor += texture(cubemap, dirRotated).rgb * dot(dirRotated, normal);
  }

  finalColor /= sampleCount;

  return finalColor;
}

#endif
