#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

#include "unpack_attributes.glsl"


layout(location = 0) in vec4 vPosNorm;
layout(location = 1) in vec4 vTexCoord;

layout(location = 0) out VS_OUT
{
  vec3 wPos;
  vec3 wNorm;
  vec2 texCoord;
} vOut;

layout(push_constant) uniform params_t
{
  mat3x4 modelTransposed;
  vec4 baseColorMetalnessFactor;
  vec4 emissionRoughnessFactors;
} params;

layout(binding = 0, set = 0) uniform frameParams_t
{
  mat4 viewProjection;
  vec4 cameraPosition;
} frameParams;

void main()
{
  const vec4 wNorm = vec4(decode_normal(floatBitsToInt(vPosNorm.w)), 0.0);

  mat4x3 modelPre = transpose(params.modelTransposed);

  mat4 model;
  model[0] = vec4(modelPre[0], 0.0);
  model[1] = vec4(modelPre[1], 0.0);
  model[2] = vec4(modelPre[2], 0.0);
  model[3] = vec4(modelPre[3], 1.0);

  vOut.wPos = (model * vec4(vPosNorm.xyz, 1.0f)).xyz;
  vOut.wNorm = normalize(mat3(transpose(inverse(model))) * wNorm.xyz);
  vOut.texCoord = vTexCoord.xy;

  gl_Position = frameParams.viewProjection * vec4(vOut.wPos, 1.0);
}
