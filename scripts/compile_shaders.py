#!/usr/bin/env python3
import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path


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


def build_interface_renames(
    stage: str,
    varying_locations: list[int],
) -> dict[str, dict[int, str]]:
    """Use consistent names on adjacent shader-stage interfaces."""
    directions_by_stage = {
        "vertex": ("out",),
        "geometry": ("in", "out"),
        "fragment": ("in",),
    }

    renames: dict[str, dict[int, str]] = {}
    for direction in directions_by_stage.get(stage, ()):
        renames[direction] = {
            location: f"varying_{location}"
            for location in varying_locations
        }
    return renames


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
        "-D__TARGET_GLSL__",
        "-o", str(spv_file),
    ]
    return run_command(cmd, "slangc SPIR-V", input_file, stage)


def compile_spirv_to_glsl(
    spirv_cross_bin: str,
    spv_file: Path,
    output_file: Path,
    glsl_version: str | None,
    interface_renames: dict[str, dict[int, str]],
    input_file: Path,
    stage: str,
) -> bool:
    cmd = [
        spirv_cross_bin,
        str(spv_file),
        "--output", str(output_file),
        "--no-420pack-extension",
    ]

    if glsl_version:
        cmd.extend(["--version", glsl_version])

    for direction, locations in interface_renames.items():
        for location, name in sorted(locations.items()):
            cmd.extend([
                "--rename-interface-variable",
                direction,
                str(location),
                name,
            ])

    return run_command(cmd, "spirv-cross", input_file, stage)


def run_slangc(
    slangc_bin: str,
    input_file: Path,
    entry_point: str,
    stage: str,
    target_lang: str,
    glsl_version: str | None,
    output_file: Path,
    spirv_cross_bin: str = "spirv-cross",
    varying_locations: list[int] | None = None,
) -> bool:
    if target_lang == "glsl":
        spv_file = output_file.with_suffix(".spv")
        interface_renames = build_interface_renames(
            stage,
            varying_locations or [],
        )

        try:
            if not compile_slang_to_spirv(
                slangc_bin,
                input_file,
                entry_point,
                stage,
                spv_file,
            ):
                return False

            if not compile_spirv_to_glsl(
                spirv_cross_bin,
                spv_file,
                output_file,
                glsl_version,
                interface_renames,
                input_file,
                stage,
            ):
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

        if target_lang == "hlsl":
            cmd.append("-D__TARGET_HLSL__")
        elif target_lang == "metal":
            cmd.append("-D__TARGET_METAL__")

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


def parse_varying_locations(value: str) -> list[int]:
    """Parse a comma-separated list such as '0,1,3'."""
    try:
        locations = [int(item.strip()) for item in value.split(",") if item.strip()]
    except ValueError as exc:
        raise argparse.ArgumentTypeError(
            "Varying locations must be comma-separated integers, e.g. 0,1,3"
        ) from exc

    if len(locations) != len(set(locations)):
        raise argparse.ArgumentTypeError("Varying locations must not contain duplicates")

    return locations


def compile_folder(
    source_folder: Path,
    platform: str,
    lang: str,
    custom_output_folder: Path | None = None,
    entry_vs: str | None = "vs_main",
    entry_gs: str | None = None,
    entry_ps: str | None = "ps_main",
    entry_cs: str | None = "cs_main",
    slangc_path: str = "slangc",
    spirv_cross_path: str = "spirv-cross",
    varying_locations: list[int] | None = None,
) -> None:
    if not source_folder.exists() or not source_folder.is_dir():
        print(f"Error: Source folder '{source_folder}' does not exist.", file=sys.stderr)
        sys.exit(1)

    output_folder = custom_output_folder or Path(f"{lang}_generated")
    if output_folder.exists():
        shutil.rmtree(output_folder)

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
                glsl_version=glsl_version,
                output_file=output_folder / output_name,
                spirv_cross_bin=spirv_cross_path,
                varying_locations=varying_locations,
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
        "-o", "--output-folder",
        type=Path,
        default=None,
        help="Output folder (default: '<lang>_generated')",
    )
    default_platform = "apple" if sys.platform == "darwin" else "windows"
    parser.add_argument(
        "--platform",
        choices=["windows", "apple"],
        default=default_platform,
        help=f"Target platform (default: {default_platform})",
    )
    parser.add_argument(
        "--lang",
        choices=["hlsl", "glsl", "metal"],
        help="Target shading language",
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
        "--varying-locations",
        type=parse_varying_locations,
        default=[],
        help="Comma-separated interface locations to rename, e.g. 0,1",
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

    compile_folder(
        source_folder=args.source_folder,
        platform=args.platform,
        lang=args.lang,
        custom_output_folder=args.output_folder,
        entry_vs=args.entry_vs,
        entry_gs=args.entry_gs,
        entry_ps=args.entry_ps,
        entry_cs=args.entry_cs,
        slangc_path=args.slangc_path,
        spirv_cross_path=args.spirv_cross_path,
        varying_locations=args.varying_locations,
    )


if __name__ == "__main__":
    main()
