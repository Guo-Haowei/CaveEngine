#!/usr/bin/env python3
import argparse
import subprocess
import sys
from pathlib import Path


def run_slangc(
    slangc_bin: str,
    input_file: Path,
    entry_point: str,
    stage: str,
    target_lang: str,
    glsl_version: str | None,
    output_file: Path,
) -> bool:
    cmd = [
        slangc_bin,
        str(input_file),
        "-entry",
        entry_point,
        "-stage",
        stage,
        "-target",
        target_lang,
        "-no-mangle",
    ]

    if target_lang == "glsl" and glsl_version:
        cmd.extend(["-profile", f"glsl_{glsl_version}"])

    cmd.extend(["-o", str(output_file)])

    print(cmd)

    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"[FAIL] {input_file.name} ({stage}):\n{result.stderr}", file=sys.stderr)
        return False

    print(f"[OK] {input_file.name} [{stage}] -> {output_file.name}")
    return True


def compile_folder(
    source_folder: Path,
    platform: str,
    lang: str,
    custom_output_folder: Path | None = None,
    entry_vs: str = "vs_main",
    entry_ps: str = "ps_main",
    slangc_path: str = "slangc",
):
    if not source_folder.exists() or not source_folder.is_dir():
        print(f"Error: Source folder '{source_folder}' does not exist.", file=sys.stderr)
        sys.exit(1)

    output_folder = custom_output_folder or Path(f"{lang}_generated")
    output_folder.mkdir(parents=True, exist_ok=True)

    glsl_version = "410" if platform == "apple" else "460"
    # glsl_version = "460"

    slang_files = list(source_folder.glob("*.slang"))
    if not slang_files:
        print(f"No .slang files found in '{source_folder}'.")
        return

    print(f"Found {len(slang_files)} shader file(s) in '{source_folder}'.")
    print(f"Target Language: {lang.upper()} (Platform: {platform})")
    print(f"Output Directory: '{output_folder.resolve()}'\n")

    total_compiled = 0
    total_failed = 0

    for slang_file in slang_files:
        stem = slang_file.stem
        stages = [
            ("vertex", entry_vs, f"{stem}.vs.{lang}"),
            ("fragment", entry_ps, f"{stem}.ps.{lang}"),
        ]

        for stage_name, entry_point, out_filename in stages:
            out_path = output_folder / out_filename
            success = run_slangc(
                slangc_bin=slangc_path,
                input_file=slang_file,
                entry_point=entry_point,
                stage=stage_name,
                target_lang=lang,
                glsl_version=glsl_version,
                output_file=out_path,
            )
            if success:
                total_compiled += 1
            else:
                total_failed += 1

    print(f"\nFinished: {total_compiled} stage(s) compiled successfully, {total_failed} failed.")
    if total_failed > 0:
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(
        description="Batch compile all .slang shaders in a directory to language-specific output folders."
    )
    parser.add_argument(
        "-s",
        "--source-folder",
        type=Path,
        default=Path("./slang"),
        help="Folder containing .slang source files (default: ./shaders)",
    )
    parser.add_argument(
        "-o",
        "--output-folder",
        type=Path,
        default=None,
        help="Optional explicit output folder (defaults to '<lang>_generated')",
    )
    parser.add_argument(
        "--platform",
        choices=["windows", "apple"],
        default="windows",
        help="Target platform (default: windows)",
    )
    parser.add_argument(
        "--lang",
        choices=["hlsl", "glsl"],
        default="hlsl",
        help="Target shading language (default: hlsl)",
    )
    parser.add_argument("--entry-vs", default="vs_main", help="Vertex shader entry point (default: vs_main)")
    parser.add_argument("--entry-ps", default="ps_main", help="Pixel/Fragment shader entry point (default: ps_main)")
    parser.add_argument("--slangc-path", default="slangc", help="Path to slangc binary (default: 'slangc')")

    args = parser.parse_args()

    compile_folder(
        source_folder=args.source_folder,
        platform=args.platform,
        lang=args.lang,
        custom_output_folder=args.output_folder,
        entry_vs=args.entry_vs,
        entry_ps=args.entry_ps,
        slangc_path=args.slangc_path,
    )


if __name__ == "__main__":
    main()