import argparse
import io
import math
import random


def generate_sphere_samples(resolution: int) -> list[(float, float)]:
    result = []

    for theta_region in range(resolution):
        for phi_region in range(resolution):
            theta_sample = (
                theta_region + random.random()
            ) / resolution
            phi_sample = (
                phi_region + random.random()
            ) / resolution

            theta = 2 * math.acos(math.sqrt(1 - theta_sample))
            phi = 2 * math.pi * phi_sample

            result.append((theta, phi))

    return result


GLSL_CONST_NAME_SAMPLE_COUNT = "sphereSamples_sampleCount"


def write_output_glsl(
        output_file: io.TextIOWrapper,
        directions: list[(float, float, float)],
        args: argparse.Namespace,
):
    directions_string = ", ".join(map(
        lambda d: f"vec3({d[0]}, {d[1]}, {d[2]})", directions,
    ))

    output_file.write(
        f"const uint {GLSL_CONST_NAME_SAMPLE_COUNT} = {len(directions)};\n"
    )


CPP_NAMESPACE_NAME = "sphere_samples"

CPP_CONST_NAME_SAMPLE_COUNT = "SAMPLE_COUNT"

CPP_CONST_NAME_SAMPLE_DIRECTIONS = "SAMPLE_DIRECTIONS"


def write_output_cpp(
        output_file: io.TextIOWrapper,
        directions: list[(float, float, float)],
        args: argparse.Namespace,
):
    directions_string = ", ".join(map(
        lambda d: f"glm::vec4({d[0]}, {d[1]}, {d[2]}, 1.0)", directions,
    ))

    output_file.write(
        "#pragma once\n"
        "\n"
        "#include <cstdint>\n"
        "\n"
        "#include <glm/glm.hpp>\n"
        "\n"
        f"namespace generated::{CPP_NAMESPACE_NAME} {{\n"
        "\n"
        f"const std::uint32_t {CPP_CONST_NAME_SAMPLE_COUNT} = {len(directions)};\n"
        "\n"
        f"const std::array<glm::vec4, {len(directions)}> {CPP_CONST_NAME_SAMPLE_DIRECTIONS} = {{{directions_string}}};\n"
        "\n"
        "}\n"
    )


def main():
    parser = argparse.ArgumentParser(
        prog="gen_sphere_samples.py",
    )

    parser.add_argument(
        "resolution", type=int,
        help="Number of segments into which a meridian or a parallel is split",
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

    samples = generate_sphere_samples(args.resolution)

    directions = [
        (
            math.cos(phi) * math.sin(theta),
            math.sin(phi) * math.sin(theta),
            math.cos(theta),
        )
        for theta, phi in samples
    ]

    with open(args.output_file_glsl, "w") as output_file:
        write_output_glsl(output_file, directions, args)

    with open(args.output_file_cpp, "w") as output_file:
        write_output_cpp(output_file, directions, args)


if __name__ == "__main__":
    main()
