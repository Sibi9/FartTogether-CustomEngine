import os
import shutil
import subprocess
import pathlib

sdk = pathlib.Path(os.environ["LOCALAPPDATA"]) / "Android" / "Sdk"
ndk = sdk / "ndk" / "25.2.9519653"
llvm_bin = ndk / "toolchains" / "llvm" / "prebuilt" / "windows-x86_64" / "bin"
raylib_src = pathlib.Path("CustomEngine/vendor/raylib_src/raylib-5.0/src")
glue_dir = ndk / "sources" / "android" / "native_app_glue"

# Compat headers and code
compat_lan = """#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket(s) close(s)
#define ioctlsocket(s, cmd, argp) fcntl(s, F_SETFL, O_NONBLOCK)
#define BOOL int
#define TRUE 1
#define FALSE 0
#define strncpy_s(dst, src, count) do { strncpy((dst), (src), (count)); (dst)[(count)-1] = '\\0'; } while(0)
#endif
"""

with open("CustomEngine/native/LanNetwork.cpp", "r", encoding="utf-8") as f:
    lan_text = f.read()

old_lan_hdr = """#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include "LanNetwork.h"

#pragma comment(lib, "ws2_32.lib")"""

lan_mod = lan_text
lan_mod = lan_mod.replace("WSADATA wsa;\n        if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0) {\n            gWsaReady = true;\n        }",
"""#ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0) {
            gWsaReady = true;
        }
#else
        gWsaReady = true;
#endif""")
lan_mod = lan_mod.replace("int fromLen = sizeof(from);", "socklen_t fromLen = sizeof(from);")

compat_engine = """
#include "rlgl.h"
#if !defined(RAYLIB_VERSION_MAJOR) || (RAYLIB_VERSION_MAJOR < 6)
inline Ray GetScreenToWorldRay(Vector2 position, Camera camera) { return GetMouseRay(position, camera); }
inline void DrawRectangleRoundedLinesEx(Rectangle rec, float roundness, int segments, float thick, Color color) {
    DrawRectangleRoundedLines(rec, roundness, segments, thick, color);
}
inline Shader GetShaderDefault_Compat() {
    Shader s = { 0 };
    s.id = rlGetShaderIdDefault();
    s.locs = rlGetShaderLocsDefault();
    return s;
}
#endif
"""

with open("CustomEngine/native/GameEngine.cpp", "r", encoding="utf-8") as f:
    engine_text = f.read()
engine_mod = engine_text.replace('#include "raylib.h"', '#include "raylib.h"' + compat_engine)

# On Android, make sure ResolveAsset and ResolveAudioFile don't fail due to FileExists
old_resolve = """static inline std::string ResolveAsset(const std::string& relPath) {

    if (FileExists(relPath.c_str())) return relPath;

    std::string customPath = "CustomEngine/" + relPath;

    if (FileExists(customPath.c_str())) return customPath;

    return relPath;

}"""

new_resolve = """static inline std::string ResolveAsset(const std::string& relPath) {
#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
    return relPath;
#else
    if (FileExists(relPath.c_str())) return relPath;
    std::string customPath = "CustomEngine/" + relPath;
    if (FileExists(customPath.c_str())) return customPath;
    return relPath;
#endif
}"""
engine_mod = engine_mod.replace(old_resolve, new_resolve)

# Ensure mobile touch HUD is always on for Android
old_hud = "bool isMobileTouchHUD = (gSettings.deviceMode == 2) || (gSettings.deviceMode == 0 && (GetTouchPointCount() > 0));"
new_hud = """#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
        bool isMobileTouchHUD = true;
#else
        bool isMobileTouchHUD = (gSettings.deviceMode == 2) || (gSettings.deviceMode == 0 && (GetTouchPointCount() > 0));
#endif"""
engine_mod = engine_mod.replace(old_hud, new_hud)

abis = [
    ("arm64-v8a", "aarch64-linux-android24-clang.cmd", "aarch64-linux-android24-clang++.cmd"),
    ("armeabi-v7a", "armv7a-linux-androideabi24-clang.cmd", "armv7a-linux-androideabi24-clang++.cmd"),
    ("x86_64", "x86_64-linux-android24-clang.cmd", "x86_64-linux-android24-clang++.cmd")
]

c_sources = [
    ("rcore", raylib_src / "rcore.c"),
    ("rshapes", raylib_src / "rshapes.c"),
    ("rtextures", raylib_src / "rtextures.c"),
    ("rtext", raylib_src / "rtext.c"),
    ("utils", raylib_src / "utils.c"),
    ("rmodels", raylib_src / "rmodels.c"),
    ("raudio", raylib_src / "raudio.c"),
    ("glue", glue_dir / "android_native_app_glue.c"),
]

build_root = pathlib.Path("android_abi_build")
build_root.mkdir(exist_ok=True)

(build_root / "LanNetwork.cpp").write_text(lan_mod, encoding="utf-8")
(build_root / "GameEngine.cpp").write_text(engine_mod, encoding="utf-8")

cflags_common = [
    "-DPLATFORM_ANDROID",
    "-DGRAPHICS_API_OPENGL_ES2",
    "-D__ANDROID__",
    "-DMAL_NO_OSS",
    "-DSTBIR_NO_SIMD",
    "-fPIC",
    "-O3",
    "-ffast-math",
    "-funroll-loops",
    "-ftree-vectorize",
    "-ffunction-sections",
    "-fdata-sections",
    f"-I{raylib_src}",
    f"-I{glue_dir}",
    "-ICustomEngine/native",
]

ldflags_common = [
    "-shared",
    "-static-libstdc++",
    "-Wl,-soname,libmain.so",
    "-Wl,--build-id",
    "-Wl,-z,noexecstack",
    "-Wl,-z,relro",
    "-Wl,-z,now",
    "-u", "ANativeActivity_onCreate",
    "-llog", "-landroid", "-lEGL", "-lGLESv2", "-lOpenSLES", "-lc", "-lm", "-latomic"
]

for abi_name, cc_name, cxx_name in abis:
    print(f"\n==========================================")
    print(f"Building ABI: {abi_name}")
    print(f"==========================================")
    abi_dir = build_root / abi_name
    abi_dir.mkdir(exist_ok=True)
    
    clang_c = llvm_bin / cc_name
    clang_cpp = llvm_bin / cxx_name
    
    objs = []
    # 1. Compile C sources
    for s_name, s_path in c_sources:
        obj_p = abi_dir / f"{s_name}.o"
        objs.append(str(obj_p))
        if not obj_p.exists():
            print(f"[{abi_name}] Compiling {s_name}.c...")
            cmd = [str(clang_c)] + cflags_common + ["-c", str(s_path), "-o", str(obj_p)]
            res = subprocess.run(cmd, capture_output=True, text=True)
            if res.returncode != 0:
                print(f"Error compiling {s_name}:", res.stderr)
                exit(1)
        else:
            print(f"[{abi_name}] {s_name}.o already exists.")

    # 2. Compile LanNetwork
    lan_obj = abi_dir / "LanNetwork.o"
    if lan_obj.exists(): lan_obj.unlink()
    objs.append(str(lan_obj))
    print(f"[{abi_name}] Compiling LanNetwork.cpp...")
    cmd = [str(clang_cpp), "-std=c++17"] + cflags_common + ["-c", str(build_root / "LanNetwork.cpp"), "-o", str(lan_obj)]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Error compiling LanNetwork:", res.stderr)
        exit(1)
            
    # 3. Compile GameEngine (remove previous failed obj)
    engine_obj = abi_dir / "GameEngine.o"
    if engine_obj.exists(): engine_obj.unlink()
    objs.append(str(engine_obj))
    print(f"[{abi_name}] Compiling GameEngine.cpp...")
    cmd = [str(clang_cpp), "-std=c++17"] + cflags_common + ["-c", str(build_root / "GameEngine.cpp"), "-o", str(engine_obj)]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Error compiling GameEngine:", res.stderr)
        exit(1)

    # 4. Link libmain.so
    out_so = abi_dir / "libmain.so"
    print(f"[{abi_name}] Linking libmain.so...")
    cmd_link = [str(clang_cpp)] + objs + ldflags_common + ["-o", str(out_so)]
    res = subprocess.run(cmd_link, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Error linking {abi_name} libmain.so:", res.stderr)
        exit(1)
    print(f"[{abi_name}] SUCCESS! libmain.so size: {out_so.stat().st_size:,} bytes")

    # Also copy libc++_shared.so as companion library
    libcxx_map = {
        "arm64-v8a": "aarch64-linux-android",
        "armeabi-v7a": "arm-linux-androideabi",
        "x86_64": "x86_64-linux-android"
    }
    libcxx_src = llvm_bin.parent / "sysroot" / "usr" / "lib" / libcxx_map[abi_name] / "libc++_shared.so"
    if libcxx_src.exists():
        shutil.copy2(libcxx_src, abi_dir / "libc++_shared.so")
        print(f"[{abi_name}] Copied libc++_shared.so ({libcxx_src.stat().st_size:,} bytes)")

print("\nALL ABIs BUILT SUCCESSFULLY!")
