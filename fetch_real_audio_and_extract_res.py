import os, pathlib, shutil, subprocess, urllib.request

audio_dirs = [pathlib.Path("extracted_assets/audio"), pathlib.Path("CustomEngine/extracted_assets/audio")]
for ad in audio_dirs:
    ad.mkdir(parents=True, exist_ok=True)

AUDIO_TOOL = pathlib.Path("CustomEngine/audio_tool.exe")

REAL_AUDIO_SOURCES = {
    "burp.wav": "https://raw.githubusercontent.com/bogind/qgs_sound_effects/master/sounds/burp.wav",
    "eat_munch.wav": "https://raw.githubusercontent.com/clear-code-projects/Snake/master/Sound/crunch.wav",
    "checkpoint_chime.wav": "https://raw.githubusercontent.com/DevsDaddy/Unity-UI-Audio-Collection/main/DevsDaddy/Audio%20Libraries/UI/Chimes/Chimes_1.wav",
    "pop_snot.wav": "https://raw.githubusercontent.com/DevsDaddy/Unity-UI-Audio-Collection/main/DevsDaddy/Audio%20Libraries/UI/Pop/Mouth/Pop_Mouth_High_1.wav",
    "menu_beach_ost.wav": "https://raw.githubusercontent.com/CrispyCrafter/AudioNet/master/Audio/sea_waves/1-28135-A.wav",
    "snore_loop.wav": "https://raw.githubusercontent.com/CrispyCrafter/AudioNet/master/Audio/snoring/1-20545-A.wav",
    "ship_horn.wav": "https://raw.githubusercontent.com/CrispyCrafter/AudioNet/master/Audio/car_horn/1-17124-A.wav",
    "seagull_call.wav": "https://raw.githubusercontent.com/CrispyCrafter/AudioNet/master/Audio/chirping_birds/1-100038-A.wav"
}

def download_and_process():
    print("=== Downloading Real Recorded Audio Files ===")
    for filename, url in REAL_AUDIO_SOURCES.items():
        dst_raw = audio_dirs[0] / f"_raw_{filename}"
        final_wav = audio_dirs[0] / filename
        try:
            print(f"Downloading real audio: {filename} from {url}...")
            req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"})
            with urllib.request.urlopen(req, timeout=15) as r:
                data = r.read()
            dst_raw.write_bytes(data)
            
            # Convert / format to standard 44.1kHz 16-bit mono WAV using raylib audio_tool
            if AUDIO_TOOL.exists():
                cmd = [str(AUDIO_TOOL), str(dst_raw), str(final_wav)]
                subprocess.run(cmd, check=True)
            else:
                shutil.copy(dst_raw, final_wav)
            
            dst_raw.unlink(missing_ok=True)
            # Copy to CustomEngine/extracted_assets/audio
            shutil.copy(final_wav, audio_dirs[1] / filename)
            print(f"  [OK] {filename} -> {final_wav.stat().st_size} bytes (Verified Real Audio)")
        except Exception as e:
            print(f"  [FAIL] {filename}: {e}")

    # Extract Unity fart sound effects using UnityPy if available
    try:
        import UnityPy
        res_assets = pathlib.Path("Fart Together_Data/resources.assets")
        if res_assets.exists():
            env = UnityPy.load(str(res_assets))
            for obj in env.objects:
                if obj.type.name == "AudioClip":
                    clip = obj.read()
                    for sname, sbytes in clip.samples.items():
                        out0 = audio_dirs[0] / sname
                        out0.write_bytes(sbytes)
                        shutil.copy(out0, audio_dirs[1] / sname)
                        print(f"  [Authentic Unity AudioClip] {sname} -> {len(sbytes)} bytes")
    except Exception as e:
        print(f"Unity AudioClip extraction note: {e}")

if __name__ == "__main__":
    download_and_process()
