import math, struct, wave, random, pathlib, shutil, os
from PIL import Image, ImageFilter, ImageDraw

def main():
    root_audio = pathlib.Path("extracted_assets/audio")
    root_tex   = pathlib.Path("extracted_assets/textures")
    ce_audio   = pathlib.Path("CustomEngine/extracted_assets/audio")
    ce_tex     = pathlib.Path("CustomEngine/extracted_assets/textures")

    for d in [root_audio, root_tex, ce_audio, ce_tex]:
        d.mkdir(parents=True, exist_ok=True)

    def save_wav_both(name, samples, sr=44100):
        for out_dir in [root_audio, ce_audio]:
            out_path = out_dir / name
            with wave.open(str(out_path), "w") as wf:
                wf.setnchannels(1)
                wf.setsampwidth(2)
                wf.setframerate(sr)
                buf = bytearray()
                for s in samples:
                    v = max(-32767, min(32767, int(s * 32767.0)))
                    buf += struct.pack("<h", v)
                wf.writeframes(buf)
        print(f"Generated audio: {name} ({len(samples)/sr:.1f}s)")

    sr = 44100

    # 1. menu_beach_ost.wav (24.0s seamless loop)
    dur_ost = 24.0
    n_ost = int(sr * dur_ost)
    ost_samples = [0.0] * n_ost

    chords = [
        (0.0,  [174.61, 220.00, 261.63, 349.23]), # F Major
        (3.0,  [146.83, 174.61, 220.00, 261.63]), # Dm7
        (6.0,  [116.54, 174.61, 233.08, 293.66]), # Bb Major
        (9.0,  [130.81, 196.00, 233.08, 329.63]), # C7
        (12.0, [174.61, 220.00, 261.63, 349.23]), # F Major
        (15.0, [146.83, 220.00, 261.63, 329.63]), # Am / Dm
        (18.0, [116.54, 174.61, 233.08, 349.23]), # Bb Major
        (21.0, [130.81, 196.00, 261.63, 329.63]), # C Major turnaround
    ]

    melody_notes = [
        (0.0, 523.25, 0.45), (0.5, 587.33, 0.45), (1.0, 659.25, 0.70), (1.75, 587.33, 0.45), (2.25, 523.25, 0.65),
        (3.0, 440.00, 0.45), (3.5, 523.25, 0.45), (4.0, 587.33, 0.85), (5.0,  440.00, 0.80),
        (6.0, 466.16, 0.45), (6.5, 523.25, 0.45), (7.0, 587.33, 0.70), (7.75, 698.46, 0.55), (8.35, 659.25, 0.55),
        (9.0, 587.33, 0.50), (9.5, 523.25, 0.50), (10.0, 587.33, 1.40),
        (12.0, 698.46, 0.45), (12.5, 659.25, 0.45), (13.0, 587.33, 0.70), (13.75, 523.25, 0.45), (14.25, 440.00, 0.65),
        (15.0, 523.25, 0.45), (15.5, 587.33, 0.45), (16.0, 659.25, 0.85), (17.0, 523.25, 0.80),
        (18.0, 466.16, 0.45), (18.5, 587.33, 0.45), (19.0, 698.46, 0.75), (19.8, 659.25, 0.55), (20.4, 587.33, 0.55),
        (21.0, 523.25, 0.60), (21.6, 587.33, 0.55), (22.2, 523.25, 1.50),
    ]

    random.seed(42)
    lp1, lp2 = 0.0, 0.0
    for i in range(n_ost):
        t = i / sr
        wn = random.uniform(-1.0, 1.0)
        wave_cycle = 0.50 + 0.50 * math.sin(2.0 * math.pi * t / 8.0 - 0.8)
        cutoff = 0.015 + 0.035 * wave_cycle
        lp1 += cutoff * (wn - lp1)
        lp2 += cutoff * (lp1 - lp2)
        surf = lp2 * (0.14 + 0.16 * wave_cycle)
        ost_samples[i] += surf

    for c_start, freqs in chords:
        for beat in range(6):
            t0 = c_start + beat * 0.5
            f_arp = freqs[beat % len(freqs)] * 2.0
            i0 = int(t0 * sr)
            n_len = int(0.48 * sr)
            for j in range(n_len):
                if i0 + j >= n_ost: break
                tj = j / sr
                env = math.exp(-tj * 6.5) * min(1.0, tj / 0.008)
                sig = (math.sin(2.0 * math.pi * f_arp * tj) + 0.3 * math.sin(4.0 * math.pi * f_arp * tj)) * env * 0.075
                ost_samples[i0 + j] += sig
        i_bass = int(c_start * sr)
        f_bass = freqs[0] * 0.5
        for j in range(int(2.9 * sr)):
            if i_bass + j >= n_ost: break
            tj = j / sr
            env = math.exp(-tj * 1.4) * min(1.0, tj / 0.02)
            ost_samples[i_bass + j] += math.sin(2.0 * math.pi * f_bass * tj) * env * 0.11

    for m_start, f_mel, m_dur in melody_notes:
        i0 = int(m_start * sr)
        n_len = int((m_dur + 0.25) * sr)
        for j in range(n_len):
            if i0 + j >= n_ost: break
            tj = j / sr
            env = math.exp(-tj * 4.8) * min(1.0, tj / 0.005)
            marimba = (math.sin(2.0 * math.pi * f_mel * tj) + 0.25 * math.exp(-tj * 18.0) * math.sin(8.0 * math.pi * f_mel * tj))
            ost_samples[i0 + j] += marimba * env * 0.15

    save_wav_both("menu_beach_ost.wav", ost_samples, sr)

    # 2. seagull_call.wav
    n_gull = int(sr * 2.2)
    gull = [0.0] * n_gull
    for call_t, dur_c, f_peak in [(0.1, 0.65, 2150.0), (0.95, 0.95, 2350.0)]:
        i0 = int(call_t * sr)
        for j in range(int(dur_c * sr)):
            tj = j / sr
            u = tj / dur_c
            freq = f_peak * (0.72 + 0.38 * math.sin(math.pi * (u ** 0.65)) - 0.18 * u)
            freq += 18.0 * math.sin(2.0 * math.pi * 28.0 * tj)
            env = math.sin(math.pi * u) ** 1.4
            ph = 2.0 * math.pi * freq * tj
            gull[i0 + j] += (0.65 * math.sin(ph) + 0.25 * math.sin(2.0 * ph) + 0.10 * math.sin(3.0 * ph)) * env * 0.28
    save_wav_both("seagull_call.wav", gull, sr)

    # 3. ship_horn.wav
    n_horn = int(sr * 3.4)
    horn = [0.0] * n_horn
    for i in range(n_horn):
        t = i / sr
        env = min(1.0, t / 0.35) * math.exp(-max(0.0, t - 1.8) * 2.2)
        s = 0.0
        for f_h, amp in [(98.0, 0.45), (146.83, 0.30), (196.0, 0.18), (293.66, 0.08)]:
            s += amp * math.sin(2.0 * math.pi * f_h * t)
        horn[i] = math.tanh(s * 1.4) * env * 0.28
    save_wav_both("ship_horn.wav", horn, sr)

    # 4. snore_loop.wav
    n_snore = int(sr * 3.6)
    snore = [0.0] * n_snore
    for i in range(n_snore):
        t = i / sr
        if t < 1.5:
            u = t / 1.5
            env = math.sin(math.pi * u)
            rasp = math.sin(2.0 * math.pi * 68.0 * t + 2.5 * math.sin(2.0 * math.pi * 22.0 * t))
            snore[i] = rasp * env * 0.30
        elif 1.8 <= t < 3.2:
            u = (t - 1.8) / 1.4
            env = math.sin(math.pi * u)
            whistle = math.sin(2.0 * math.pi * (620.0 - 140.0 * u) * t)
            snore[i] = whistle * env * 0.18
    save_wav_both("snore_loop.wav", snore, sr)

    # 5. eat_munch.wav
    n_eat = int(sr * 1.2)
    eat = [0.0] * n_eat
    for crunch_t in [0.05, 0.38, 0.72]:
        i0 = int(crunch_t * sr)
        for j in range(int(0.22 * sr)):
            tj = j / sr
            env = math.exp(-tj * 22.0)
            crunch = (random.uniform(-1.0, 1.0) * 0.6 + math.sin(2.0 * math.pi * (240.0 - tj * 500.0) * tj) * 0.4)
            eat[i0 + j] += crunch * env * 0.42
    save_wav_both("eat_munch.wav", eat, sr)

    # 6. burp.wav
    n_burp = int(sr * 0.95)
    burp = [0.0] * n_burp
    for i in range(n_burp):
        t = i / sr
        env = min(1.0, t / 0.04) * math.exp(-t * 3.8)
        f_b = 115.0 + 25.0 * math.sin(2.0 * math.pi * 9.0 * t) - 30.0 * t
        s = math.sin(2.0 * math.pi * f_b * t + 1.8 * math.sin(2.0 * math.pi * f_b * 0.5 * t))
        burp[i] = math.tanh(s * 2.2) * env * 0.45
    save_wav_both("burp.wav", burp, sr)

    # 7. checkpoint_chime.wav
    n_cp = int(sr * 1.1)
    cp_snd = [0.0] * n_cp
    for note_t, f_cp in [(0.0, 523.25), (0.11, 659.25), (0.22, 783.99), (0.34, 1046.50)]:
        i0 = int(note_t * sr)
        for j in range(int((1.05 - note_t) * sr)):
            tj = j / sr
            env = math.exp(-tj * 5.2) * min(1.0, tj / 0.005)
            cp_snd[i0 + j] += (math.sin(2.0 * math.pi * f_cp * tj) + 0.35 * math.sin(4.0 * math.pi * f_cp * tj)) * env * 0.25
    save_wav_both("checkpoint_chime.wav", cp_snd, sr)

    # Process Menu Background Layers from Image 2
    raw_img_path = r"C:/Users/ghenemi/.gemini/antigravity/brain/4ed397dd-3767-496a-a001-6720de8fc85e/.user_uploaded/media_1793456826528_c9906ef8.jpg"
    if not os.path.exists(raw_img_path):
        print(f"ERROR: {raw_img_path} not found!")
        return

    full_img = Image.open(raw_img_path).convert("RGB").resize((1280, 720), Image.Resampling.LANCZOS)
    for out_d in [root_tex, ce_tex]:
        full_img.save(out_d / "Menu_Oasis_Full.png")
    print("Saved Menu_Oasis_Full.png (1280x720)")

    # 1. Menu_Pirate_SleepingRaft.png: RGBA cutout of the raft & sleeping purple hippo
    # Raft is located at X in [540, 800], Y in [320, 515] in 1280x720
    raft_w = 260
    raft_h = 196
    rx0, ry0 = 540, 320
    raft_crop = full_img.crop((rx0, ry0, rx0 + raft_w, ry0 + raft_h)).convert("RGBA")
    
    # Build a feathered elliptical / contour mask for smooth raft edges
    mask = Image.new("L", (raft_w, raft_h), 0)
    draw = ImageDraw.Draw(mask)
    # The raft oval is roughly: center (130, 110), rx = 120, ry = 80
    draw.ellipse([8, 15, raft_w - 8, raft_h - 10], fill=255)
    # Upper left white pillow / hippo head:
    draw.ellipse([4, 18, 120, 115], fill=255)
    mask = mask.filter(ImageFilter.GaussianBlur(3.5))

    raft_rgba = raft_crop.copy()
    raft_rgba.putalpha(mask)
    for out_d in [root_tex, ce_tex]:
        raft_rgba.save(out_d / "Menu_Pirate_SleepingRaft.png")
    print("Saved Menu_Pirate_SleepingRaft.png")

    # 2. Menu_Oasis_CleanPond.png: The background with the pond filled with water so raft can move cleanly
    clean_pond = full_img.copy()
    # Sample clean water color from pond (35, 185, 185) with gentle radial gradient
    water_patch = Image.new("RGB", (raft_w + 40, raft_h + 40), (32, 175, 178))
    wp_draw = ImageDraw.Draw(water_patch)
    for r in range(120, 0, -5):
        alpha = int(255 * (r / 120))
        c = (int(25 + 15 * (1 - r/120)), int(185 + 25 * (1 - r/120)), int(188 + 20 * (1 - r/120)))
        wp_draw.ellipse([raft_w//2 + 20 - r, raft_h//2 + 20 - int(r*0.65), raft_w//2 + 20 + r, raft_h//2 + 20 + int(r*0.65)], outline=c, width=3)
    water_patch = water_patch.filter(ImageFilter.GaussianBlur(6.0))

    patch_mask = Image.new("L", (raft_w + 40, raft_h + 40), 0)
    p_draw = ImageDraw.Draw(patch_mask)
    p_draw.ellipse([10, 10, raft_w + 30, raft_h + 30], fill=255)
    patch_mask = patch_mask.filter(ImageFilter.GaussianBlur(8.0))

    clean_pond.paste(water_patch, (rx0 - 20, ry0 - 20), patch_mask)
    for out_d in [root_tex, ce_tex]:
        clean_pond.save(out_d / "Menu_Oasis_CleanPond.png")
    print("Saved Menu_Oasis_CleanPond.png")

if __name__ == "__main__":
    main()
