import os
import time
import shutil
import argparse

NAME = "glow"
BUILD_DIR = f"build{os.sep}"

CCXX_COMPILER = {"cl":"cl", "gcc":"g++", "clang":"clang++"}

BUILD_CONFIGS = ["Debug", "Release", "RelWithDebInfo"]
CMAKE_GENERATOR = None  # "Ninja"
CMAKE_C_COMPILER = None  # "clang"
CMAKE_CXX_COMPILER = None  # "clang++"


start = time.time()

def debug_print(*args, **kwargs):
    if args:
        print("\033[92m", end="")
        print(f"Δt: {(time.time() - start):.4f}s")
        print("===", *args, "===\033[0m", **kwargs)
    else:
        print(f"\033[92mΔt: {(time.time() - start):.4f}s\033[0m")


def main(cmake, make, run, config, generator, warnings, prog_args):
    if generator is not None:
        global CMAKE_GENERATOR
        CMAKE_GENERATOR = generator
    else:
        generator = "default generator" if CMAKE_GENERATOR is None else CMAKE_GENERATOR

    if cmake:
        cmake_cmd = f"cd {BUILD_DIR} && cmake .."
        cmake_cmd += " " if CMAKE_GENERATOR is None else f' -G "{CMAKE_GENERATOR}"'
        cmake_cmd += " " if CMAKE_C_COMPILER is None else f" -DCMAKE_C_COMPILER={CMAKE_C_COMPILER}"
        cmake_cmd += " " if CMAKE_CXX_COMPILER is None else f" -DCMAKE_CXX_COMPILER={CMAKE_CXX_COMPILER}"
        cmake_cmd += f" -D{NAME.upper()}_ENABLE_WARNINGS={'ON' if warnings else 'OFF'}"

        debug_print(f"Running CMake inside {BUILD_DIR} with {generator}")
        shutil.rmtree(BUILD_DIR, ignore_errors=True)
        os.mkdir(BUILD_DIR)
        if (err := os.system(cmake_cmd)):
            raise Exception(f"error code = {err}")

    if make:
        debug_print(f"Building {NAME} with {config} config")
        if (err := os.system(f"cmake --build {BUILD_DIR} --config {config}")):
            raise Exception(f"error code = {err}")

    if run:
        def exec(exe):
            debug_print(f"Running {exe}")
            if (err := os.system(f"{exe} {' '.join(prog_args)}")):
                raise Exception(f"error code = {err}")
        for build_dir in [f"bin{os.sep}{config}{os.sep}", f"{config}{os.sep}", f"bin{os.sep}", ""]:
            exe = f".{os.sep}{BUILD_DIR}{build_dir}{NAME}.exe"
            if os.path.exists(exe):
                exec(exe)
                break
        else:
            raise Exception(f"could not find {NAME}.exe file location")
    else:
        debug_print()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="")

    parser.add_argument(
        "--cmake", "-c",
        action="store_true",
        help="(Re)generate the build folder and run CMake inside",
    )
    parser.add_argument(
        "--make", "-m",
        action="store_true",
        help=f"Build a {NAME} executable",
    )
    parser.add_argument(
        "--run", "-r",
        action="store_true",
        help=f"Run a {NAME} executable",
    )
    parser.add_argument(
        "--config", "-cfg",
        nargs="?",
        choices=BUILD_CONFIGS,
        default=BUILD_CONFIGS[-1],  # RelWithDebInfo
        help="Set the build config used for --make and --run  (default: %(default)s)",
    )
    parser.add_argument(
        "--generator", "-G",
        type=str,
        help="Specify a build system generator used with --cmake",
    )
    parser.add_argument(
        "--warnings", "-W",
        action="store_true",
        help="Enable compiler warnings",
    )
    parser.add_argument(
        "--compiler", "-cc",
        type=str,
        choices=CCXX_COMPILER.keys(),
        help="Specify a C/C++ compiler",
    )

    args, prog_args = parser.parse_known_args()

    if not args.cmake and not args.make and not args.run:
        debug_print("Doing nothing, no options passed (try using --help).")

    if args.compiler:
        CMAKE_C_COMPILER = args.compiler
        CMAKE_CXX_COMPILER = CCXX_COMPILER[args.compiler]

    if prog_args:
        debug_print(f"{NAME} args: {prog_args}")

    main(
        args.cmake,
        args.make,
        args.run,
        args.config,
        args.generator,
        args.warnings,
        prog_args,
    )
