import os
import shutil
import argparse

BUILD_DIR = f"build{os.sep}"

BUILD_CONFIGS = ["Debug", "Release", "RelWithDebInfo"]
CMAKE_GENERATOR = None  # "Ninja Multi-Config"
CMAKE_C_COMPILER = None  # "clang"
CMAKE_CXX_COMPILER = None  # "clang++"


def debug_print(*args, **kwargs):
    print("\033[92m", end="")
    print("===", *args, "===\033[0m", **kwargs)


def main(cmake, make, run, config, generator, args):
    if generator is not None:
        global CMAKE_GENERATOR
        CMAKE_GENERATOR = generator
    else:
        generator = "default generator" if CMAKE_GENERATOR is None else CMAKE_GENERATOR

    cmake_cmd = f"cd {BUILD_DIR} && cmake .."
    cmake_cmd += " " if CMAKE_GENERATOR is None else f' -G "{CMAKE_GENERATOR}"'
    cmake_cmd += " " if CMAKE_C_COMPILER is None else f" -DCMAKE_C_COMPILER={CMAKE_C_COMPILER}"
    cmake_cmd += " " if CMAKE_CXX_COMPILER is None else f" -DCMAKE_CXX_COMPILER={CMAKE_CXX_COMPILER}"

    if cmake:
        debug_print(f"Running CMake inside {BUILD_DIR} with {generator}")
        shutil.rmtree(BUILD_DIR, ignore_errors=True)
        os.mkdir(BUILD_DIR)
        if (err := os.system(cmake_cmd)):
            raise Exception(f"error code = {err}")

    if make:
        debug_print(f"Building glow with {config} config")
        if (err := os.system(f"cmake --build {BUILD_DIR} --config {config}")):
            raise Exception(f"error code = {err}")

    if run:
        def exec(exe):
            debug_print(f"Running {exe}")
            if (err := os.system(f"{exe} {' '.join(args)}")):
                raise Exception(f"error code = {err}")
        try:
            exec(f".{os.sep}{BUILD_DIR}{config}{os.sep}glow.exe")
        except:
            exec(f".{os.sep}{BUILD_DIR}glow.exe")


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
        help="Build a glow executable",
    )
    parser.add_argument(
        "--run", "-r",
        action="store_true",
        help="Run a glow executable",
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

    args, glow_args = parser.parse_known_args()

    if not args.cmake and not args.make and not args.run and not args.test:
        debug_print("Doing nothing, no options passed (try using --help).")
    if glow_args:
        debug_print(f"glow args: {glow_args}")

    main(
        args.cmake,
        args.make,
        args.run,
        args.config,
        args.generator,
        glow_args,
    )
