#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in VS_OUT
{
  vec3 texCoord;
}
fIn;

layout(location = 0) out vec4 out_color;

layout(binding = 0, set = 0) uniform samplerCube cubemap;

void main()
{
  out_color = texture(cubemap, fIn.texCoord);
}
