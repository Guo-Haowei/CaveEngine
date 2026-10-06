# scripts/ci/format.py

from scripts.ci.ci_framework import finish, run_python_task


def main():
    result = run_python_task(
        "Format Code",
        "scripts/format_code.py",
    )

    if not result.success:
        finish(result)

    result = run_python_task(
        "Check Changes",
        "scripts/get_git_changed_files.py",
    )

    finish(result)


if __name__ == "__main__":
    main()