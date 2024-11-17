#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require


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

void main()
{
  out_color = texture(cubemap, fIn.wNorm);
}
