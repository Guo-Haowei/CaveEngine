# scripts/ci/compile_shader.py

from scripts.ci.ci_framework import finish, run_python_task


def main():
    result = run_python_task(
        "Shader Compilation",
        "scripts/compile_shaders.py",
        "--source-folder",
        "cave/shader/slang",
    )

    finish(result)


if __name__ == "__main__":
    main()