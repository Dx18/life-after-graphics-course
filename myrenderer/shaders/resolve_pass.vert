#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require


layout(location = 0) out VS_OUT
{
  vec2 texCoord;
}
vOut;

const vec2 vertices[] = vec2[](vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0), vec2(1.0, -1.0));

const uint indices[] = uint[](0, 1, 2, 0, 2, 3);

void main()
{
  vec2 vertex = vertices[indices[gl_VertexIndex]];

  gl_Position = vec4(vertex, 0.0, 1.0);

  vOut.texCoord = (vertex + 1.0) / 2.0;
}
