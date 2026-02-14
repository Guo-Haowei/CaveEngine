import os
import re
import shutil
import sys

def get_engine_src_folder():
    source_folder = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.dirname(source_folder)
    return os.path.join(project_dir, 'cave', 'engine')

print('Project root folder:', get_engine_src_folder())

# throw 'Source folder not found'

# ========= CONFIG ==========
FILES = [
    # components
    'public/cave/runtime/ecs/components/CameraComponent.h',
    'public/cave/runtime/ecs/components/HierarchyComponent.h',
    'public/cave/runtime/ecs/components/NameComponent.h',
    'public/cave/runtime/ecs/components/LuaScriptComponent.h',
    'public/cave/runtime/ecs/components/TransformComponent.h',
    # assets
    'public/cave/runtime/assets/AssetMetaData.h',
    'private/runtime/assets/MaterialAsset.h',
    'private/runtime/assets/SpriteAnimationAsset.h',
    'private/runtime/assets/TileMapAsset.h',
    'private/runtime/assets/TileSetAsset.h',
    # components
    'private/runtime/scene/ColliderComponent.h',
    'private/runtime/scene/LightComponent.h',
    'private/runtime/scene/MaterialComponent.h',
    'private/runtime/scene/SceneComponent.h',
    'private/runtime/scene/SkeletalAnimationComponent.h',
    'private/runtime/scene/SpriteAnimatorComponent.h',
    # renderers
    'private/runtime/scene/MeshRendererComponent.h',
    'private/runtime/scene/SpriteRendererComponent.h',
    'private/runtime/scene/TileMapRendererComponent.h',
]

OUTPUT_DIR = os.path.join(get_engine_src_folder(), 'private/core/reflection/generated')
SCRIPT_NAME = os.path.basename(__file__)

META_CPP_SUFFIX = '.generated.cpp'
META_CPP_ALL = 'MetaAll.cpp'

# ========= REGEX ==========

meta_regex = re.compile(r"CAVE_META\s*\(\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*\)")
prop_regex = re.compile(r"CAVE_PROP\s*\((.*?)\)")
field_regex = re.compile(r"([a-zA-Z_][\w:]*)\s+([a-zA-Z_]\w*)\s*;")

# ========= PARSING & GENERATION ==========
def extract_field_name_and_type(line: str) -> str:
    line = line.strip()
    if line.endswith(';'):
        line = line[:-1].strip()

    parts = line.split()

    if '=' in parts:
        index = parts.index('=')
        parts = parts[:index]

    token = parts[-1]
    type_name = ' '.join(parts[:-1])
    return token, type_name

def remove_prefix(name: str) -> str:
    return name[2:] if name.startswith('m_') else name

def parse_extra(meta_data: str):
    extra = {}
    extra['serialize'] = True  # default to true
    for meta in meta_data.split(','):
        meta = meta.strip()
        if meta.startswith('editor = '):
            extra['editor'] = meta[len('editor = '):].strip()
        if meta.startswith('min = '):
            extra['min'] = meta[len('min = '):].strip()
        if meta.startswith('max = '):
            extra['max'] = meta[len('max = '):].strip()
        if meta.startswith('serialize = false'):
            extra['serialize'] = False

    return extra

def parse_file(file_path):
    metas = {}
    last_class_fields = None

    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    i = 0
    while i < len(lines):
        line = lines[i].strip()

        if meta_match := meta_regex.match(line):
            class_name = meta_match.group(1)
            if class_name in metas:
                raise RuntimeError(f'Duplicate CAVE_META found for class: {class_name}')
            last_class_fields = metas[class_name] = []

        if prop_match := prop_regex.match(line):
            assert class_name is not None, 'class_name must not be None'

            metadata = prop_match.group(1).strip()
            extra = parse_extra(metadata)

            i += 1
            while i < len(lines) and lines[i].strip() == '':
                i += 1
            if i < len(lines):
                next_line = lines[i].strip()
                token, type_name = extract_field_name_and_type(next_line)
                if token:
                    last_class_fields.append({
                        'type': type_name,
                        'name': token,
                        'meta': metadata,
                        'extra': extra,
                    })

        i += 1

    return metas

def generate_meta_for_class(f, class_name, fields):
    for field in fields:
        f.write(f"// {field['type']} {field['name']} ({field['meta']})\n")

    f.write("\ntemplate<>\n")
    f.write(f"const MetaTableFields& MetaDataTable<{class_name}>::GetFields() {{\n")
    f.write("    static MetaTableFields s_table = {\n")
    for field in fields:
        editor_hint = field['extra'].get('editor', 'None')
        num_min = field['extra'].get('min', None)
        num_max = field['extra'].get('max', None)
        field_name = field['name']
        display_name = remove_prefix(field_name)
        flags = 'FieldFlag::Serialize'
        if field['extra'].get('serialize', True) is False:
            flags = 'FieldFlag::None'
        register_field = f'REGISTER_FIELD({class_name}, "{display_name}", {field_name}, {flags}, EditorHint::{editor_hint}'
        if num_min is not None:
            register_field += f', {num_min}'
        if num_max is not None:
            register_field += f', {num_max}'
        f.write(f'        {register_field}),\n')
        continue
    f.write("    };\n\n")
    f.write("    return s_table;\n")
    f.write("}\n\n")
    f.write(f"// Avoid lazy init\n")
    f.write(f"[[maybe_unused]] static const auto& s_{class_name}_meta = MetaDataTable<{class_name}>::GetFields();\n\n")
    return

def generate_meta_file(base_path, file_path, metas):
    filename = os.path.basename(file_path)
    base = os.path.splitext(filename)[0]
    output_file = os.path.join(OUTPUT_DIR, base + META_CPP_SUFFIX)

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    with open(output_file, "w", encoding="utf-8") as f:
        f.write(f"// DO NOT EDIT THIS FILE\n")
        f.write(f"// Auto-generated metadata for {filename}\n")
        f.write(f"// Check {SCRIPT_NAME} for more details\n\n")

        short_path = f'engine/{base_path}'
        if (short_path.startswith('engine/public/')):
            short_path = short_path[len('engine/public/'):]

        f.write(f'#include "{short_path}"\n')
        f.write('#include "engine/private/core/reflection/MetaEditor.h"\n')
        f.write('#include "engine/private/serialization/yaml_include.h"\n')
        f.write('\nnamespace cave {\n\n')

        for class_name, fields in metas.items():
            generate_meta_for_class(f, class_name, fields)

        f.write('}  // namespace cave\n')

    print(f'Generated: {output_file}')
    return os.path.basename(output_file)

def main():
    if os.path.exists(OUTPUT_DIR):
        shutil.rmtree(OUTPUT_DIR)

    os.makedirs(OUTPUT_DIR)

    generated_files = []

    for base_path in FILES:
        file_path = os.path.join(get_engine_src_folder(), base_path)
        if not os.path.isfile(file_path):
            print(f'File not found: {file_path}')
            raise FileNotFoundError(f'File not found: {file_path}')

        metas = parse_file(file_path)

        if len(metas) > 0:
            generated_file_name = generate_meta_file(base_path, file_path, metas)
            generated_files.append(generated_file_name)
        else:
            print(f'No CAVE_PROP found in: {file_path}')

    output_file = os.path.join(OUTPUT_DIR, '..', META_CPP_ALL)
    print(output_file)

    with open(output_file, 'w') as f:
        f.write('// DO NOT EDIT THIS FILE\n')
        f.write(f'// run {SCRIPT_NAME} to generate the meta files\n\n')
        for cpp_file in generated_files:
            filename = os.path.basename(cpp_file)
            f.write(f'#include "generated/{filename}"\n')

if __name__ == '__main__':
    main()