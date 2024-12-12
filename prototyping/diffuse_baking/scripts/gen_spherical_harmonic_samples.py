import argparse
import io
import math
import random

import gen_sphere_samples


def multiply_polynomials(a: slice, b: slice) -> list[float]:
    result = []
    for deg in range(len(a) + len(b) - 1):
        coeff = 0
        for deg_a in range(deg + 1):
            deg_b = deg - deg_a

            if deg_a >= len(a) or deg_b >= len(b):
                continue

            coeff += a[deg_a] * b[deg_b]
            
        result.append(coeff)

    return result


def add_polynomials(a: slice, b: slice) -> list[float]:
    return [
        (a[i] if i < len(a) else 0) + (b[i] if i < len(b) else 0)
        for i in range(max(len(a), len(b)))
    ]


def evaluate_polynomial(a: slice, x: float) -> float:
    value = 0
    curr_x = 1

    for i in len(polynomial):
        value += polynomial[i] * curr_x
        curr_x *= x

    return value


class AssociatedLegendrePolynomialsTable:
    max_band: int

    coefficients: list[float]

    def __init__(self, max_band: int):
        assert max_band >= 0

        # double_factorial[i] := (2 * i - 1)!!
        #
        # (2 * i - 1)!! = (2 * (i - 1) + 1)!! =
        #               = (2 * (i - 1) - 1)!! * (2 * (i - 1) + 1) =
        #               = (2 * (i - 1) - 1)!! * (2 * i - 1)
        double_factorial = [1]
        for i in range(1, max_band + 1):
            double_factorial.append(
                double_factorial[-1] * (2 * i - 1)
            )

        self.max_band = max_band
        self.coefficients = []

        for band in range(max_band + 1):
            for num in range(band + 1):
                if num <= band - 2:
                    new_polynomial = multiply_polynomials(
                        [1 / (band - num)],
                        add_polynomials(
                            multiply_polynomials(
                                [0, 2 * band - 1],
                                self.get_polynomial(band - 1, num),
                            ),
                            multiply_polynomials(
                                [1 - band - num],
                                self.get_polynomial(band - 2, num),
                            ),
                        ),
                    )
                elif num == band - 1:
                    new_polynomial = multiply_polynomials(
                        [0, 2 * num + 1],
                        self.get_polynomial(band - 1, num),
                    )
                else: # num == band
                    new_polynomial = [
                        (-1) ** (num % 2) * double_factorial[num]
                    ]

                assert len(new_polynomial) == band - num + 1
                    
                self.coefficients.extend(new_polynomial)


    def get_polynomial(self, band: int, num: int) -> slice:
        assert band <= self.max_band
        assert num <= band

        index = band * (band - 1) * (band + 4) // 6 + band + (2 * band + 3 - num) * num // 2

        return self.coefficients[index:index + band - num + 1]


    def evaluate(self, band: int, num: int, arg: float) -> float:
        polynomial = self.get_polynomial(band, num)

        value = 0
        curr_arg = 1

        for i in range(len(polynomial)):
            value += polynomial[i] * curr_arg
            curr_arg *= arg

        value *= (1.0 - arg * arg) ** (num / 2)

        return value


class SphericalHarmonicsTable:
    max_band: int

    alp_table: AssociatedLegendrePolynomialsTable

    coefficients: list[float]

    def __init__(self, max_band: int):
        assert max_band >= 0

        factorial = [1]
        for i in range(1, 2 * max_band + 1):
            factorial.append(factorial[-1] * i)
        
        self.max_band = max_band
        self.alp_table = AssociatedLegendrePolynomialsTable(max_band)
        self.coefficients = []

        for band in range(max_band + 1):
            for num in range(band + 1):
                self.coefficients.append(math.sqrt(
                    (1 if num == 0 else 2)
                    * (2 * band + 1) / (4 * math.pi)
                    * factorial[band - num] / factorial[band + num]
                ))


    def get_coefficient(self, band: int, num: int) -> float:
        assert band <= self.max_band
        assert abs(num) <= band

        return self.coefficients[band * (band + 1) // 2 + abs(num)]


    def evaluate(
            self, band: int, num: int, theta: float, phi: float
    ) -> float:
        if num > 0:
            return (
                self.get_coefficient(band, num)
                * math.cos(num * phi)
                * self.alp_table.evaluate(band, num, math.cos(theta))
            )
        elif num < 0:
            return (
                self.get_coefficient(band, num)
                * math.sin(-num * phi)
                * self.alp_table.evaluate(band, -num, math.cos(theta))
            )

        return (
            self.get_coefficient(band, 0)
            * self.alp_table.evaluate(band, 0, math.cos(theta))
        )


class SphericalHarmonicSample:
    parameters: (float, float)
    direction: (float, float, float)
    values: list[float]

    def __init__(
            self, parameters: (float, float),
            direction: (float, float, float),
            values: list[float],
    ):
        self.parameters = parameters
        self.direction = direction
        self.values = values


def generate_spherical_harmonic_samples(
        resolution: int,
        spherical_harmonics_table: SphericalHarmonicsTable,
) -> list[SphericalHarmonicSample]:
    sphere_samples = gen_sphere_samples.generate_sphere_samples(
        resolution
    )
    
    sampled_spherical_harmonics = [
        [
            spherical_harmonics_table.evaluate(band, num, theta, phi)
            for band in range(spherical_harmonics_table.max_band + 1)
            for num in range(-band, band + 1)
        ]
        for theta, phi in sphere_samples
    ]

    return [
        SphericalHarmonicSample(
            angles,
            (
                math.cos(angles[1]) * math.sin(angles[0]),
                math.sin(angles[1]) * math.sin(angles[0]),
                math.cos(angles[0]),
            ),
            values,
        )
        for angles, values in zip(
                sphere_samples, sampled_spherical_harmonics
        )
    ]


GLSL_CONST_NAME_MAX_BAND = "sphericalHarmonicSamples_maxBand"
GLSL_CONST_NAME_SAMPLE_COUNT = "sphericalHarmonicSamples_sampleCount"

GLSL_CONST_NAME_ALP_COEFFICIENTS = "sphericalHarmonicSamples_alpCoefficients"
GLSL_CONST_NAME_SH_COEFFICIENTS = "sphericalHarmonicSamples_shCoefficients"
# GLSL_CONST_NAME_SAMPLE_DIRECTIONS = "sphericalHarmonicSamples_sampleDirections"
# GLSL_CONST_NAME_PRECOMPUTED_VALUES = "sphericalHarmonicSamples_precomputedValues"


def write_output_glsl(
        output_file: io.TextIOWrapper,
        table: SphericalHarmonicsTable,
        samples: list[SphericalHarmonicSample],
        args: argparse.Namespace,
):
    alp_coefficients_string = ", ".join(map(
        str, table.alp_table.coefficients
    ))
    sh_coefficients_string = ", ".join(map(
        str, table.coefficients
    ))
    sample_directions_string = ", ".join(map(
        lambda d: f"vec3({d[0]}, {d[1]}, {d[2]})",
        map(lambda sample: sample.direction, samples),
    ))
    precomputed_values_string = ", ".join(sum(map(
            lambda values: list(map(str, values)),
            map(lambda sample: sample.values, samples),
    ), start=[]))

    output_file.write(
        f"const uint {GLSL_CONST_NAME_MAX_BAND} = {table.max_band};\n"
        f"const uint {GLSL_CONST_NAME_SAMPLE_COUNT} = {len(samples)};\n"
        f"const float {GLSL_CONST_NAME_ALP_COEFFICIENTS}[] = float[]({alp_coefficients_string});\n"
        f"const float {GLSL_CONST_NAME_SH_COEFFICIENTS}[] = float[]({sh_coefficients_string});\n"
        # f"const vec3 {GLSL_CONST_NAME_SAMPLE_DIRECTIONS}[] = vec3[]({sample_directions_string});\n"
        # f"const float {GLSL_CONST_NAME_PRECOMPUTED_VALUES}[] = float[]({precomputed_values_string});\n"
    )


CPP_NAMESPACE_NAME = "spherical_harmonic_samples"

CPP_CONST_NAME_MAX_BAND = "MAX_BAND"
CPP_CONST_NAME_SAMPLE_COUNT = "SAMPLE_COUNT"

CPP_CONST_NAME_SAMPLE_DIRECTIONS = "SAMPLE_DIRECTIONS"
CPP_CONST_NAME_PRECOMPUTED_VALUES = "PRECOMPUTED_VALUES"

    
def write_output_cpp(
        output_file: io.TextIOWrapper,
        table: SphericalHarmonicsTable,
        samples: list[SphericalHarmonicSample],
        args: argparse.Namespace,
):
    sample_directions_string = ", ".join(map(
        lambda d: f"glm::vec4({d[0]}, {d[1]}, {d[2]}, 1.0)",
        map(lambda sample: sample.direction, samples),
    ))
    precomputed_values_string = ", ".join(sum(map(
            lambda values: list(map(str, values)),
            map(lambda sample: sample.values, samples),
    ), start=[]))

    output_file.write(
        "#pragma once\n"
        "\n"
        "#include <cstdint>\n"
        "\n"
        "#include <glm/glm.hpp>\n"
        "\n"
        f"namespace generated::{CPP_NAMESPACE_NAME} {{\n"
        "\n"
        f"const std::uint32_t {CPP_CONST_NAME_MAX_BAND} = {table.max_band};\n"
        "\n"
        f"const std::uint32_t {CPP_CONST_NAME_SAMPLE_COUNT} = {len(samples)};\n"
        "\n"
        f"const std::array<glm::vec4, {len(samples)}> {CPP_CONST_NAME_SAMPLE_DIRECTIONS} = {{{sample_directions_string}}};\n"
        "\n"
        f"const std::array<float, {len(samples) * (table.max_band + 1) ** 2}> {CPP_CONST_NAME_PRECOMPUTED_VALUES} = {{{precomputed_values_string}}};\n"
        "\n"
        "}\n"
    )


def main():
    parser = argparse.ArgumentParser(
        prog="gen_spherical_harmonic_samples.py",
    )

    parser.add_argument(
        "resolution", type=int,
        help="Number of segments into which a meridian or a parallel is split",
    )
    parser.add_argument(
        "max_band", type=int,
        help="Max band of a spherical harmonic",
        metavar="max-band",
    )
    parser.add_argument(
        "output_file_glsl", type=str,
        help="The output file for GLSL",
        metavar="output-file-glsl",
    )
    parser.add_argument(
        "output_file_cpp", type=str,
        help="The output file for C++",
        metavar="output-file-cpp",
    )

    args = parser.parse_args()

    spherical_harmonics_table = SphericalHarmonicsTable(args.max_band)

    samples = generate_spherical_harmonic_samples(
        args.resolution, spherical_harmonics_table
    )

    with open(args.output_file_glsl, "w") as output_file:
        write_output_glsl(
            output_file, spherical_harmonics_table, samples, args
        )

    with open(args.output_file_cpp, "w") as output_file:
        write_output_cpp(
            output_file, spherical_harmonics_table, samples, args
        )


if __name__ == "__main__":
    main()
