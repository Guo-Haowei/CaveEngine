# scripts/ci/compile_shader.py

from scripts.ci.ci_framework import finish, run_python_task


def main():
    result = run_python_task(
        "Shader Compilation",
        "scripts/compile_shaders.py",
        "--platform",
        "apple",
        "--source-folder",
        "cave/shader/slang",
        "--lang",
        "glsl",
        "--varying-locations",
        "0,1,2,3"
    )

    finish(result)


if __name__ == "__main__":
    main()