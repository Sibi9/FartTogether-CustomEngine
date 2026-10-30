import os
import zipfile
import shutil

desktop_dir = os.path.expanduser(r'~\Desktop')
zip_output_path = os.path.join(desktop_dir, 'FartTogether_CustomEngine.zip')

print(f"Target ZIP path: {zip_output_path}")

# Files and directories to include from CustomEngine:
# We want a root folder inside the zip named 'FartTogether_CustomEngine'
# Inside it:
# - FartTogetherNative.exe
# - launch.bat
# - build_tool.bat
# - level_layout.json
# - PROJECT_DOCUMENTATION_AND_ANALYSIS.md
# - native/GameEngine.cpp
# - vendor/ (all headers and lib)
# - extracted_assets/ (all meshes, textures, audio, ui, fonts)

include_files = [
    ('CustomEngine/FartTogetherNative.exe', 'FartTogetherNative.exe'),
    ('CustomEngine/launch.bat', 'launch.bat'),
    ('CustomEngine/build_tool.bat', 'build_tool.bat'),
    ('CustomEngine/level_layout.json', 'level_layout.json'),
    ('CustomEngine/PROJECT_DOCUMENTATION_AND_ANALYSIS.md', 'PROJECT_DOCUMENTATION_AND_ANALYSIS.md'),
    ('CustomEngine/COMPARISON_AND_GAPS_REPORT.md', 'COMPARISON_AND_GAPS_REPORT.md'),
    ('VISUAL_AND_PHYSICS_REPORT.md', 'VISUAL_AND_PHYSICS_REPORT.md'),
    ('CustomEngine/native/GameEngine.cpp', 'native/GameEngine.cpp'),
    ('CustomEngine/native/spline_code.inl', 'native/spline_code.inl'),
]

include_dirs = [
    ('CustomEngine/vendor', 'vendor'),
    ('CustomEngine/extracted_assets', 'extracted_assets'),
    ('screenshots', 'screenshots'),
]

print("Creating ZIP archive...")
with zipfile.ZipFile(zip_output_path, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as zf:
    # 1. Add individual files
    for src, arc in include_files:
        if os.path.exists(src):
            arcname = os.path.join('FartTogether_CustomEngine', arc)
            print(f"Adding file: {src} -> {arcname}")
            zf.write(src, arcname)
        else:
            print(f"WARNING: File not found: {src}")

    # 2. Add directories
    for src_dir, arc_dir in include_dirs:
        for root, dirs, files in os.walk(src_dir):
            for file in files:
                # Exclude any python scripts or obj/pdb intermediates
                if file.endswith(('.py', '.pyc', '.pdb', '.ilk')):
                    continue
                fp = os.path.join(root, file)
                rel = os.path.relpath(fp, src_dir)
                arcname = os.path.join('FartTogether_CustomEngine', arc_dir, rel)
                zf.write(fp, arcname)

size_mb = os.path.getsize(zip_output_path) / (1024 * 1024)
print(f"\nSuccessfully created ZIP on Desktop!")
print(f"File: {zip_output_path}")
print(f"Size: {size_mb:.2f} MB")

