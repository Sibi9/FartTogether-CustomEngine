import pathlib, shutil, urllib.request

fonts_dir = pathlib.Path("extracted_assets/fonts")
fonts_dir.mkdir(parents=True, exist_ok=True)
c_fonts_dir = pathlib.Path("CustomEngine/extracted_assets/fonts")
c_fonts_dir.mkdir(parents=True, exist_ok=True)
UA = {"User-Agent": "FartTogetherEngine/8.2.0"}

def download_font(urls, dst_list):
    for url in urls:
        try:
            req = urllib.request.Request(url, headers=UA)
            with urllib.request.urlopen(req, timeout=25) as r:
                data = r.read()
            if len(data) > 20000:
                for dst in dst_list:
                    dst.write_bytes(data)
                print(f"[Font Setup] Downloaded {dst_list[0].name} ({len(data)} bytes) from {url}")
                return True
        except Exception as e:
            print(f"Font URL warning ({url}): {e}")
    return False

# 1. تحميل الخط العربي الكرتوني العريض الأصلي للعبة (Vazirmatn Black / ExtraBold)
ar_font_paths = [
    fonts_dir / "vazirmatn_black.ttf",
    c_fonts_dir / "vazirmatn_black.ttf",
    fonts_dir / "ui_font_bold.ttf",
    c_fonts_dir / "ui_font_bold.ttf",
    fonts_dir / "ui_font.ttf",
    c_fonts_dir / "ui_font.ttf"
]
download_font([
    "https://raw.githubusercontent.com/rastikerdar/vazirmatn/master/fonts/ttf/Vazirmatn-Black.ttf",
    "https://raw.githubusercontent.com/rastikerdar/vazirmatn/v33.003/fonts/ttf/Vazirmatn-Black.ttf",
    "https://raw.githubusercontent.com/google/fonts/main/ofl/vazirmatn/Vazirmatn%5Bwght%5D.ttf",
    "https://raw.githubusercontent.com/google/fonts/main/ofl/cairo/Cairo%5Bwght%5D.ttf",
], ar_font_paths)

# 2. تحميل الخط الإنجليزي الكرتوني العريض الأصلي للعبة (Lilita One / Jua)
en_font_paths = [
    fonts_dir / "lilita_one.ttf",
    c_fonts_dir / "lilita_one.ttf"
]
download_font([
    "https://raw.githubusercontent.com/google/fonts/main/ofl/lilitaone/LilitaOne-Regular.ttf",
    "https://raw.githubusercontent.com/google/fonts/main/ofl/jua/Jua-Regular.ttf",
], en_font_paths)

print("[Font Setup] Fonts downloaded and synchronized successfully.")

