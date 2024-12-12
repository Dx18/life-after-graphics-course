#ifndef SURFACE_POINT_INFO_H
#define SURFACE_POINT_INFO_H

const uint MATERIAL_TYPE_EMISSIVE = 0;
const uint MATERIAL_TYPE_METALLIC = 1;

struct SurfacePointInfo
{
  uint materialType;

  vec3 albedo;
  vec3 normal;

  float occlusion;
  float roughness;

  float metalness;

  vec3 emission;

  vec3 position;
};

struct CameraInfo
{
  vec3 position;

  vec3 viewDir;
  vec3 viewUpDir;

  float aspect;

  float zNear;
  float zFar;
};

SurfacePointInfo unpackSurfacePointInfo(
  vec2 texCoord, vec4 channels[3], float depth, CameraInfo cameraInfo)
{
  SurfacePointInfo info;

  info.materialType = channels[1].w == 0.0 ? MATERIAL_TYPE_EMISSIVE : MATERIAL_TYPE_METALLIC;

  info.albedo = channels[0].xyz;
  info.normal = channels[1].xyz * 2.0 - 1.0;

  info.occlusion = channels[0].w;
  info.roughness = channels[2].w;

  if (info.materialType == MATERIAL_TYPE_EMISSIVE)
  {
    info.metalness = 0.0;
    info.emission = channels[2].xyz;
  }
  else
  {
    info.metalness = channels[2].x;
    info.emission = vec3(0.0);
  }

  float zLocal = cameraInfo.zNear * cameraInfo.zFar /
    (cameraInfo.zFar + depth * (cameraInfo.zNear - cameraInfo.zFar));

  vec3 viewDownVec = -cameraInfo.viewUpDir;
  vec3 viewRightVec = cross(cameraInfo.viewDir, cameraInfo.viewUpDir) * cameraInfo.aspect;

  vec3 position = cameraInfo.position +
    (cameraInfo.viewDir + viewRightVec * (texCoord.x * 2.0 - 1.0) +
     viewDownVec * (texCoord.y * 2.0 - 1.0)) *
      zLocal;

  info.position = position;

  return info;
}

vec4[3] packSurfacePointInfo(SurfacePointInfo info)
{
  vec4 channels[3];

  if (info.materialType == MATERIAL_TYPE_EMISSIVE)
  {
    channels[0] = vec4(info.albedo, info.occlusion);
    channels[1] = vec4((info.normal + 1.0) / 2.0, 0.0);
    channels[2] = vec4(info.emission, info.roughness);
  }
  else
  {
    channels[0] = vec4(info.albedo, info.occlusion);
    channels[1] = vec4((info.normal + 1.0) / 2.0, 1.0);
    channels[2] = vec4(info.metalness, 0.0, 0.0, info.roughness);
  }

  return channels;
}

#endif
