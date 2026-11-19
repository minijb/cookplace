"""Configure, build, and test every independent Part 02 example.

Run after configuring a C++ compiler environment and VCPKG_ROOT.
Example: python tutorials/opengl-compute-shader/code/test-part02.py
Options: --clean removes each Debug build first; --ninja PATH sets CMAKE_MAKE_PROGRAM.
"""
from pathlib import Path
import shutil
import subprocess
import sys

CODE_DIR = Path(__file__).resolve().parent
PROJECTS = sorted(
    path for path in CODE_DIR.glob("part-02-[0-9][0-9]-*")
    if path.is_dir() and (path / "CMakeLists.txt").is_file()
)

if not PROJECTS:
    raise SystemExit("No Part 02 projects found")

configure_command = ["cmake", "--preset", "debug"]
if "--ninja" in sys.argv:
    ninja_index = sys.argv.index("--ninja")
    if ninja_index + 1 >= len(sys.argv):
        raise SystemExit("--ninja requires a path")
    configure_command.append(f"-DCMAKE_MAKE_PROGRAM={sys.argv[ninja_index + 1]}")

for project in PROJECTS:
    print(f"\n===== {project.name} =====", flush=True)
    if "--clean" in sys.argv[1:]:
        shutil.rmtree(project / "build" / "debug", ignore_errors=True)
    for command in (
        configure_command,
        ["cmake", "--build", "--preset", "debug"],
        ["ctest", "--preset", "debug"],
    ):
        subprocess.run(command, cwd=project, check=True)

print(f"\nAll {len(PROJECTS)} Part 02 projects passed.", flush=True)
