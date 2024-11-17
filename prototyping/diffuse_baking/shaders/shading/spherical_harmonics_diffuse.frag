#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require


#include "generated/SphericalHarmonicSamples.inc"


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

layout(binding = 0, set = 0, std430) readonly buffer coefficients_t
{
  vec4
    coefficients[(sphericalHarmonicSamples_maxBand + 1) * (sphericalHarmonicSamples_maxBand + 1)];
};

vec3 evaluateSphericalHarmonic(vec3 normal)
{
  vec3 result = vec3(0.0);

  float cosTheta = normal.z;

  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

  float cosPhi = clamp(normal.x / sinTheta, -1.0, 1.0);
  float sinPhi = clamp(normal.y / sinTheta, -1.0, 1.0);

  float phi = acos(cosPhi) * (sinPhi < 0.0 ? -1.0 : 1.0);

  uint alpCoefficientOffset = 0;
  uint shCoefficientOffset = 0;
  uint coefficientOffset = 0;

  for (int band = 0; band <= sphericalHarmonicSamples_maxBand; ++band)
  {
    {
      // float alpValue = sphericalHarmonicSamples_alpCoefficients[alpCoefficientOffset];

      float alpValue = 0.0;

      float currArg = 1.0;
      for (int i = 0; i <= band; ++i)
      {
	alpValue += sphericalHarmonicSamples_alpCoefficients[alpCoefficientOffset + i] * currArg;
	currArg *= cosTheta;
      }

      float shCoefficient = sphericalHarmonicSamples_shCoefficients[shCoefficientOffset];

      result += shCoefficient * alpValue * coefficients[coefficientOffset + band].rgb;

      alpCoefficientOffset += band + 1;
    }

    for (int absNum = 1; absNum <= band; ++absNum)
    {
      float alpValue = 0.0;

      float currArg = 1.0;
      for (int i = 0; i <= band - absNum; ++i)
      {
	alpValue += sphericalHarmonicSamples_alpCoefficients[alpCoefficientOffset + i] * currArg;
	currArg *= cosTheta;
      }

      alpValue *= pow(sinTheta, absNum / 2.0);

      float shCoefficient =
        sphericalHarmonicSamples_shCoefficients[shCoefficientOffset + absNum - 1];

      result += shCoefficient * cos(absNum * phi) * alpValue *
          coefficients[coefficientOffset + band + absNum].rgb +
        shCoefficient * sin(absNum * phi) * alpValue *
          coefficients[coefficientOffset + band - absNum].rgb;

      alpCoefficientOffset += band - absNum + 1;
    }

    shCoefficientOffset += band + 1;
    coefficientOffset += 2 * band + 1;
  }

  return result;
}

void main()
{
  out_color = vec4(evaluateSphericalHarmonic(normalize(fIn.wNorm)), 1.0);
}
