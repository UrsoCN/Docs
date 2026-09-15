#!/usr/bin/env python3
"""轮足 vs 小车：轮式里程计误差时变曲线 + 轨迹对比图"""
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

plt.rcParams['font.sans-serif'] = ['WenQuanYi Zen Hei', 'WenQuanYi Micro Hei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

L, W = 0.500, 0.480
RC = np.array([[L/2, W/2], [L/2, -W/2], [-L/2, W/2], [-L/2, -W/2]])
DT, STEPS = 0.02, 2500
V_NOM, N_TRIAL = 0.8, 40
T = np.arange(STEPS) * DT

def wheel_from_body(vx, vy, wz):
    v = np.empty(4); d = np.empty(4)
    for i, (x, y) in enumerate(RC):
        Vx = vx - wz*y; Vy = vy + wz*x
        v[i] = np.hypot(Vx, Vy); d[i] = np.arctan2(Vy, Vx)
    return v, d

def inner_to_central(di):
    return np.arctan2(L*np.sin(di), L*np.cos(di) + W*np.sin(di))

def odom_translation(v, d):
    return np.mean(v*np.cos(d)), np.mean(v*np.sin(d)), 0.0

def odom_dual_ackermann(v, d):
    vv = np.mean(v); fm = 0.5*(d[0]+d[1])
    if abs(fm) < 1e-8:
        return vv, 0.0, 0.0
    in_f, in_r = (d[0], d[2]) if fm > 0 else (d[1], d[3])
    sgn = 1.0 if fm > 0 else -1.0
    phi = 0.5*(sgn*inner_to_central(sgn*in_f) + sgn*inner_to_central(sgn*(-in_r)))
    return vv*np.cos(phi), 0.0, 2*vv*np.sin(phi)/L

def traj(t):
    if   t < 10.0: phi = 0.0
    elif t < 30.0: phi = np.radians(10.0)
    elif t < 40.0: phi = 0.0
    else:          phi = np.radians(-10.0)
    return (V_NOM, 0.0) if abs(phi) < 1e-9 else (V_NOM, 2*V_NOM*np.sin(phi)/L)

def run_ts(seed, errs):
    """返回位置误差序列、航向误差序列、真实轨迹、里程计轨迹"""
    rng = np.random.default_rng(seed)
    eps_r = rng.normal(0, 0.005, 4)
    d0 = np.radians(rng.normal(0, 0.2, 4))
    swing_phase = np.array([0.0, np.pi, np.pi, 0.0])   # 对角 trot
    SWING_A = 0.19
    p_t = np.zeros(3); p_o = np.zeros(3)
    pe = np.zeros(STEPS); ye = np.zeros(STEPS)
    tr_t = np.zeros((STEPS, 2)); tr_o = np.zeros((STEPS, 2))
    for k in range(STEPS):
        t = k*DT
        vx_t, wz_t = traj(t)
        v_true, d_true = wheel_from_body(vx_t, 0.0, wz_t)
        horiz = 1.0
        v_enc = v_true*(1+eps_r) + rng.normal(0, 0.005*v_true + 1e-4)
        d_fb = d_true + d0 + np.radians(rng.normal(0, 0.05, 4))
        if errs:
            if 'swing' in errs:
                v_enc = v_enc + SWING_A*np.sin(2*np.pi*2.0*t + swing_phase)
            if 'slip' in errs:
                v_enc = v_enc*(1 + rng.normal(0.10, 0.04, 4))
            if 'airborne' in errs:
                air = (np.sin(2*np.pi*2.0*t + swing_phase) > 0)
                v_enc = np.where(air, 0.0, v_enc)
            if 'pitch' in errs:
                horiz = np.cos(np.radians(8.0)*np.sin(2*np.pi*t/8.0))
        v_enc = np.abs(v_enc)
        if abs(0.5*(d_fb[0]+d_fb[1])) < 1e-8:
            vx_o, vy_o, wz_o = odom_translation(v_enc, d_fb)
        else:
            vx_o, vy_o, wz_o = odom_dual_ackermann(v_enc, d_fb)
        p_t += DT*np.array([vx_t*horiz*np.cos(p_t[2]), vx_t*horiz*np.sin(p_t[2]), wz_t])
        p_o += DT*np.array([vx_o*np.cos(p_o[2]) - vy_o*np.sin(p_o[2]),
                            vx_o*np.sin(p_o[2]) + vy_o*np.cos(p_o[2]), wz_o])
        pe[k] = np.hypot(p_o[0]-p_t[0], p_o[1]-p_t[1])
        dy = np.degrees(p_o[2]-p_t[2]); dy = (dy+180) % 360 - 180
        ye[k] = abs(dy)
        tr_t[k] = p_t[:2]; tr_o[k] = p_o[:2]
    return pe, ye, tr_t, tr_o

CASES = [
    ("小车 · 精标定（基准）", set(),            "#2b8a3e", "-"),
    ("轮足 · 仅打滑 10%",   {'slip'},           "#e8590c", "--"),
    ("轮足 · 仅离地空转",   {'airborne'},       "#c92a2a", "-."),
    ("轮足 · 全部叠加",     {'slip','airborne','pitch','swing'}, "#5f3dc4", "-"),
]

res = {}
for name, errs, c, ls in CASES:
    P = np.zeros((N_TRIAL, STEPS)); Y = np.zeros((N_TRIAL, STEPS))
    tr_t = tr_o = None
    for s in range(N_TRIAL):
        pe, ye, a, b = run_ts(s, errs)
        P[s], Y[s] = pe, ye
        if s == 0:
            tr_t, tr_o = a, b
    res[name] = (P, Y, tr_t, tr_o)
    print(f"{name:<22} 终点位置误差中位 {np.median(P[:,-1]):5.2f} m | 航向 {np.median(Y[:,-1]):6.1f}°")

fig = plt.figure(figsize=(15, 11))
gs = fig.add_gridspec(2, 2, height_ratios=[1, 1.15], hspace=0.28, wspace=0.22)

ax1 = fig.add_subplot(gs[0, 0])
ax2 = fig.add_subplot(gs[0, 1])

for name, errs, c, ls in CASES:
    P, Y, _, _ = res[name]
    m = np.median(P, axis=0); lo = np.percentile(P, 25, axis=0); hi = np.percentile(P, 75, axis=0)
    ax1.plot(T, m, color=c, ls=ls, lw=2.2, label=name)
    ax1.fill_between(T, lo, hi, color=c, alpha=0.13)
    m2 = np.median(Y, axis=0); l2 = np.percentile(Y, 25, axis=0); h2 = np.percentile(Y, 75, axis=0)
    ax2.plot(T, m2, color=c, ls=ls, lw=2.2, label=name)
    ax2.fill_between(T, l2, h2, color=c, alpha=0.13)

for ax, ttl, yl in [(ax1, "位置误差随时间累积（中位数 + 25/75 分位带）", "位置误差 (m)"),
                    (ax2, "航向误差随时间累积（中位数 + 25/75 分位带）", "航向误差 (°)")]:
    ax.set_title(ttl, fontsize=13, pad=10)
    ax.set_xlabel("时间 (s)   [对应行驶距离 ≈ 0.8 m/s × t]", fontsize=10)
    ax.set_ylabel(yl, fontsize=11)
    ax.grid(alpha=0.3, ls=":")
    ax.legend(fontsize=9, loc="upper left")

# 阶段标注
for ax in (ax1, ax2):
    for xv, lab in [(10, "起转"), (30, "回直"), (40, "反向右转")]:
        ax.axvline(xv, color="gray", ls=":", lw=1, alpha=0.7)
        ax.text(xv, ax.get_ylim()[1]*0.02, lab, fontsize=8, color="gray", rotation=90, va="bottom")

ax3 = fig.add_subplot(gs[1, :])
for name, errs, c, ls in CASES:
    _, _, tr_t, tr_o = res[name]
    ax3.plot(tr_o[:, 0], tr_o[:, 1], color=c, ls=ls, lw=2.0, label=f"{name}（里程计）")
ax3.plot(res[CASES[0][0]][2][:, 0], res[CASES[0][0]][2][:, 1],
         color="black", lw=3.0, alpha=0.55, label="真实轨迹（真值）")
ax3.plot(0, 0, "o", color="black", ms=9)
ax3.text(0.15, 0.15, "起点", fontsize=10)
ax3.set_title("轨迹对比：里程计输出 vs 真实轨迹（单次典型运行）", fontsize=13, pad=10)
ax3.set_xlabel("x (m)"); ax3.set_ylabel("y (m)")
ax3.set_aspect("equal"); ax3.grid(alpha=0.3, ls=":")
ax3.legend(fontsize=9, loc="best")

fig.suptitle("轮足机器狗 vs 轮式小车：轮式里程计（纯航迹推算）漂移对比\n"
             "40 m 轨迹 / 120 次蒙特卡洛 / 精标定基准（轮径0.5%、转向零位0.2°）",
             fontsize=14, y=0.985)
fig.savefig("/tmp/legged_odom_analysis.png", dpi=130, bbox_inches="tight", facecolor="white")
print("\n已保存: /tmp/legged_odom_analysis.png")
