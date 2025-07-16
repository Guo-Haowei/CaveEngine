import os
import re
import sys

def get_engine_src_folder():
    source_folder = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.dirname(source_folder)
    return os.path.join(project_dir, 'engine', 'src', 'engine')

print("Project root folder:", get_engine_src_folder())

# throw "Source folder not found"

# ========= CONFIG ==========
FILES = [
    "tile_map/tile_map_asset.h",
    "tile_map/tile_map_renderer.h",
    "scene/transform_component.h",
]

OUTPUT_DIR = os.path.join(get_engine_src_folder(), "reflection/generated")

META_CPP_SUFFIX = ".meta.cpp"

# ========= REGEX ==========

meta_regex = re.compile(r"CAVE_META\s*\(\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*\)")
prop_regex = re.compile(r"CAVE_PROP\s*\((.*?)\)")
field_regex = re.compile(r"([a-zA-Z_][\w:]*)\s+([a-zA-Z_]\w*)\s*;")

# ========= PARSING & GENERATION ==========
def extract_field_name(line: str) -> str:
    line = line.strip()
    if line.endswith(';'):
        line = line[:-1].strip()

    parts = line.split()

    if not parts:
        return ""

    # The last part may contain '= something' or just the name
    last_part = parts[-1]

    # Check if '=' exists in last_part (like: name=val)
    if '=' in last_part:
        name = last_part.split('=')[0].strip()
    else:
        name = last_part.strip()

    type_tokens = parts[:-1]
    # But if last_part contained '=', it might split name and type incorrectly
    # So safer to re-join all except last token, then remove possible trailing = in name
    type_name = " ".join(type_tokens)

    return type_name, name

def filed_meta():
    pass

def parse_file(file_path):
    results = []

    class_name = None

    with open(file_path, "r", encoding="utf-8") as f:
        lines = f.readlines()

    i = 0
    while i < len(lines):
        line = lines[i].strip()

        if meta_match := meta_regex.match(line):
            assert class_name is None, "class_name must be None"
            class_name = meta_match.group(1)

        if prop_match := prop_regex.match(line):
            assert class_name is not None, "class_name must not be None"

            metadata = prop_match.group(1).strip()
            i += 1
            while i < len(lines) and lines[i].strip() == "":
                i += 1
            if i < len(lines):
                next_line = lines[i].strip()
                type_name, field_name = extract_field_name(next_line)
                if field_name:
                    results.append({
                        "type": type_name,
                        "name": field_name,
                        "meta": metadata
                    })

        i += 1

    return class_name, results


def generate_meta_file(base_path, file_path, class_name, fields):
    filename = os.path.basename(file_path)
    base = os.path.splitext(filename)[0]
    output_file = os.path.join(OUTPUT_DIR, base + META_CPP_SUFFIX)

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    with open(output_file, "w", encoding="utf-8") as f:
        f.write(f"// Auto-generated metadata for {filename}\n\n")
        for field in fields:
            f.write(f"// {field['type']} {field['name']} ({field['meta']})\n")

        f.write('\n#include "engine/systems/serialization/serialization.h"\n')
        f.write(f'#include "engine/{base_path}"\n\n')
        f.write("namespace cave {\n\n")
        # f.write(f"class {class_name};\n\n")
        f.write("template<>\n")
        f.write(f"const MetaTableFields& GetMetaTableFields<{class_name}>() {{\n")
        f.write("    static MetaTableFields s_table = {\n")
        for field in fields:
            field_name = field['name']
            f.write(f'        DEFINE_FILED({class_name}, "{field_name}", {field_name}),\n')
            continue
        f.write("    };\n\n")
        f.write("    return s_table;\n")
        f.write("}\n\n")
        f.write("}  // namespace cave\n")

    print(f"Generated: {output_file}")


def main():
    for base_path in FILES:
        file_path = os.path.join(get_engine_src_folder(), base_path)
        if not os.path.isfile(file_path):
            print(f"File not found: {file_path}")
            continue

        class_name, fields = parse_file(file_path)
        assert class_name is not None, "class must not be None"

        if fields:
            generate_meta_file(base_path, file_path, class_name, fields)
        else:
            print(f"No CAVE_PROP found in: {file_path}")


if __name__ == "__main__":
    main()