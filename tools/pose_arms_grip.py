#!/usr/bin/env python3
"""
pose_arms_grip.py
يعدّل وضعية Pirate_SeatedA.obj بحيث تنزل اليدان وتستقران على سطح الأرضية أمام القرصان
(بدل التدلي بجانب الجسم) دون Blender، بتقنية Linear Blend Skinning بعظام افتراضية:

  1) انحناء إضافي للجزء العلوي من الجسم حول الورك (spine pitch) بوزن ناعم (القدمان والمؤخرة ثابتان)
  2) لكل ذراع: عظمة A (كتف -> رسغ) تُدار بدوران أقصر قوس + تمدد خفيف حتى يصل الرسغ للهدف
  3) عظمة اليد B تُدار حول الرسغ لتصير اليد مسطحة (أفقية) على الأرضية
  4) الأوزان من كبسولة حول مسار الذراع (كتف->كوع->رسغ->أصابع) مع تلاشٍ ناعم عند الكتف
  5) تُحوَّل الـ normals بنفس المزج، وتبقى الـ UV والوجوه وعدد وترتيب الرؤوس كما هي
     (مهم: فهارس رؤوس العينين والمؤخرة في الشيدر لا تتغير)

الاستخدام:
  python3 pose_arms_grip.py --in Pirate_SeatedA.obj --out Pirate_SeatedA_Grip.obj \
         --pitch 26 --hand-x 0.20 --hand-z 0.55 --deck-y 0.008 --preview grip_preview.png

كل الإحداثيات في فضاء نموذج القرصان (قدماه عند y≈0، وجهه نحو +Z).
deck-y = (ارتفاع سطح الأرضية بفضاء القارب 0.518) - kRimSeatHeight(0.51) = 0.008
"""
import argparse, json, math
import numpy as np

PIVOT = np.array([0.0, 0.34, 0.0])           # محور الورك (نفس محور lean في المحرك)
# مسار الذراع الأيمن المقاس من المجسم: كتف، كوع، رسغ، طرف الأصابع
ARM_S = np.array([0.30, 0.64, 0.19])
ARM_E = np.array([0.40, 0.40, 0.26])
ARM_W = np.array([0.45, 0.22, 0.35])
ARM_T = np.array([0.45, 0.19, 0.43])
HAND_CENTER0 = np.array([0.45, 0.193, 0.362])  # مركز رقعة اليد الأصلية


def smoothstep(a, b, x):
    t = np.clip((x - a) / (b - a), 0.0, 1.0)
    return t * t * (3 - 2 * t)


def rot_x(deg):
    t = math.radians(deg); c, s = math.cos(t), math.sin(t)
    return np.array([[1, 0, 0], [0, c, -s], [0, s, c]])


def rot_between(a, b):
    """أقصر دوران يحول المتجه a إلى b."""
    a = a / np.linalg.norm(a); b = b / np.linalg.norm(b)
    v = np.cross(a, b); c = float(np.dot(a, b))
    if c > 0.999999:
        return np.eye(3)
    if c < -0.999999:
        return np.diag([1, -1, -1]).astype(float)
    vx = np.array([[0, -v[2], v[1]], [v[2], 0, -v[0]], [-v[1], v[0], 0]])
    return np.eye(3) + vx + vx @ vx * (1.0 / (1.0 + c))


def seg_dist(P, A, B):
    """مسافة كل نقطة من القطعة AB + معامل الإسقاط t (0..1)."""
    AB = B - A; L2 = float(AB @ AB)
    t = np.clip(((P - A) @ AB) / L2, 0, 1)
    proj = A + t[:, None] * AB
    return np.linalg.norm(P - proj, axis=1), t


def polyline_dist(P, pts):
    """مسافة النقاط من مسار متعدد القطع + طول القوس عند أقرب نقطة."""
    best_d = np.full(len(P), 1e9); best_s = np.zeros(len(P)); acc = 0.0
    for A, B in zip(pts[:-1], pts[1:]):
        d, t = seg_dist(P, A, B)
        L = np.linalg.norm(B - A)
        m = d < best_d
        best_d[m] = d[m]; best_s[m] = acc + t[m] * L
        acc += L
    return best_d, best_s


def load_obj(fn):
    lines = open(fn, encoding="utf-8", errors="ignore").read().split("\n")
    vi = [i for i, l in enumerate(lines) if l.startswith("v ")]
    ni = [i for i, l in enumerate(lines) if l.startswith("vn ")]
    V = np.array([[float(x) for x in lines[i].split()[1:4]] for i in vi])
    N = np.array([[float(x) for x in lines[i].split()[1:4]] for i in ni])
    return lines, vi, ni, V, N


def pose(V, N, side_sign_list, pitch, hand_x, hand_y, hand_z, kmax):
    """يطبق الانحناء + تثبيت اليدين. يعيد (V', N', info)."""
    Rsp = rot_x(pitch)
    # وزن العمود الفقري: 0 للساقين والمؤخرة السفلية، 1 للجذع والرأس والذراعين
    ws = np.maximum(smoothstep(0.30, 0.46, V[:, 1]), smoothstep(0.24, 0.30, np.abs(V[:, 0])))
    Psp = PIVOT + (V - PIVOT) @ Rsp.T
    P0 = (1 - ws)[:, None] * V + ws[:, None] * Psp
    N0 = (1 - ws)[:, None] * N + ws[:, None] * (N @ Rsp.T)

    Vout = P0.copy(); Nout = N0.copy()
    info = {}
    for sgn in side_sign_list:
        m = np.array([sgn, 1, 1])
        pts = [ARM_S * m, ARM_E * m, ARM_W * m, ARM_T * m]
        d, s = polyline_dist(V, pts)                     # الأوزان تُحسب على الشبكة الأصلية
        arm_len_w = np.linalg.norm(pts[1] - pts[0]) + np.linalg.norm(pts[2] - pts[1])
        w_cap = 1.0 - smoothstep(0.075, 0.125, d)
        w_cap *= smoothstep(0.25, 0.31, np.abs(V[:, 0]))      # لا تلتقط جانب الجذع
        w_sh = smoothstep(0.0, 0.14, s)                       # تلاشٍ عند الكتف
        warm = w_cap * w_sh                                   # وزن الذراع الكلي
        hb = smoothstep(arm_len_w - 0.05, arm_len_w + 0.04, s)  # وزن عظمة اليد

        # مفاصل بعد انحناء العمود الفقري
        def spj(p): return PIVOT + Rsp @ (p - PIVOT)
        S1 = spj(pts[0]); W1 = spj(pts[2]); T1 = spj(pts[3]); HC1 = spj(HAND_CENTER0 * m)

        # الهدف: مركز اليد
        target = np.array([sgn * hand_x, hand_y, hand_z])
        # الرسغ المستهدف = الهدف - إزاحة (مركز اليد - الرسغ) بعد أن تصير اليد أفقية تقريباً
        wrist_off = np.array([0.0, 0.0, -0.075])              # اليد أفقية تشير للأمام (+z)
        Wt = target + wrist_off
        reach = np.linalg.norm(Wt - S1); L = np.linalg.norm(W1 - S1)
        k = float(np.clip(reach / L, 1.0, kmax))
        RA = rot_between(W1 - S1, Wt - S1)
        # عظمة A
        def tA(P): return S1 + k * ((P - S1) @ RA.T)
        W_A = tA(W1); T_A = tA(T1)
        # عظمة اليد B: اجعل اتجاه اليد (الرسغ->الأصابع) أفقياً للأمام
        dhand = T_A - W_A
        RB = rot_between(dhand, np.array([0.0, 0.0, 1.0]))
        def tB(P): return W_A + (P - W_A) @ RB.T
        PA = tA(P0); PB = tB(PA)
        parm = (1 - hb)[:, None] * PA + hb[:, None] * PB
        Vout = (1 - warm)[:, None] * Vout + warm[:, None] * parm
        # normals
        NA = N0 @ RA.T; NB = NA @ RB.T
        narm = (1 - hb)[:, None] * NA + hb[:, None] * NB
        Nout = (1 - warm)[:, None] * Nout + warm[:, None] * narm
        info[f"side{sgn:+d}"] = dict(stretch=round(k, 3), reach=round(float(reach), 3),
                                     arm_len=round(float(L), 3), reachable=bool(reach <= L * kmax + 1e-6),
                                     verts=int((warm > 0.5).sum()))
    Nout /= np.maximum(np.linalg.norm(Nout, axis=1, keepdims=True), 1e-9)
    return Vout, Nout, info, ws


def hand_min_y(V, sgn):
    m = (V[:, 0] * sgn > 0.12) & (V[:, 2] > 0.30)           # منطقة اليد بعد الوضعية
    return float(V[m, 1].min()) if m.any() else float("nan")


def save_obj(lines, vi, ni, V, N, fn, note):
    out = list(lines)
    for i, p in zip(vi, V): out[i] = "v %.6f %.6f %.6f" % tuple(p)
    for i, n in zip(ni, N): out[i] = "vn %.6f %.6f %.6f" % tuple(n)
    out.insert(0, "# " + note)
    open(fn, "w", encoding="utf-8").write("\n".join(out))


def preview(V0, V1, F, N0, N1, path, deck_y):
    import matplotlib; matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from matplotlib.collections import PolyCollection
    L = np.array([0.4, 0.6, 0.7]); L /= np.linalg.norm(L)
    views = [("side (+Z right)", lambda V: (V[:, 2], V[:, 1], V[:, 0])),
             ("back", lambda V: (-V[:, 0], V[:, 1], -V[:, 2])),
             ("top (+Z up)", lambda V: (V[:, 0], V[:, 2], V[:, 1]))]
    fig, axs = plt.subplots(2, 3, figsize=(15, 9))
    for r, (V, N, name) in enumerate([(V0, N0, "BEFORE"), (V1, N1, "AFTER")]):
        for c, (vn, fn) in enumerate(views):
            ax = axs[r, c]; u, v, d = fn(V)
            fc = np.array([N[f].mean(0) @ L for f in F]); fc = 0.35 + 0.55 * np.clip(fc, 0, 1)
            tris = [np.c_[u[f], v[f]] for f in F]; dep = [d[f].mean() for f in F]
            o = np.argsort(dep)
            ax.add_collection(PolyCollection([tris[i] for i in o], facecolors=[(fc[i] * 0.9, fc[i] * 0.6, fc[i] * 0.3) for i in o], edgecolors="none"))
            ax.set_xlim(-0.8, 0.9); ax.set_ylim(-0.15, 1.15) if c < 2 else ax.set_ylim(-0.5, 0.9)
            ax.set_aspect("equal"); ax.grid(alpha=.3); ax.set_title(f"{name} - {vn}")
            if c == 0: ax.axhline(deck_y, color="g", ls="--", lw=1)
    plt.tight_layout(); plt.savefig(path, dpi=65); plt.close(fig)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in", dest="inp", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--pitch", type=float, default=26.0, help="انحناء إضافي للجذع (درجات)")
    ap.add_argument("--hand-x", type=float, default=0.20)
    ap.add_argument("--hand-z", type=float, default=0.55)
    ap.add_argument("--deck-y", type=float, default=0.008, help="ارتفاع سطح الأرضية بفضاء القرصان")
    ap.add_argument("--hand-half-h", type=float, default=0.03, help="نصف ارتفاع اليد المسطحة")
    ap.add_argument("--kmax", type=float, default=1.15, help="أقصى تمدد للذراع")
    ap.add_argument("--preview", default=None)
    a = ap.parse_args()

    lines, vi, ni, V, N = load_obj(a.inp)
    F = [[int(p.split("/")[0]) - 1 for p in l.split()[1:4]] for l in lines if l.startswith("f ")]
    hy = a.deck_y + a.hand_half_h
    for it in range(3):          # تكرار لضبط ارتفاع اليد حتى تلامس الأرضية
        V1, N1, info, ws = pose(V, N, (+1, -1), a.pitch, a.hand_x, hy, a.hand_z, a.kmax)
        miny = min(hand_min_y(V1, +1), hand_min_y(V1, -1))
        err = miny - a.deck_y
        if abs(err) < 0.003: break
        hy -= err
    rep = dict(params=vars(a), arms=info, hand_min_y=round(miny, 4), hand_vs_deck=round(miny - a.deck_y, 4),
               head_front_z_before=round(float(V[V[:, 1] > 0.8, 2].max()), 3),
               head_min_y_after=round(float(V1[V[:, 1] > 0.8, 1].min()), 3),
               head_front_z_after=round(float(V1[V[:, 1] > 0.8, 2].max()), 3),
               max_vertex_move=round(float(np.linalg.norm(V1 - V, axis=1).max()), 3),
               legs_unmoved=bool(np.abs(V1[V[:, 1] < 0.25][:, :] - V[V[:, 1] < 0.25]).max() < 0.02) if False else None)
    # الساقان والقدمان يجب ألا تتحركا
    low = (V[:, 1] < 0.22) & (np.abs(V[:, 0]) < 0.2)
    rep["feet_max_move"] = round(float(np.linalg.norm(V1[low] - V[low], axis=1).max()), 4)
    print(json.dumps(rep, indent=2, ensure_ascii=False))
    save_obj(lines, vi, ni, V1, N1, a.out, f"Pirate Seated GRIP pose (pitch={a.pitch}, hand=({a.hand_x},{hy:.3f},{a.hand_z})) generated by pose_arms_grip.py")
    if a.preview:
        preview(V, V1, F, N, N1, a.preview, a.deck_y)
        print("preview saved:", a.preview)


if __name__ == "__main__":
    main()
