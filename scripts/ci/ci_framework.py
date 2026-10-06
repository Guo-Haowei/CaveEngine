# scripts/ci/ci_framework.py

import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from scripts.common.log import Log


ROOT = Path(__file__).resolve().parents[2]


@dataclass
class TaskResult:
    name: str
    success: bool
    duration: float


def run_task(
    name: str,
    command: list[str],
    cwd: Path = ROOT,
) -> TaskResult:
    Log.info(f"Starting: {name}")
    Log.info(f"Command: {' '.join(command)}")

    start = time.perf_counter()

    try:
        result = subprocess.run(
            command,
            cwd=cwd,
            check=False,
        )
        success = result.returncode == 0
    except Exception as e:
        Log.fail(f"{name}: {e}")
        success = False

    duration = time.perf_counter() - start

    if success:
        Log.ok(f"{name} ({duration:.2f}s)")
    else:
        Log.fail(f"{name} ({duration:.2f}s)")

    return TaskResult(
        name=name,
        success=success,
        duration=duration,
    )


def run_python_task(
    name: str,
    script: str,
    *args: str,
) -> TaskResult:
    return run_task(
        name,
        [sys.executable, script, *args],
    )


def finish(result: TaskResult) -> None:
    sys.exit(0 if result.success else 1)