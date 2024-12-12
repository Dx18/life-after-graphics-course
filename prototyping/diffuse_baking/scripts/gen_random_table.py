import argparse
import io
import random


GLSL_CONST_NAME_VALUE_COUNT = "randomTable_valueCount"
GLSL_CONST_NAME_VALUES = "randomTable_values"


def write_output_glsl(
        output_file: io.TextIOWrapper,
        values: list[int], args: argparse.Namespace,
):
    values_string = ", ".join(map(str, values))

    output_file.write(
        f"const uint {GLSL_CONST_NAME_VALUE_COUNT} = {len(values)};\n"
        f"const uint {GLSL_CONST_NAME_VALUES}[] = uint[]({values_string});\n"
    )


def main():
    parser = argparse.ArgumentParser(
        prog="gen_random_table.py",
    )

    parser.add_argument(
        "count", type=int,
        help="Number of values to generate",
    )
    parser.add_argument(
        "output_file_glsl", type=str,
        help="The output file for GLSL",
        metavar="output-file-glsl",
    )

    args = parser.parse_args()

    values = [
        random.randint(0, 2 ** 32 - 1) for _ in range(args.count)
    ]

    with open(args.output_file_glsl, "w") as output_file:
        write_output_glsl(output_file, values, args)


if __name__ == "__main__":
    main()
