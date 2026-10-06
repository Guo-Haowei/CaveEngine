# scripts/ci/build.py

from pathlib import Path

from scripts.ci.ci_framework import finish, run_task
from scripts.common.log import Log


ROOT = Path(__file__).resolve().parents[2]
BUILD_DIR = ROOT / "build"


def main():
    Log.info(f"Project root: {ROOT}")
    Log.info(f"Build directory: {BUILD_DIR}")

    BUILD_DIR.mkdir(parents=True, exist_ok=True)

    result = run_task(
        "Configure Build",
        [
            "cmake",
            "-S", str(ROOT),
            "-B", str(BUILD_DIR),
            "-GXcode",
            "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
            "-DCAVE_BUILD_ASSIMP=OFF",
            "-DCAVE_BUILD_UNIT_TESTS=ON",
        ],
    )

    if not result.success:
        finish(result)

    result = run_task(
        "Build Debug",
        [
            "cmake",
            "--build", str(BUILD_DIR),
            "--config", "Debug",
        ],
    )

    finish(result)


if __name__ == "__main__":
    main()