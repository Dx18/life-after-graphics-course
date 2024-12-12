#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require


layout(location = 0) out VS_OUT
{
  vec3 wNorm;
}
vOut;

layout(push_constant) uniform params_t
{
  mat4 viewProjection;
  vec4 cameraPositionTime;
}
params;

#include "generated/SphereMesh.inc"

void main()
{
  vec3 vertex = sphereMeshVertices[sphereMeshIndices[gl_VertexIndex]];

  gl_Position = params.viewProjection * vec4(vertex, 1.0);

  vOut.wNorm = vertex;
}
