import argparse
import os
import re
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
    spirv_cross_bin: str = "spirv-cross",
) -> bool:
    if target_lang == "glsl":
        # Step 1: Slang -> SPIR-V
        spv_file = output_file.with_suffix(".spv")
        slang_cmd = [
            slangc_bin,
            str(input_file),
            "-entry",
            entry_point,
            "-stage",
            stage,
            "-target",
            "spirv",
            "-no-mangle",
            "-D__TARGET_GLSL__",
            "-o",
            str(spv_file),
        ]

        print(slang_cmd)
        result = subprocess.run(slang_cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"[FAIL slangc SPIR-V] {input_file.name} ({stage}):\n{result.stderr}", file=sys.stderr)
            if spv_file.exists():
                spv_file.unlink()
            return False

        # Step 2: SPIR-V -> GLSL using spirv-cross
        spirv_cross_cmd = [
            spirv_cross_bin,
            str(spv_file),
            "--output",
            str(output_file),
            "--no-420pack-extension",
        ]
        if glsl_version:
            spirv_cross_cmd.extend(["--version", glsl_version])

        print(spirv_cross_cmd)
        spv_result = subprocess.run(spirv_cross_cmd, capture_output=True, text=True)

        # Cleanup temporary intermediate .spv file
        if spv_file.exists():
            spv_file.unlink()

        if spv_result.returncode != 0:
            print(f"[FAIL spirv-cross] {input_file.name} ({stage}):\n{spv_result.stderr}", file=sys.stderr)
            return False

    else:
        # Direct compilation (HLSL, etc.)
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

        if target_lang == "hlsl":
            cmd.extend(["-D__TARGET_HLSL__"])

        cmd.extend(["-o", str(output_file)])

        print(cmd)

        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"[FAIL] {input_file.name} ({stage}):\n{result.stderr}", file=sys.stderr)
            return False

    print(f"[OK] {input_file.name} [{stage}] -> {output_file.name}")
    return True


def has_entry_point(content: str, entry_point: str | None) -> bool:
    """Check if an entry point function name exists as a whole word in the shader source."""
    if not entry_point:
        return False
    return bool(re.search(rf"\b{re.escape(entry_point)}\b", content))


def compile_folder(
    source_folder: Path,
    platform: str,
    lang: str,
    custom_output_folder: Path | None = None,
    entry_vs: str | None = "vs_main",
    entry_ps: str | None = "ps_main",
    entry_cs: str | None = "cs_main",
    slangc_path: str = "slangc",
    spirv_cross_path: str = "spirv-cross",
):
    if not source_folder.exists() or not source_folder.is_dir():
        print(f"Error: Source folder '{source_folder}' does not exist.", file=sys.stderr)
        sys.exit(1)

    output_folder = custom_output_folder or Path(f"{lang}_generated")
    output_folder.mkdir(parents=True, exist_ok=True)

    glsl_version = "410" if platform == "apple" else "460"

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
        try:
            content = slang_file.read_text(encoding="utf-8")
        except Exception as e:
            print(f"Error reading {slang_file}: {e}", file=sys.stderr)
            total_failed += 1
            continue

        # Candidate stages: (stage_name, entry_point, output_filename)
        candidate_stages = [
            ("vertex", entry_vs, f"{stem}.vs.{lang}"),
            ("fragment", entry_ps, f"{stem}.ps.{lang}"),
            ("compute", entry_cs, f"{stem}.cs.{lang}"),
        ]

        # Filter stages that exist in the shader file
        stages = [
            (stage, entry, out_name)
            for stage, entry, out_name in candidate_stages
            if entry and has_entry_point(content, entry)
        ]

        if not stages:
            print(f"Skipping '{slang_file.name}': No matching entry points found.")
            continue

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
                spirv_cross_bin=spirv_cross_path,
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
        help="Folder containing .slang source files (default: ./slang)",
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
    parser.add_argument("--slangc-path", default="slangc", help="Path to slangc binary (default: 'slangc')")

    args = parser.parse_args()

    project_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.dirname(project_dir)
    spirv_cross_path = os.path.join(project_dir, 'bin/spirv-cross')

    compile_folder(
        source_folder=args.source_folder,
        platform=args.platform,
        lang=args.lang,
        custom_output_folder=args.output_folder,
        slangc_path=args.slangc_path,
        spirv_cross_path=spirv_cross_path,
    )


if __name__ == "__main__":
    main()