#ifndef SAMPLE_COUNT
#error "SAMPLE_COUNT must be defined in order to use the environment diffuse implementation"
#endif
#ifndef SAMPLE_DIRECTIONS_ARRAY
#error                                                                                             \
  "SAMPLE_DIRECTIONS_ARRAY must be defined in order to use the environment diffuse implementation"
#endif


#include "generated/RandomTable.inc"

const float PI = 3.14159265359;

uint randomUint(uint seed)
{
  return randomTable_values[seed % randomTable_valueCount];
}

vec3 calculateEnvironmentDiffuse(vec3 normal, int sampleCount, uint seed, samplerCube cubemap)
{
  vec3 finalColor = vec3(0.0);

  vec3 refDir = SAMPLE_DIRECTIONS_ARRAY[randomUint(seed) % SAMPLE_COUNT].xyz;
  if (length(cross(normal, refDir)) < 1e-7)
  {
    refDir = SAMPLE_DIRECTIONS_ARRAY[(randomUint(seed) + 1) % SAMPLE_COUNT].xyz;
  }
  vec3 axis = normalize(cross(normal, refDir));
  vec3 perp = cross(axis, normal);

  float cosAngle = clamp(dot(normal, refDir), -1.0, 1.0);
  float sinAngle = sqrt(clamp(1.0 - cosAngle * cosAngle, 0.0, 1.0));

  for (int i = 0; i < sampleCount; ++i)
  {
    vec3 dir =
      SAMPLE_DIRECTIONS_ARRAY[randomUint(seed + 53 * i + seed * seed + 31) % SAMPLE_COUNT].xyz;

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
