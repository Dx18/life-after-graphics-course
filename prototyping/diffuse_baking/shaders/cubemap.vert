#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out VS_OUT
{
  vec3 texCoord;
}
vOut;

layout(push_constant) uniform params_t
{
  mat4 viewProjection;
}
params;

const vec3 vertices[] = vec3[](
  vec3(-1.0, -1.0, -1.0),
  vec3(-1.0, -1.0, 1.0),
  vec3(-1.0, 1.0, -1.0),
  vec3(-1.0, 1.0, 1.0),
  vec3(1.0, -1.0, -1.0),
  vec3(1.0, -1.0, 1.0),
  vec3(1.0, 1.0, -1.0),
  vec3(1.0, 1.0, 1.0));

const uint indices[] = uint[](
  0, 2, 3, 0, 3, 1,
  4, 5, 7, 4, 7, 6,
  0, 1, 5, 0, 5, 4,
  2, 6, 7, 2, 7, 3,
  0, 4, 6, 0, 6, 2,
  1, 3, 7, 1, 7, 5);

void main()
{
  vec3 vertex = vertices[indices[gl_VertexIndex]];

  gl_Position = params.viewProjection * vec4(vertex, 1.0);

  vOut.texCoord = vertex;
}
