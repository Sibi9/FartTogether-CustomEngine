# ==============================================================================
# Fart Together - GitHub Project Uploader
# Automatic GitHub Repository Creator, Authenticator & Real-Time Upload Monitor
# ==============================================================================

import sys
import os
import re
import time
import subprocess
import webbrowser

# Enable Windows UTF-8 and ANSI terminal processing
if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
        sys.stderr.reconfigure(encoding='utf-8', errors='replace')
    except Exception:
        pass
    os.system('') # Activates ANSI escape sequences on Windows 10/11

# ANSI Color Codes
CYAN    = "\033[96m"
GREEN   = "\033[92m"
YELLOW  = "\033[93m"
RED     = "\033[91m"
BOLD    = "\033[1m"
DIM     = "\033[2m"
RESET   = "\033[0m"

def print_banner():
    os.system('cls' if os.name == 'nt' else 'clear')
    print(f"{CYAN}{BOLD}" + "═" * 78)
    print("   🎮 أداة رفع مشروع Fart Together إلى GitHub التلقائية")
    print("   Automatic GitHub Project Uploader with Live Progress & Speed Tracker")
    print("═" * 78 + f"{RESET}\n")

def format_time(seconds):
    if seconds is None or seconds < 0 or seconds > 86400 * 7:
        return "جاري الحساب..."
    secs = int(seconds)
    if secs < 60:
        return f"{secs} ثانية"
    mins = secs // 60
    rem_secs = secs % 60
    if mins < 60:
        return f"{mins} دقيقة و {rem_secs} ثانية"
    hours = mins // 60
    rem_mins = mins % 60
    return f"{hours} ساعة و {rem_mins} دقيقة"

def render_progress(phase, pct, curr, total, size_str, speed_str, eta_str, elapsed_str):
    bar_width = 32
    clamped_pct = max(0.0, min(100.0, pct))
    filled = int(bar_width * (clamped_pct / 100.0))
    bar = "█" * filled + "░" * (bar_width - filled)
    
    lines = [
        f"\r\033[K{CYAN}" + "─" * 74 + f"{RESET}",
        f"\r\033[K  {BOLD}🚀 المرحلة الحالية:{RESET} {YELLOW}{phase}{RESET}",
        f"\r\033[K  [{GREEN}{bar}{RESET}] {BOLD}{clamped_pct:5.1f}%{RESET}",
        f"\r\033[K  " + "-" * 70,
        f"\r\033[K  📦 {BOLD}الكائنات المرفوعة :{RESET} {curr} / {total} كائن",
        f"\r\033[K  💾 {BOLD}حجم البيانات المنقولة:{RESET} {GREEN}{size_str}{RESET}",
        f"\r\033[K  ⚡ {BOLD}سرعة الرفع الحالية   :{RESET} {CYAN}{speed_str}{RESET}",
        f"\r\033[K  ⏳ {BOLD}الوقت المتبقي التقديري:{RESET} {YELLOW}{eta_str}{RESET}",
        f"\r\033[K  ⏱️  {BOLD}الوقت المنقضي         :{RESET} {elapsed_str}",
        f"\r\033[K{CYAN}" + "─" * 74 + f"{RESET}",
        f"\033[10A"
    ]
    sys.stdout.write("\n".join(lines) + "\n")
    sys.stdout.flush()

def ensure_gitignore():
    gitignore_path = ".gitignore"
    if not os.path.exists(gitignore_path):
        content = """# MSVC Build Artifacts
/GameEngine.obj
/test_compile.obj
/test_compile.exe
*.pdb
*.ilk
*.log
.system_generated/
CustomEngine/scratch/
"""
        with open(gitignore_path, "w", encoding="utf-8") as f:
            f.write(content)
        print(f"{DIM}تم إنشاء ملف .gitignore لحماية المستودع من مخلفات الترجمة المؤقتة.{RESET}")

def main():
    print_banner()

    # Step 1: Account Information
    print(f"{YELLOW}{BOLD}[الخطوة 1 من 4] معلومات حسابك في GitHub:{RESET}")
    print("أدخل بريدك الإلكتروني المرتبط بحسابك على GitHub (أو اسم المستخدم):")
    email_or_user = input(f"{BOLD}GitHub Email / Username > {RESET}").strip()
    while not email_or_user:
        print(f"{RED}يرجى إدخال البريد الإلكتروني أو اسم المستخدم للمتابعة:{RESET}")
        email_or_user = input(f"{BOLD}GitHub Email / Username > {RESET}").strip()

    # Determine username from email or input
    if "@" in email_or_user:
        default_user = email_or_user.split("@")[0]
        user_email = email_or_user
    else:
        default_user = email_or_user
        user_email = f"{email_or_user}@users.noreply.github.com"

    print(f"\nأدخل اسمك المعروض (Display Name) أو اضغط {BOLD}Enter{RESET} لاستخدام [{default_user}]:")
    custom_name = input(f"{BOLD}Display Name [{default_user}] > {RESET}").strip()
    user_name = custom_name if custom_name else default_user

    # Apply Git Config
    subprocess.run(["git", "config", "--global", "user.email", user_email], check=False)
    subprocess.run(["git", "config", "--global", "user.name", user_name], check=False)
    subprocess.run(["git", "config", "core.longpaths", "true"], check=False)
    subprocess.run(["git", "config", "http.postBuffer", "524288000"], check=False)
    subprocess.run(["git", "config", "http.lowSpeedLimit", "0"], check=False)
    subprocess.run(["git", "config", "http.lowSpeedTime", "999999"], check=False)

    print(f"{GREEN}✔ تم ضبط إعدادات Git بنجاح ({user_name} <{user_email}>){RESET}\n")

    # Step 2: Passwordless Authentication
    print(f"{YELLOW}{BOLD}[الخطوة 2 من 4] تسجيل الدخول إلى GitHub (بدون كتابة كلمة سر):{RESET}")
    print("اختر طريقة تسجيل الدخول السهلة والآمنة:")
    print(f"  {BOLD}[1]{RESET} تسجيل الدخول بضغطة زر عبر المتصفح ({GREEN}مستحسن - الأسهل والأسرع{RESET})")
    print(f"  {BOLD}[2]{RESET} استخدام رمز الجهاز (Device Code) عبر الرابط https://github.com/login/device")
    print(f"  {BOLD}[3]{RESET} استخدام رمز وصول شخصي (Personal Access Token - PAT)")
    print(f"  {BOLD}[4]{RESET} الحساب مسجل مسبقاً / تخطي خطوة تسجيل الدخول")
    
    choice = input(f"{BOLD}اختر [1]: {RESET}").strip()
    if not choice:
        choice = "1"

    if choice == "1":
        print(f"\n{CYAN}جاري فتح المتصفح للموافقة على تسجيل الدخول بضغطة زر...{RESET}")
        cmd = ["git", "credential-manager", "github", "login", "--browser"]
        if default_user:
            cmd.extend(["--username", default_user])
        res = subprocess.run(cmd)
        if res.returncode == 0:
            print(f"{GREEN}✔ تم تسجيل الدخول والمصادقة مع GitHub بنجاح!{RESET}")
        else:
            print(f"{YELLOW}تنبيه: ستتم مطالبتك بالمصادقة تلقائياً عند بدء عملية الرفع.{RESET}")

    elif choice == "2":
        print(f"\n{CYAN}جاري بدء مصادقة رمز الجهاز (Device Code)...{RESET}")
        cmd = ["git", "credential-manager", "github", "login", "--device"]
        if default_user:
            cmd.extend(["--username", default_user])
        subprocess.run(cmd)

    elif choice == "3":
        cmd = ["git", "credential-manager", "github", "login", "--pat"]
        if default_user:
            cmd.extend(["--username", default_user])
        subprocess.run(cmd)

    # Step 3: Repository Target
    print(f"\n{YELLOW}{BOLD}[الخطوة 3 من 4] تحديد مستودع GitHub المستهدف:{RESET}")
    default_repo_name = "FartTogether-CustomEngine"
    print(f"أدخل اسم المستودع أو الرابط كاملاً (اضغط {BOLD}Enter{RESET} للاسم الافتراضي: {CYAN}{default_repo_name}{RESET}):")
    repo_input = input(f"{BOLD}Repository Name or URL [{default_repo_name}] > {RESET}").strip()
    if not repo_input:
        repo_input = default_repo_name

    if repo_input.startswith("http://") or repo_input.startswith("https://") or repo_input.startswith("git@"):
        repo_url = repo_input
        repo_name = repo_input.rstrip("/").split("/")[-1].replace(".git", "")
    else:
        repo_name = repo_input
        repo_url = f"https://github.com/{default_user}/{repo_name}.git"

    print(f"\n{CYAN}المستودع المستهدف: {BOLD}{repo_url}{RESET}")
    print(f"\n{YELLOW}تأكد من إنشاء هذا المستودع على حسابك في GitHub إذا لم تكن قد أنشأته بعد.{RESET}")
    print("هل تريد فتح صفحة إنشاء المستودع في المتصفح الآن؟")
    print(f"  اضغط {BOLD}Enter{RESET} للمتابعة للرفع مباشرة")
    print(f"  أو اكتب {BOLD}o{RESET} لفتح https://github.com/new في المتصفح")
    open_choice = input(f"{BOLD}اختيارك > {RESET}").strip().lower()
    if open_choice == 'o':
        create_url = f"https://github.com/new?name={repo_name}&description=Fart+Together+Native+Engine+and+Assets"
        webbrowser.open(create_url)
        print(f"{CYAN}تم فتح صفحة إنشاء المستودع في المتصفح. أنشئ المستودع ثم اضغط Enter هنا للمتابعة...{RESET}")
        input(f"{BOLD}[اضغط Enter بعد الانتهاء] > {RESET}")

    # Step 4: Git Init, Commit & Prep
    print(f"\n{YELLOW}{BOLD}[الخطوة 4 من 4] تجهيز ملفات المشروع محلياً:{RESET}")
    ensure_gitignore()

    if not os.path.exists(".git"):
        print(f"{DIM}جاري تهيئة مستودع Git محلي...{RESET}")
        subprocess.run(["git", "init"], check=True, stdout=subprocess.DEVNULL)

    subprocess.run(["git", "branch", "-M", "main"], check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    print(f"{CYAN}جاري فحص وفهرسة الملفات (git add .)... قد يستغرق لحظات لحجم المشروع...{RESET}")
    subprocess.run(["git", "add", "."], check=True)

    # Check status
    st = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)
    if st.stdout.strip():
        print(f"{CYAN}جاري تسجيل التغييرات (Commit)...{RESET}")
        commit_msg = "Initial commit: Fart Together Native C++17 Custom Engine & Authentic Assets"
        subprocess.run(["git", "commit", "-m", commit_msg], check=True, stdout=subprocess.DEVNULL)
    else:
        print(f"{DIM}كافة الملفات مسجلة ومحفوظة مسبقاً.{RESET}")

    # Setup Remote
    subprocess.run(["git", "remote", "remove", "origin"], check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["git", "remote", "add", "origin", repo_url], check=True)

    # Launch Upload (git push --progress)
    print(f"\n{GREEN}{BOLD}══════════════════════════════════════════════════════════════════════{RESET}")
    print(f"{GREEN}{BOLD}   🚀 بدء عملية الرفع إلى GitHub مع مراقبة السرعة والوقت المتبقي...{RESET}")
    print(f"{GREEN}{BOLD}══════════════════════════════════════════════════════════════════════{RESET}\n")

    t_start = time.time()
    t_write_start = None

    proc = subprocess.Popen(
        ["git", "push", "-u", "origin", "main", "--progress"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        encoding='utf-8',
        errors='replace'
    )

    re_write = re.compile(
        r'Writing objects:\s+(\d+)%\s+\((\d+)/(\d+)\)(?:,\s+([\d\.]+\s+[KkMmGg]?i?B))?(?:\s+\|\s+([\d\.]+\s+[KkMmGg]?i?B/s))?(?:,\s+([0-9:]+|done))?'
    )
    re_count = re.compile(r'Counting objects:\s+(\d+)%\s+\((\d+)/(\d+)\)')
    re_comp  = re.compile(r'Compressing objects:\s+(\d+)%\s+\((\d+)/(\d+)\)')

    last_pct = 0.0
    last_curr = 0
    last_total = 0
    last_size = "0 MB"
    last_speed = "جاري القياس..."
    last_eta = "جاري الحساب..."

    buffer = ""
    in_box_mode = False

    while True:
        ch = proc.stderr.read(1)
        if not ch and proc.poll() is not None:
            break
        if ch in ('\r', '\n'):
            line = buffer.strip()
            buffer = ""
            if not line:
                continue

            # Check Writing objects
            mw = re_write.search(line)
            if mw:
                if t_write_start is None:
                    t_write_start = time.time()
                
                pct_val = float(mw.group(1))
                curr_val = int(mw.group(2))
                tot_val = int(mw.group(3))
                size_val = mw.group(4) or last_size
                speed_val = mw.group(5) or last_speed
                git_eta = mw.group(6)

                last_pct = pct_val
                last_curr = curr_val
                last_total = tot_val
                last_size = size_val
                last_speed = speed_val

                # Calculate ETA
                elapsed_write = time.time() - t_write_start
                if git_eta and git_eta != "done":
                    eta_text = f"{git_eta}"
                elif pct_val > 0:
                    total_est = elapsed_write / (pct_val / 100.0)
                    rem_sec = total_est - elapsed_write
                    eta_text = format_time(rem_sec)
                else:
                    eta_text = "جاري الحساب..."

                elapsed_text = format_time(time.time() - t_start)
                render_progress("رفع البيانات إلى المستودع (Uploading Packfile)", pct_val, curr_val, tot_val, size_val, speed_val, eta_text, elapsed_text)
                in_box_mode = True
                continue

            # Check Counting objects
            mc = re_count.search(line)
            if mc:
                pct_val = float(mc.group(1))
                curr_val = int(mc.group(2))
                tot_val = int(mc.group(3))
                elapsed_text = format_time(time.time() - t_start)
                render_progress("إحصاء وفهرسة الكائنات (Counting Objects)", pct_val, curr_val, tot_val, "0 MB", "--", "ثوانٍ معدودة", elapsed_text)
                in_box_mode = True
                continue

            # Check Compressing objects
            mp = re_comp.search(line)
            if mp:
                pct_val = float(mp.group(1))
                curr_val = int(mp.group(2))
                tot_val = int(mp.group(3))
                elapsed_text = format_time(time.time() - t_start)
                render_progress("ضغط وتجهيز الملفات (Compressing Objects)", pct_val, curr_val, tot_val, "0 MB", "--", "ثوانٍ معدودة", elapsed_text)
                in_box_mode = True
                continue

            # Other important status messages
            if not in_box_mode and line:
                print(f"{DIM}> {line}{RESET}")

        else:
            buffer += ch

    # Ensure cursor clears box area
    if in_box_mode:
        sys.stdout.write("\n" * 11)

    proc.wait()
    total_duration = time.time() - t_start

    if proc.returncode == 0:
        print(f"\n{GREEN}{BOLD}══════════════════════════════════════════════════════════════════════{RESET}")
        print(f"{GREEN}{BOLD}   🎉 تم رفع المشروع بالكامل إلى GitHub بنجاح!{RESET}")
        print(f"{GREEN}   🔗 رابط المستودع: {BOLD}{repo_url}{RESET}")
        print(f"{GREEN}   ⏱️ إجمالي وقت الرفع: {format_time(total_duration)}{RESET}")
        print(f"{GREEN}{BOLD}══════════════════════════════════════════════════════════════════════{RESET}\n")
    else:
        print(f"\n{RED}{BOLD}══════════════════════════════════════════════════════════════════════{RESET}")
        print(f"{RED}{BOLD}   ❌ حدث خطأ أثناء عملية الرفع (Exit code: {proc.returncode}){RESET}")
        print(f"{YELLOW}   ملاحظة: إذا كان الخطأ بسبب عدم وجود المستودع، تأكد من إنشائه على:")
        print(f"   https://github.com/new?name={repo_name}")
        print(f"{RED}{BOLD}══════════════════════════════════════════════════════════════════════{RESET}\n")

    input(f"\n{BOLD}اضغط Enter لإغلاق هذه النافذة...{RESET}")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print(f"\n{YELLOW}تم إيقاف العملية بواسطة المستخدم.{RESET}")
        sys.exit(0)

