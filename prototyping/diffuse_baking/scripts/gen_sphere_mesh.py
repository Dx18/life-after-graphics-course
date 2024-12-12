import argparse
import io
import math


class Sphere:
    vertices: list[(float, float, float)]
    indices: list[int]

    def __init__(
            self, vertices: list[(float, float, float)],
            indices: list[int]
    ):
        self.vertices = vertices
        self.indices = indices


def gen_sphere_mesh(
        resolution_latitude: int, resolution_longitude: int,
) -> Sphere:
    vertices = [(0, -1, 0), (0, 1, 0)]

    for i in range(resolution_latitude - 1):
        latitude = math.pi * (
            (i + 1) / (resolution_latitude + 1) - 0.5
        )
        y = math.sin(latitude)

        for j in range(resolution_longitude):
            longitude = 2 * math.pi * j / resolution_longitude
            x = math.sin(longitude) * math.cos(latitude)
            z = math.cos(longitude) * math.cos(latitude)

            vertices.append((x, y, z))

    indices = []

    for j in range(resolution_longitude):
        indices.append(0)
        indices.append(2 + (j + 1) % resolution_longitude)
        indices.append(2 + j)

    for i in range(resolution_latitude - 2):
        for j in range(resolution_longitude):
            indices.append(2 + i * resolution_longitude + j)
            indices.append(2 + i * resolution_longitude + (j + 1) % resolution_longitude)
            indices.append(2 + (i + 1) * resolution_longitude + (j + 1) % resolution_longitude)
            indices.append(2 + i * resolution_longitude + j)
            indices.append(2 + (i + 1) * resolution_longitude + (j + 1) % resolution_longitude)
            indices.append(2 + (i + 1) * resolution_longitude + j)

    for j in range(resolution_longitude):
        indices.append(2 + (resolution_latitude - 2) * resolution_longitude + j)
        indices.append(2 + (resolution_latitude - 2) * resolution_longitude + (j + 1) % resolution_longitude)
        indices.append(1)

    return Sphere(vertices, indices)


def write_output_glsl(
        output_file: io.TextIOWrapper,
        sphere: Sphere, args: argparse.Namespace,
):
    vertices_string = ", ".join(map(
        lambda v: f"vec3({v[0]}, {v[1]}, {v[2]})", sphere.vertices
    ))
    output_file.write(
        f"const vec3 {args.vertex_array_glsl_const_name}[] = vec3[]({vertices_string});\n"
    )

    indices_string = ", ".join(map(str, sphere.indices))
    output_file.write(
        f"const uint {args.index_array_glsl_const_name}[] = uint[]({indices_string});\n"
    )


def write_output_cpp(
        output_file: io.TextIOWrapper,
        sphere: Sphere, args: argparse.Namespace,
):
    output_file.write(
        "#pragma once\n"
        "\n"
        "#include <cstdint>\n"
        "\n"
        f"namespace generated::{args.cpp_namespace_name} {{\n"
        "\n"
        f"const std::uint32_t {args.latitude_resolution_cpp_const_name} = {args.latitude_resolution};\n"
        f"const std::uint32_t {args.longitude_resolution_cpp_const_name} = {args.longitude_resolution};\n"
        "\n"
        "}\n"
    )


def main():
    parser = argparse.ArgumentParser(
        prog="gen_sphere_mesh.py",
    )

    parser.add_argument(
        "latitude_resolution", type=int,
        help="Number of segments into which a meridian is split",
        metavar="latitude-resolution",
    )
    parser.add_argument(
        "longitude_resolution", type=int,
        help="Number of segments into which a parallel is split",
        metavar="longitude-resolution",
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

    parser.add_argument(
        "--vertex-array-glsl-const-name", type=str,
        default="sphereMeshVertices",
        help="Name of the vertex array (for GLSL header)",
    )
    parser.add_argument(
        "--index-array-glsl-const-name", type=str,
        default="sphereMeshIndices",
        help="Name of the vertex array (for GLSL header)",
    )
    parser.add_argument(
        "--latitude-resolution-cpp-const-name", type=str,
        default="LATITUDE_RESOLUTION",
        help="Name of the latitude resolution constant (for C++ header)",
    )
    parser.add_argument(
        "--longitude-resolution-cpp-const-name", type=str,
        default="LONGITUDE_RESOLUTION",
        help="Name of the longitude resolution constant (for C++ header)",
    )
    parser.add_argument(
        "--cpp-namespace-name", type=str,
        default="sphere_mesh",
        help="Name of the namespace in the generated C++ header",
    )

    args = parser.parse_args()

    sphere = gen_sphere_mesh(
        args.latitude_resolution, args.longitude_resolution
    )

    with open(args.output_file_glsl, "w") as output_file:
        write_output_glsl(output_file, sphere, args)

    with open(args.output_file_cpp, "w") as output_file:
        write_output_cpp(output_file, sphere, args)


if __name__ == "__main__":
    main()
