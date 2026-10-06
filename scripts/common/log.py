# scripts/common/log.py

class Log:
    RESET = "\033[0m"

    BOLD = "\033[1m"

    RED = "\033[31m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    BLUE = "\033[34m"
    CYAN = "\033[36m"

    @staticmethod
    def _print(tag: str, color: str, message: str) -> None:
        print(
            f"{Log.BOLD}{color}[{tag}]{Log.RESET} "
            f"{message}"
        )

    @staticmethod
    def ok(message: str) -> None:
        Log._print("OK", Log.GREEN, message)

    @staticmethod
    def fail(message: str) -> None:
        Log._print("FAIL", Log.RED, message)

    @staticmethod
    def info(message: str) -> None:
        Log._print("INFO", Log.BLUE, message)

    @staticmethod
    def warn(message: str) -> None:
        Log._print("WARN", Log.YELLOW, message)

    @staticmethod
    def debug(message: str) -> None:
        Log._print("DEBUG", Log.CYAN, message)