#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require


layout(location = 0) in VS_OUT
{
  vec2 texCoord;
}
fIn;

layout(location = 0) out vec4 out_fragColor;

layout(binding = 0, set = 0) uniform sampler2D sceneTexture;

void main()
{
  out_fragColor = texture(sceneTexture, fIn.texCoord);
}
