# scripts/ci/build.py

from scripts.ci.ci_framework import finish, run_python_task


def main():
    #   - run: echo "Building project..."
    #   - run: python3 ./scripts/generate_meta.py
    #   # - run: ./scripts/ci_build_job.bat Debug
    #   # - run: echo "Testing project..."
    #   # - run: ./build/bin/Debug/unit_tests.exe
    result = run_python_task(
        "Buid project",
        "scripts/generate_meta.py",
    )

    finish(result)


if __name__ == "__main__":
    main()