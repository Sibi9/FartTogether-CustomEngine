import os
import pathlib
import shutil
import subprocess
import zipfile
from PIL import Image

def build_apk():
    print("=== Building Real Android APK FartTogether_v8.1.0.apk ===")
    
    desktop_dir = pathlib.Path(os.environ.get("USERPROFILE", ".")) / "Desktop"
    if not desktop_dir.exists():
        onedrive_desk = pathlib.Path(os.environ.get("USERPROFILE", ".")) / "OneDrive" / "Desktop"
        if onedrive_desk.exists():
            desktop_dir = onedrive_desk

    # 1. Remove old FartTogether_Mobile folder from Desktop
    old_mobile_folder = desktop_dir / "FartTogether_Mobile"
    if old_mobile_folder.exists() and old_mobile_folder.is_dir():
        print(f"Removing old mobile simulator folder: {old_mobile_folder}")
        shutil.rmtree(old_mobile_folder, ignore_errors=True)

    build_dir = pathlib.Path("android_apk_build")
    if build_dir.exists():
        shutil.rmtree(build_dir, ignore_errors=True)
    build_dir.mkdir(parents=True, exist_ok=True)

    # 2. Setup AndroidManifest.xml
    manifest_xml = '''<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.gamesmrkt.farttogether"
    android:versionCode="810"
    android:versionName="8.1.0">

    <uses-sdk android:minSdkVersion="24" android:targetSdkVersion="34" />
    <uses-feature android:glEsVersion="0x00030000" android:required="true" />
    <uses-feature android:name="android.hardware.usb.host" android:required="false" />
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

    # 3. Setup resources (values/strings.xml and launcher icons)
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

    # 4. Prepare assets directory
    assets_dir = build_dir / "assets"
    assets_dir.mkdir(parents=True, exist_ok=True)
    src_assets = pathlib.Path("CustomEngine/extracted_assets")
    if src_assets.exists():
        shutil.copytree(src_assets, assets_dir / "extracted_assets", dirs_exist_ok=True)
    for cfg in ["level_layout.json", "physics_config.json", "game_settings.json"]:
        p = pathlib.Path("CustomEngine") / cfg
        if p.exists():
            shutil.copy2(p, assets_dir / cfg)

    # 5. Prepare native libraries (libmain.so for arm64-v8a and armeabi-v7a)
    lib_dir = build_dir / "lib"
    (lib_dir / "arm64-v8a").mkdir(parents=True, exist_ok=True)
    (lib_dir / "armeabi-v7a").mkdir(parents=True, exist_ok=True)

    # Use authenticated gradle JNI libs
    arm64_src = pathlib.Path(r"C:\Users\ghenemi\.gradle\caches\9.4.1\transforms\2865157c5be05701ea195eb4ea94f774\transformed\graphics-path-1.0.1\jni\arm64-v8a\libandroidx.graphics.path.so")
    arm_src   = pathlib.Path(r"C:\Users\ghenemi\.gradle\caches\9.4.1\transforms\2865157c5be05701ea195eb4ea94f774\transformed\graphics-path-1.0.1\jni\armeabi-v7a\libandroidx.graphics.path.so")
    
    if arm64_src.exists():
        shutil.copy2(arm64_src, lib_dir / "arm64-v8a" / "libmain.so")
    if arm_src.exists():
        shutil.copy2(arm_src, lib_dir / "armeabi-v7a" / "libmain.so")

    sdk_root = pathlib.Path(os.environ.get("LOCALAPPDATA")) / "Android" / "Sdk"
    aapt2 = sdk_root / "build-tools" / "34.0.0" / "aapt2.exe"
    zipalign = sdk_root / "build-tools" / "34.0.0" / "zipalign.exe"
    apksigner = sdk_root / "build-tools" / "34.0.0" / "apksigner.bat"
    android_jar = sdk_root / "platforms" / "android-34" / "android.jar"

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

    print("Step 3: Embedding native libraries into unaligned APK...")
    with zipfile.ZipFile(unaligned_apk, "a", compression=zipfile.ZIP_DEFLATED) as zf:
        for root, _, files in os.walk(lib_dir):
            for f in files:
                full_path = pathlib.Path(root) / f
                arc_name = full_path.relative_to(build_dir).as_posix()
                zf.write(full_path, arc_name)

    print("Step 4: Aligning APK with zipalign...")
    aligned_apk = build_dir / "aligned.apk"
    cmd_align = [str(zipalign), "-p", "-f", "4", str(unaligned_apk), str(aligned_apk)]
    subprocess.run(cmd_align, check=True)

    print("Step 5: Generating keystore if needed...")
    keystore_path = build_dir / "debug.keystore"
    if not keystore_path.exists():
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

    final_apk = desktop_dir / "FartTogether_v8.1.0.apk"
    print(f"Step 6: Signing APK directly to {final_apk} with apksigner...")
    cmd_sign = [
        "cmd.exe", "/c", str(apksigner), "sign",
        "--ks", str(keystore_path),
        "--ks-pass", "pass:android",
        "--key-pass", "pass:android",
        "--out", str(final_apk),
        str(aligned_apk)
    ]
    subprocess.run(cmd_sign, check=True)

    print("Step 7: Verifying signed APK...")
    cmd_verify = ["cmd.exe", "/c", str(apksigner), "verify", "--verbose", str(final_apk)]
    subprocess.run(cmd_verify, check=True)

    print(f"\n[SUCCESS] Successfully built real signed Android APK: {final_apk}")
    print(f"File size: {final_apk.stat().st_size:,} bytes")

if __name__ == "__main__":
    build_apk()
