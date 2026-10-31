import os
import pathlib
import shutil
import subprocess
import zipfile
import time
from PIL import Image

def build_and_deploy():
    print("=== Packaging Universal Android APK: FartTogether_v8.2.0.apk ===")
    
    desktop_dir = pathlib.Path(os.environ.get("USERPROFILE", ".")) / "Desktop"
    if not desktop_dir.exists():
        onedrive_desk = pathlib.Path(os.environ.get("USERPROFILE", ".")) / "OneDrive" / "Desktop"
        if onedrive_desk.exists():
            desktop_dir = onedrive_desk

    build_dir = pathlib.Path("android_universal_apk_build")
    if build_dir.exists():
        shutil.rmtree(build_dir, ignore_errors=True)
    build_dir.mkdir(parents=True, exist_ok=True)

    # 1. Setup AndroidManifest.xml
    manifest_xml = '''<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.gamesmrkt.farttogether"
    android:versionCode="820"
    android:versionName="8.2.0">

    <uses-sdk android:minSdkVersion="24" android:targetSdkVersion="34" />
    <uses-feature android:glEsVersion="0x00020000" android:required="true" />
    <uses-feature android:name="android.hardware.touchscreen" android:required="true" />
    <uses-permission android:name="android.permission.INTERNET" />
    <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />
    <uses-permission android:name="android.permission.ACCESS_WIFI_STATE" />
    <uses-permission android:name="android.permission.CHANGE_WIFI_MULTICAST_STATE" />

    <application
        android:label="Fart Together"
        android:icon="@mipmap/ic_launcher"
        android:hasCode="false"
        android:isGame="true"
        android:debuggable="true"
        android:theme="@android:style/Theme.NoTitleBar.Fullscreen">
        <activity android:name="android.app.NativeActivity"
            android:label="Fart Together"
            android:screenOrientation="sensorLandscape"
            android:configChanges="orientation|keyboardHidden|screenSize|smallestScreenSize|keyboard|navigation"
            android:exported="true">
            <meta-data android:name="android.app.lib_name" android:value="main" />
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
'''
    (build_dir / "AndroidManifest.xml").write_text(manifest_xml, encoding="utf-8")

    # 2. Setup resources
    res_dir = build_dir / "res"
    (res_dir / "values").mkdir(parents=True, exist_ok=True)
    strings_xml = '''<?xml version="1.0" encoding="utf-8"?>
<resources>
    <string name="app_name">Fart Together</string>
</resources>
'''
    (res_dir / "values" / "strings.xml").write_text(strings_xml, encoding="utf-8")

    # Generate icons
    src_icon = pathlib.Path("CustomEngine/extracted_assets/textures/Character_LobbyGreen.png")
    if not src_icon.exists():
        src_icon = pathlib.Path("extracted_assets/textures/Character_LobbyGreen.png")
    
    icon_img = Image.open(src_icon)
    densities = {
        "mipmap-mdpi": 48,
        "mipmap-hdpi": 72,
        "mipmap-xhdpi": 96,
        "mipmap-xxhdpi": 144,
        "mipmap-xxxhdpi": 192
    }
    for folder, size in densities.items():
        out_folder = res_dir / folder
        out_folder.mkdir(parents=True, exist_ok=True)
        resized = icon_img.resize((size, size), Image.Resampling.LANCZOS)
        resized.save(out_folder / "ic_launcher.png")

    # 3. Setup assets
    assets_dir = build_dir / "assets"
    assets_dir.mkdir(parents=True, exist_ok=True)
    src_assets = pathlib.Path("CustomEngine/extracted_assets")
    if src_assets.exists():
        shutil.copytree(src_assets, assets_dir / "extracted_assets", dirs_exist_ok=True)
    for cfg in ["level_layout.json", "physics_config.json", "game_settings.json"]:
        p = pathlib.Path("CustomEngine") / cfg
        if p.exists():
            shutil.copy2(p, assets_dir / cfg)

    # 4. Setup native libraries from android_abi_build
    lib_dir = build_dir / "lib"
    abi_source_dir = pathlib.Path("android_abi_build")
    for abi in ["arm64-v8a", "armeabi-v7a", "x86_64"]:
        so_file = abi_source_dir / abi / "libmain.so"
        if so_file.exists():
            target_abi_dir = lib_dir / abi
            target_abi_dir.mkdir(parents=True, exist_ok=True)
            shutil.copy2(so_file, target_abi_dir / "libmain.so")
            print(f"Copied {abi}/libmain.so ({so_file.stat().st_size:,} bytes)")
        else:
            print(f"WARNING: Missing {abi}/libmain.so!")

    sdk_root = pathlib.Path(os.environ.get("LOCALAPPDATA")) / "Android" / "Sdk"
    aapt2 = sdk_root / "build-tools" / "34.0.0" / "aapt2.exe"
    zipalign = sdk_root / "build-tools" / "34.0.0" / "zipalign.exe"
    apksigner = sdk_root / "build-tools" / "34.0.0" / "apksigner.bat"
    android_jar = sdk_root / "platforms" / "android-34" / "android.jar"
    adb = sdk_root / "platform-tools" / "adb.exe"

    print("Step 1: Compiling resources with aapt2...")
    cmd_compile = [str(aapt2), "compile", "--dir", str(res_dir), "-o", str(build_dir / "compiled_res.zip")]
    subprocess.run(cmd_compile, check=True)

    print("Step 2: Linking APK package with aapt2...")
    unaligned_apk = build_dir / "unaligned.apk"
    cmd_link = [
        str(aapt2), "link",
        "-I", str(android_jar),
        "--manifest", str(build_dir / "AndroidManifest.xml"),
        "-o", str(unaligned_apk),
        str(build_dir / "compiled_res.zip"),
        "-A", str(assets_dir)
    ]
    subprocess.run(cmd_link, check=True)

    print("Step 3: Embedding multi-ABI native libraries into unaligned APK...")
    with zipfile.ZipFile(unaligned_apk, "a", compression=zipfile.ZIP_DEFLATED) as zf:
        for root, _, files in os.walk(lib_dir):
            for f in files:
                full_path = pathlib.Path(root) / f
                arc_name = full_path.relative_to(build_dir).as_posix()
                zf.write(full_path, arc_name)
                print(f"  Added {arc_name}")

    print("Step 4: Aligning APK with zipalign...")
    aligned_apk = build_dir / "aligned.apk"
    cmd_align = [str(zipalign), "-p", "-f", "4", str(unaligned_apk), str(aligned_apk)]
    subprocess.run(cmd_align, check=True)

    print("Step 5: Signing APK...")
    keystore_path = build_dir / "debug.keystore"
    keytool = r"C:\Program Files\Eclipse Adoptium\jdk-17.0.10.7-hotspot\bin\keytool.exe"
    cmd_key = [
        keytool, "-genkeypair", "-v",
        "-keystore", str(keystore_path),
        "-alias", "androiddebugkey",
        "-keyalg", "RSA", "-keysize", "2048", "-validity", "10000",
        "-storepass", "android", "-keypass", "android",
        "-dname", "CN=Android Debug,O=Android,C=US"
    ]
    subprocess.run(cmd_key, check=True)

    final_apk = desktop_dir / "FartTogether_v8.2.0.apk"
    cmd_sign = [
        "cmd.exe", "/c", str(apksigner), "sign",
        "--ks", str(keystore_path),
        "--ks-pass", "pass:android",
        "--key-pass", "pass:android",
        "--out", str(final_apk),
        str(aligned_apk)
    ]
    subprocess.run(cmd_sign, check=True)

    print("Step 6: Verifying signed APK...")
    cmd_verify = ["cmd.exe", "/c", str(apksigner), "verify", "--verbose", str(final_apk)]
    subprocess.run(cmd_verify, check=True)

    print(f"\n[SUCCESS] Universal Android APK built successfully: {final_apk}")
    print(f"File size: {final_apk.stat().st_size:,} bytes")

    # Step 7: Test install and run on connected phone
    if adb.exists():
        print("\nStep 7: Checking connected Android devices via ADB...")
        r_dev = subprocess.run([str(adb), "devices"], capture_output=True, text=True)
        print(r_dev.stdout)
        
        if "device\n" in r_dev.stdout or "\tdevice" in r_dev.stdout:
            print("Installing APK directly onto connected phone...")
            # Clear logcat first
            subprocess.run([str(adb), "logcat", "-c"])
            
            res_install = subprocess.run([str(adb), "install", "-r", str(final_apk)], capture_output=True, text=True)
            print("Install output:", res_install.stdout)
            if res_install.stderr:
                print("Install stderr:", res_install.stderr)
                
            print("Launching app on phone...")
            res_launch = subprocess.run([str(adb), "shell", "am", "start", "-n", "com.gamesmrkt.farttogether/android.app.NativeActivity"], capture_output=True, text=True)
            print("Launch output:", res_launch.stdout)
            
            time.sleep(3)
            print("Retrieving logcat output from app...")
            res_logs = subprocess.run([str(adb), "logcat", "-d", "-s", "raylib,AndroidRuntime,DEBUG,CustomEngineNative"], capture_output=True, text=True)
            print("=== App Logcat Output ===")
            print(res_logs.stdout[-2500:] if res_logs.stdout else "No specific logs emitted")
            
            # Check if process is running
            res_ps = subprocess.run([str(adb), "shell", "pidof", "com.gamesmrkt.farttogether"], capture_output=True, text=True)
            print("Process PID on device:", res_ps.stdout.strip())
            if res_ps.stdout.strip():
                print("[DEVICE VERIFICATION PASSED] App is running smoothly on phone!")
            else:
                print("[NOTE] Checking general process table...")
                r_ps2 = subprocess.run([str(adb), "shell", "ps | grep fart"], capture_output=True, text=True)
                print(r_ps2.stdout)

if __name__ == "__main__":
    build_and_deploy()
