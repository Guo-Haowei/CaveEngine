#!/usr/bin/env python3
import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path

PLATFORM_APIS = {
    "windows": ["d3d11", "d3d12", "opengl"],
    "apple": ["metal"],
}

API_INFO = {
    "d3d11": ("hlsl", "__TARGET_D3D11__"),
    "d3d12": ("hlsl", "__TARGET_D3D11__"),
    "opengl": ("glsl", "__TARGET_OPENGL__"),
    "metal": ("metal", "__TARGET_METAL__"),
}


def run_command(cmd: list[str], label: str, input_file: Path, stage: str) -> bool:
    print(cmd)
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(
            f"[FAIL {label}] {input_file.name} ({stage}):\n{result.stderr}",
            file=sys.stderr,
        )
        return False
    return True


def compile_slang_to_spirv(
    slangc_bin: str,
    input_file: Path,
    entry_point: str,
    stage: str,
    spv_file: Path,
) -> bool:
    cmd = [
        slangc_bin,
        str(input_file),
        "-entry", entry_point,
        "-stage", stage,
        "-target", "spirv",
        "-no-mangle",
        "-D__TARGET_OPENGL__",
        "-o", str(spv_file),
    ]
    return run_command(cmd, "slangc SPIR-V", input_file, stage)


def run_slangc(
    slangc_bin: str,
    input_file: Path,
    entry_point: str,
    stage: str,
    target_lang: str,
    define: str,
    output_file: Path,
    spirv_cross_bin: str = "spirv-cross",
    
) -> bool:
    if target_lang == "glsl":
        spv_file = output_file.with_suffix(".spv")

        try:
            if not compile_slang_to_spirv(
                slangc_bin,
                input_file,
                entry_point,
                stage,
                spv_file,
            ):
                return False

            cmd = [
                spirv_cross_bin,
                str(spv_file),
                "--output", str(output_file),
                "--version", "460"
            ]

            if not run_command(cmd, "spirv-cross", input_file, stage):
                return False

        finally:
            spv_file.unlink(missing_ok=True)

    else:
        cmd = [
            slangc_bin,
            str(input_file),
            "-entry", entry_point,
            "-stage", stage,
            "-target", target_lang,
            "-no-mangle",
        ]

        cmd.append(f"-D{define}")
        cmd.extend(["-o", str(output_file)])

        if not run_command(cmd, "slangc", input_file, stage):
            return False

    print(f"[OK] {input_file.name} [{stage}] -> {output_file.name}")
    return True


def has_entry_point(content: str, entry_point: str | None) -> bool:
    """Check whether an entry-point function name appears as a whole word."""
    if not entry_point:
        return False
    return bool(re.search(rf"\b{re.escape(entry_point)}\b", content))


def compile_folder(
    source_folder: Path,
    api: str,
    entry_vs: str | None = "vs_main",
    entry_gs: str | None = None,
    entry_ps: str | None = "ps_main",
    entry_cs: str | None = "cs_main",
    slangc_path: str = "slangc",
    spirv_cross_path: str = "spirv-cross",
) -> None:
    lang, define = API_INFO[api]

    if not source_folder.exists() or not source_folder.is_dir():
        print(f"Error: Source folder '{source_folder}' does not exist.", file=sys.stderr)
        sys.exit(1)

    output_folder = Path(f"{api}_generated")
    if output_folder.exists():
        shutil.rmtree(output_folder)

    output_folder.mkdir(parents=True, exist_ok=True)

    slang_files = list(source_folder.glob("*.slang"))
    if not slang_files:
        print(f"No .slang files found in '{source_folder}'.")
        return

    print(f"Found {len(slang_files)} shader file(s) in '{source_folder}'.")
    print(f"Target Language: {lang.upper()}")
    print(f"Output Directory: '{output_folder.resolve()}'\n")

    total_compiled = 0
    total_failed = 0

    for slang_file in slang_files:
        try:
            content = slang_file.read_text(encoding="utf-8")
        except Exception as exc:
            print(f"Error reading {slang_file}: {exc}", file=sys.stderr)
            total_failed += 1
            continue

        stem = slang_file.stem
        candidate_stages = [
            ("vertex", entry_vs, f"{stem}.vs.{lang}"),
            ("geometry", entry_gs, f"{stem}.gs.{lang}"),
            ("fragment", entry_ps, f"{stem}.ps.{lang}"),
            ("compute", entry_cs, f"{stem}.cs.{lang}"),
        ]

        stages = [
            (stage, entry, output_name)
            for stage, entry, output_name in candidate_stages
            if entry and has_entry_point(content, entry)
        ]

        if not stages:
            print(f"Skipping '{slang_file.name}': No matching entry points found.")
            continue

        for stage, entry_point, output_name in stages:
            success = run_slangc(
                slangc_bin=slangc_path,
                input_file=slang_file,
                entry_point=entry_point,
                stage=stage,
                target_lang=lang,
                define=define,
                output_file=output_folder / output_name,
                spirv_cross_bin=spirv_cross_path,
            )

            if success:
                total_compiled += 1
            else:
                total_failed += 1

    print(
        f"\nFinished: {total_compiled} stage(s) compiled successfully, "
        f"{total_failed} failed."
    )
    if total_failed:
        sys.exit(1)


def main() -> None:
    if sys.platform == "darwin":
        platform = "apple"
    elif sys.platform.startswith("win") or sys.platform == "win32":
        platform = "windows"
    else:
        print(
            f"Error: Unsupported host platform '{sys.platform}'. "
            f"This script only supports Windows and macOS.",
            file=sys.stderr,
        )
        sys.exit(1)

    allowed_apis = PLATFORM_APIS.get(platform, [])
    if not allowed_apis:
        print(
            f"Error: No graphics APIs configured for platform '{platform}'.",
            file=sys.stderr,
        )
        sys.exit(1)

    all_api_choices = allowed_apis + ["all"]

    parser = argparse.ArgumentParser(
        description="Batch compile .slang shaders to language-specific output."
    )
    parser.add_argument(
        "-s", "--source-folder",
        type=Path,
        default=Path("./slang"),
        help="Folder containing .slang files (default: ./slang)",
    )
    parser.add_argument(
        "--api",
        choices=all_api_choices,
        default=all_api_choices[0],
        help=f"Target graphics API (default: '{all_api_choices[0]}')",
    )
    parser.add_argument(
        "--entry-vs",
        default="vs_main",
        help="Vertex shader entry point",
    )
    parser.add_argument(
        "--entry-gs",
        default=None,
        help="Optional geometry shader entry point",
    )
    parser.add_argument(
        "--entry-ps",
        default="ps_main",
        help="Fragment shader entry point",
    )
    parser.add_argument(
        "--entry-cs",
        default="cs_main",
        help="Compute shader entry point",
    )
    parser.add_argument(
        "--slangc-path",
        default="slangc",
        help="Path to slangc",
    )
    parser.add_argument(
        "--spirv-cross-path",
        default="spirv-cross",
        help="Path to spirv-cross",
    )

    args = parser.parse_args()

    if args.api != "all":
        allowed_apis = [args.api]

    for api in allowed_apis:
        compile_folder(
            source_folder=args.source_folder,
            api=api,
            entry_vs=args.entry_vs,
            entry_gs=args.entry_gs,
            entry_ps=args.entry_ps,
            entry_cs=args.entry_cs,
            slangc_path=args.slangc_path,
            spirv_cross_path=args.spirv_cross_path,
    )


if __name__ == "__main__":
    main()
