import argparse
import sys
import os
from pathlib import Path, PurePath
import subprocess

TESTS_DIR = Path(__file__).parent / "tests"
os.makedirs(TESTS_DIR, exist_ok=True)

OUTPUT_DIR = Path(__file__).parent / "output"
os.makedirs(OUTPUT_DIR, exist_ok=True)

RESET_COLOR = "\x1b[0m"
GREEN = "\x1b[32m"
RED = "\x1b[31m"


class Test:
    name: str
    rel_path: PurePath
    full_path: Path
    expected_build_exit_code: int = 0
    expected_exit_code: int = 0
    expected_stdout: str = ""
    expected_stderr: str = ""

    def __init__(self, rel_path: PurePath):
        """
        Read a test from a .sasc file given its path relative to TESTS_DIR
        """
        self.rel_path = rel_path
        self.name = rel_path.as_posix()

        self.full_path = TESTS_DIR / rel_path
        if not self.full_path.is_file():
            print(
                f'Invalid test "{self.name}": Unable to find file at "{self.full_path}"'
            )
            sys.exit(1)

        try:
            with open(self.full_path, "r") as f:
                line = f.readline()
                while line.startswith("//!"):
                    pair = line[3:].lstrip().rstrip("\n").split("=")
                    key = pair[0].lstrip()
                    val = pair[1]
                    match key:
                        case "BUILD_EXIT_CODE":
                            self.expected_build_exit_code = int(val)
                        case "EXIT_CODE":
                            self.expected_exit_code = int(val)
                        case "STDOUT":
                            self.expected_stdout = val
                        case "STDERR":
                            self.expected_stdout = val
                    line = f.readline()
        except FileNotFoundError:
            print(
                f'Invalid test "{self.name}": Unable to open file at {self.expected_path}'
            )
            sys.exit(1)

    def output_path(self) -> Path:
        out = OUTPUT_DIR / self.rel_path.with_suffix("")
        os.makedirs(out, exist_ok=True)
        return out

    def run(self, compiler_path: Path, compiler_verbosity: int) -> tuple[bool, str]:
        output_dir = self.output_path()
        build_output_dir = output_dir / "build"
        os.makedirs(build_output_dir, exist_ok=True)

        # building command
        with (
            open(build_output_dir / "stdout.txt", "w") as stdout_f,
            open(build_output_dir / "stderr.txt", "w") as stderr_f,
        ):
            build_res = subprocess.run(
                [compiler_path, self.full_path, *(["-v"] * compiler_verbosity)],
                stdout=stdout_f,
                stderr=stderr_f,
            )

        if build_res.returncode != self.expected_build_exit_code:
            # build failed
            return (False, "failed, unexpected build exit code")

        return (False, "cant run yet")


if __name__ == "__main__":
    arg_parser = argparse.ArgumentParser(description="Test runner")
    arg_parser.add_argument("compiler_path", help="Path of compiler binary to test")
    arg_parser.add_argument(
        "--compiler-verbosity",
        type=int,
        default=1,
        metavar="N",
        help="Verbosity setting when setting compiler (default: 1)",
    )
    args = arg_parser.parse_args()

    # validating args
    compiler_path = Path(args.compiler_path)
    if not compiler_path.is_file():
        print(f'Could not find file: "{compiler_path}"')
        sys.exit(1)
    compiler_verbosity = args.compiler_verbosity

    print(f"Reading tests from {TESTS_DIR}")
    tests = [Test(f.relative_to(TESTS_DIR)) for f in TESTS_DIR.rglob("*.sasc")]

    print(f"Running {len(tests)} test{'s' if len(tests) > 1 else ''}")

    successes = 0
    for test in tests:
        print(f" {test.name} -> ", end="", flush=True)
        success, msg = test.run(compiler_path, compiler_verbosity)
        print((GREEN if success else RED) + msg + RESET_COLOR)
        if success:
            successes += 1
    print(
        f"Testing complete\n{successes}/{len(tests)} tests passing ({100.0 * float(successes) / float(len(tests)):.2f}%)"
    )

    sys.exit(0 if successes == len(tests) else 1)
