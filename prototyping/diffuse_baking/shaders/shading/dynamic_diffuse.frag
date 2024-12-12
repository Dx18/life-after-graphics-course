#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require


#include "generated/SphereSamples.inc"

layout(location = 0) in VS_OUT
{
  vec3 wNorm;
}
fIn;

layout(location = 0) out vec4 out_color;

layout(push_constant) uniform params_t
{
  mat4 viewProjection;
  vec4 cameraPositionTime;
}
params;

layout(binding = 0, set = 0) uniform samplerCube cubemap;

layout(binding = 1, set = 0, std140) uniform samples_t
{
  vec4 sampleDirections[sphereSamples_sampleCount];
}
samples;


#define SAMPLE_COUNT sphereSamples_sampleCount
#define SAMPLE_DIRECTIONS_ARRAY samples.sampleDirections
#include "../impl/EnvironmentDiffuse.inc"


uint getSeed()
{
  uint timeBits = floatBitsToUint(params.cameraPositionTime.w);
  uint bits1 = floatBitsToUint(fIn.wNorm.x);
  uint bits2 = floatBitsToUint(fIn.wNorm.y);
  uint bits3 = floatBitsToUint(fIn.wNorm.z);

  return bits1 * 1001459 + bits2 * 1553 + bits3 + 523 + timeBits * 1091 +
    timeBits * timeBits * 10891;
}

void main()
{
  out_color = vec4(calculateEnvironmentDiffuse(normalize(fIn.wNorm), 32, getSeed(), cubemap), 1.0);
}
