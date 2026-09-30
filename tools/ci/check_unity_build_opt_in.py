"""Exercise the real Unity CMake gates without downloading/building SimLibs."""
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    source = (ROOT / "CMakeLists.txt").read_text()
    option = re.search(r'^option\(PROJECTAIRSIM_BUILD_UNITY .*\)$', source, re.M).group()
    gates = re.findall(r'(?m)^([ \t]*)if\(PROJECTAIRSIM_BUILD_UNITY\)\n(.*?)^\1endif\(\)', source, re.S)
    assert len(gates) == 3, "Review coverage when adding/removing Unity gates"
    blocks = "\n".join(f"if(PROJECTAIRSIM_BUILD_UNITY)\n{body}endif()" for _, body in gates)
    with tempfile.TemporaryDirectory(prefix="pas-unity-gates-") as directory:
        root = Path(directory)
        for name in ("unity", "onnx", "zlib"):
            (root / name).mkdir()
        (root / "onnx/libonnxruntime.so").write_text("fixture")
        (root / "zlib/libz.so").write_text("fixture")
        (root / "unity/CMakeLists.txt").write_text(
            'add_custom_target(sim_unity_wrapper ALL COMMAND ${CMAKE_COMMAND} -E touch "${UNITY_WRAPPER_DLL_DIR}/wrapper-built")\n')
        (root / "CMakeLists.txt").write_text('''cmake_minimum_required(VERSION 3.15)
project(UnityGateContract NONE)
include(ExternalProject)
set(UNITY_WRAPPER_DLL_DIR "${CMAKE_SOURCE_DIR}/plugins")
set(ONNX_LIB_DIR "${CMAKE_SOURCE_DIR}/onnx")
set(ZLIB_INSTALL_LIB_DIR "${CMAKE_SOURCE_DIR}/zlib")
ExternalProject_Add(zlib-external SOURCE_DIR "${CMAKE_SOURCE_DIR}/zlib"
    DOWNLOAD_COMMAND "" CONFIGURE_COMMAND "" BUILD_COMMAND "" INSTALL_COMMAND "")
add_custom_target(shared_simlibs ALL COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_BINARY_DIR}/shared-built")
''' + option + "\n" + blocks)
        build = root / "build"
        plugins = root / "plugins"
        def run(*args):
            subprocess.run(["cmake", *args], check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        run("-S", str(root), "-B", str(build), "-G", "Ninja")
        run("--build", str(build))
        assert (build / "shared-built").exists()
        assert not plugins.exists(), "Default build touched Unity staging"
        run("-S", str(root), "-B", str(build), "-DPROJECTAIRSIM_BUILD_UNITY=ON")
        run("--build", str(build))
        assert {p.name for p in plugins.iterdir()} == {"libonnxruntime.so", "libz.so", "wrapper-built"}
        before = {p.name: p.stat().st_mtime_ns for p in plugins.iterdir()}
        run("-S", str(root), "-B", str(build), "-DPROJECTAIRSIM_BUILD_UNITY=OFF")
        run("--build", str(build))
        assert before == {p.name: p.stat().st_mtime_ns for p in plugins.iterdir()}, "Disabled build restaged Unity outputs"
        result = subprocess.run(["cmake", "--build", str(build), "--target", "sim_unity_wrapper"], capture_output=True)
        assert result.returncode != 0, "Unity target survived disabling the option"
    print("PASS: default OFF, explicit ON, reused-cache OFF; shared target preserved and Unity outputs untouched when disabled")


if __name__ == "__main__":
    main()
