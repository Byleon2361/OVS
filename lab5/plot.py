import numpy as np
import os
os.makedirs('./graphs', exist_ok=True)
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from matplotlib.ticker import LogLocator, LogFormatter

# ─── Параметры ────────────────────────────────────────────────────────────────
N = 65536
LAMBDA_VALS = [1e-7, 1e-6, 1e-5]
MU_VALS     = [1.0, 10.0, 100.0, 1000.0]
M_VALS      = [1, 2, 3]
N_RANGE     = list(range(65527, 65537))   # 65527 … 65536

# ─── Формулы ──────────────────────────────────────────────────────────────────
def mu_l(l, N, m, mu):
    """Интенсивность восстановления при l работоспособных ЭМ."""
    if l >= N - m:
        return (N - l) * mu
    return m * mu

def compute_theta(N, n_min, lam, m, mu):
    """Обратная рекуррентность: θ_N = 1/(Nλ), θ_k = 1/(kλ) + μ_k/(kλ)·θ_{k+1}"""
    theta = np.zeros(N + 1)
    theta[N] = 1.0 / (N * lam)
    for k in range(N - 1, n_min - 1, -1):
        mk = mu_l(k, N, m, mu)
        theta[k] = 1.0 / (k * lam) + (mk / (k * lam)) * theta[k + 1]
    return theta  # индексация: theta[k] = θ_k

def compute_T(N, lam, m, mu):
    """Прямая рекуррентность: T_1 = 1/μ_0, T_{k+1} = 1/μ_k + kλ/μ_k·T_k"""
    T = np.zeros(N + 1)
    mu0 = mu_l(0, N, m, mu)
    T[1] = 1.0 / mu0 if mu0 > 0 else 0.0
    for k in range(1, N):
        mk = mu_l(k, N, m, mu)
        if mk > 0:
            T[k + 1] = 1.0 / mk + (k * lam / mk) * T[k]
    return T  # индексация: T[k] = T_k

# ─── Стиль ────────────────────────────────────────────────────────────────────
plt.rcParams.update({
    'font.family': 'DejaVu Serif',
    'axes.spines.top': False,
    'axes.spines.right': False,
    'axes.grid': True,
    'grid.alpha': 0.25,
    'grid.linestyle': '--',
    'figure.facecolor': '#0f1117',
    'axes.facecolor': '#181c27',
    'axes.labelcolor': '#c8ccd8',
    'xtick.color': '#8a8fa8',
    'ytick.color': '#8a8fa8',
    'axes.titlecolor': '#e8ecf5',
    'legend.facecolor': '#1e2235',
    'legend.edgecolor': '#3a3f55',
    'legend.labelcolor': '#c8ccd8',
    'text.color': '#e8ecf5',
    'axes.edgecolor': '#2e3347',
    'grid.color': '#2e3347',
})

PALETTE_THETA = ['#4cc9f0', '#f72585', '#7209b7', '#3a0ca3',
                 '#4361ee', '#4cc9f0', '#fb8500', '#8ecae6']
PALETTE_T     = ['#f4a261', '#e76f51', '#2a9d8f', '#264653',
                 '#e9c46a', '#f4a261', '#80b918', '#bc4749']

def make_fig(title):
    fig = plt.figure(figsize=(18, 13), facecolor='#0f1117')
    fig.suptitle(title, fontsize=15, fontweight='bold',
                 color='#e8ecf5', y=0.98)
    return fig

# ═══════════════════════════════════════════════════════════════════════════════
# БЛОК 1 — зависимость от n  (фикс: λ=1e-6, μ=10, m=2)
# ═══════════════════════════════════════════════════════════════════════════════
fig1 = make_fig("Зависимость θ и T от n  (λ=10⁻⁶, μ=10, m=2)")
gs   = gridspec.GridSpec(1, 2, figure=fig1, wspace=0.35)

ax_th = fig1.add_subplot(gs[0])
ax_T  = fig1.add_subplot(gs[1])

lam_fix, mu_fix, m_fix = 1e-6, 10.0, 2
theta_arr = compute_theta(N, min(N_RANGE), lam_fix, m_fix, mu_fix)
T_arr     = compute_T(N, lam_fix, m_fix, mu_fix)

# Для каждого n рисуем точку (n, θ_n) и (n, T_n)
ns       = np.array(N_RANGE)
theta_n  = np.array([theta_arr[n] for n in ns])
T_n      = np.array([T_arr[n]     for n in ns])

ax_th.semilogy(ns, theta_n, 'o-', color='#4cc9f0', lw=2, ms=6, label='θ_n')
ax_th.set_xlabel('n', fontsize=11)
ax_th.set_ylabel('θ_n, ч', fontsize=11)
ax_th.set_title('Среднее время безотказной работы θ_n', fontsize=11)
ax_th.legend()

ax_T.plot(ns, T_n, 's-', color='#f4a261', lw=2, ms=6, label='T_n')
ax_T.set_xlabel('n', fontsize=11)
ax_T.set_ylabel('T_n, ч', fontsize=11)
ax_T.set_title('Среднее время восстановления T_n', fontsize=11)
ax_T.legend()

plt.savefig('./graphs/graph1_vs_n.png', dpi=150,
            bbox_inches='tight', facecolor='#0f1117')
plt.close()

# ═══════════════════════════════════════════════════════════════════════════════
# БЛОК 2 — зависимость от λ  (фикс: μ=10, m=2, n=65530)
# ═══════════════════════════════════════════════════════════════════════════════
fig2 = make_fig("Зависимость θ и T от λ  (μ=10, m=2, n=65530)")
gs2  = gridspec.GridSpec(1, 2, figure=fig2, wspace=0.35)
ax_th2 = fig2.add_subplot(gs2[0])
ax_T2  = fig2.add_subplot(gs2[1])

n_fix2, mu_fix2, m_fix2 = 65530, 10.0, 2
theta_by_lam = []
T_by_lam     = []
for lam in LAMBDA_VALS:
    th = compute_theta(N, n_fix2, lam, m_fix2, mu_fix2)
    Tv = compute_T(N, lam, m_fix2, mu_fix2)
    theta_by_lam.append(th[n_fix2])
    T_by_lam.append(Tv[n_fix2])

lam_labels = [r'$10^{-7}$', r'$10^{-6}$', r'$10^{-5}$']
x = np.arange(len(LAMBDA_VALS))
ax_th2.bar(x, theta_by_lam, color=['#4cc9f0','#4361ee','#7209b7'], width=0.5)
ax_th2.set_xticks(x); ax_th2.set_xticklabels(lam_labels, fontsize=12)
ax_th2.set_yscale('log')
ax_th2.set_xlabel('λ, 1/ч', fontsize=11)
ax_th2.set_ylabel('θ_n, ч', fontsize=11)
ax_th2.set_title('θ_n при разных λ', fontsize=11)

ax_T2.bar(x, T_by_lam, color=['#f4a261','#e76f51','#2a9d8f'], width=0.5)
ax_T2.set_xticks(x); ax_T2.set_xticklabels(lam_labels, fontsize=12)
ax_T2.set_xlabel('λ, 1/ч', fontsize=11)
ax_T2.set_ylabel('T_n, ч', fontsize=11)
ax_T2.set_title('T_n при разных λ', fontsize=11)

plt.savefig('./graphs/graph2_vs_lambda.png', dpi=150,
            bbox_inches='tight', facecolor='#0f1117')
plt.close()

# ═══════════════════════════════════════════════════════════════════════════════
# БЛОК 3 — зависимость от μ  (фикс: λ=1e-6, m=2, n=65530)
# ═══════════════════════════════════════════════════════════════════════════════
fig3 = make_fig("Зависимость θ и T от μ  (λ=10⁻⁶, m=2, n=65530)")
gs3  = gridspec.GridSpec(1, 2, figure=fig3, wspace=0.35)
ax_th3 = fig3.add_subplot(gs3[0])
ax_T3  = fig3.add_subplot(gs3[1])

lam_fix3, n_fix3, m_fix3 = 1e-6, 65530, 2
theta_by_mu = []
T_by_mu     = []
for mu_v in MU_VALS:
    th = compute_theta(N, n_fix3, lam_fix3, m_fix3, mu_v)
    Tv = compute_T(N, lam_fix3, m_fix3, mu_v)
    theta_by_mu.append(th[n_fix3])
    T_by_mu.append(Tv[n_fix3])

mu_labels = ['1', '10', '100', '1000']
x3 = np.arange(len(MU_VALS))
ax_th3.plot(x3, theta_by_mu, 'o-', color='#4cc9f0', lw=2, ms=8)
ax_th3.set_xticks(x3); ax_th3.set_xticklabels(mu_labels)
ax_th3.set_yscale('log')
ax_th3.set_xlabel('μ, 1/ч', fontsize=11)
ax_th3.set_ylabel('θ_n, ч', fontsize=11)
ax_th3.set_title('θ_n при разных μ', fontsize=11)

ax_T3.plot(x3, T_by_mu, 's-', color='#f4a261', lw=2, ms=8)
ax_T3.set_xticks(x3); ax_T3.set_xticklabels(mu_labels)
ax_T3.set_yscale('log')
ax_T3.set_xlabel('μ, 1/ч', fontsize=11)
ax_T3.set_ylabel('T_n, ч', fontsize=11)
ax_T3.set_title('T_n при разных μ', fontsize=11)

plt.savefig('./graphs/graph3_vs_mu.png', dpi=150,
            bbox_inches='tight', facecolor='#0f1117')
plt.close()

# ═══════════════════════════════════════════════════════════════════════════════
# БЛОК 4 — зависимость от m  (фикс: λ=1e-6, μ=10, n=65530)
# ═══════════════════════════════════════════════════════════════════════════════
fig4 = make_fig("Зависимость θ и T от m  (λ=10⁻⁶, μ=10, n=65530)")
gs4  = gridspec.GridSpec(1, 2, figure=fig4, wspace=0.35)
ax_th4 = fig4.add_subplot(gs4[0])
ax_T4  = fig4.add_subplot(gs4[1])

lam_fix4, mu_fix4, n_fix4 = 1e-6, 10.0, 65530
theta_by_m = []
T_by_m     = []
for m_v in M_VALS:
    th = compute_theta(N, n_fix4, lam_fix4, m_v, mu_fix4)
    Tv = compute_T(N, lam_fix4, m_v, mu_fix4)
    theta_by_m.append(th[n_fix4])
    T_by_m.append(Tv[n_fix4])

x4 = np.arange(len(M_VALS))
m_labels = ['1', '2', '3']
bars_th = ax_th4.bar(x4, theta_by_m,
                     color=['#7209b7','#4361ee','#4cc9f0'], width=0.5)
ax_th4.set_xticks(x4); ax_th4.set_xticklabels(m_labels)
ax_th4.set_yscale('log')
ax_th4.set_xlabel('m (кол-во восст. устройств)', fontsize=11)
ax_th4.set_ylabel('θ_n, ч', fontsize=11)
ax_th4.set_title('θ_n при разных m', fontsize=11)
for bar, val in zip(bars_th, theta_by_m):
    ax_th4.text(bar.get_x() + bar.get_width()/2,
                bar.get_height() * 1.2,
                f'{val:.2e}', ha='center', va='bottom', fontsize=8,
                color='#c8ccd8')

bars_T = ax_T4.bar(x4, T_by_m,
                   color=['#e76f51','#f4a261','#2a9d8f'], width=0.5)
ax_T4.set_xticks(x4); ax_T4.set_xticklabels(m_labels)
ax_T4.set_xlabel('m (кол-во восст. устройств)', fontsize=11)
ax_T4.set_ylabel('T_n, ч', fontsize=11)
ax_T4.set_title('T_n при разных m', fontsize=11)
for bar, val in zip(bars_T, T_by_m):
    ax_T4.text(bar.get_x() + bar.get_width()/2,
                bar.get_height() + ax_T4.get_ylim()[1]*0.005,
                f'{val:.4f}', ha='center', va='bottom', fontsize=8,
                color='#c8ccd8')

plt.savefig('./graphs/graph4_vs_m.png', dpi=150,
            bbox_inches='tight', facecolor='#0f1117')
plt.close()

# ═══════════════════════════════════════════════════════════════════════════════
# БЛОК 5 — полные векторы θ_k и T_k  при n=65527, λ=1e-6, μ=10, m=2
#           (все элементы от n до N)
# ═══════════════════════════════════════════════════════════════════════════════
fig5 = make_fig("Полные векторы θ и T по k  (n=65527, λ=10⁻⁶, μ=10, m=2)")
gs5  = gridspec.GridSpec(1, 2, figure=fig5, wspace=0.35)
ax_th5 = fig5.add_subplot(gs5[0])
ax_T5  = fig5.add_subplot(gs5[1])

lam5, mu5, m5, n5 = 1e-6, 10.0, 2, 65527
th5 = compute_theta(N, n5, lam5, m5, mu5)
T5  = compute_T(N, lam5, m5, mu5)
ks  = np.arange(n5, N + 1)

ax_th5.semilogy(ks, [th5[k] for k in ks], color='#4cc9f0', lw=2)
ax_th5.set_xlabel('k', fontsize=11)
ax_th5.set_ylabel('θ_k, ч', fontsize=11)
ax_th5.set_title('Вектор θ = {θ_n, …, θ_N}', fontsize=11)
ax_th5.fill_between(ks, [th5[k] for k in ks], alpha=0.15, color='#4cc9f0')

ax_T5.plot(ks, [T5[k] for k in ks], color='#f4a261', lw=2)
ax_T5.set_xlabel('k', fontsize=11)
ax_T5.set_ylabel('T_k, ч', fontsize=11)
ax_T5.set_title('Вектор T = {T_n, …, T_N}', fontsize=11)
ax_T5.fill_between(ks, [T5[k] for k in ks], alpha=0.15, color='#f4a261')

plt.savefig('./graphs/graph5_full_vectors.png', dpi=150,
            bbox_inches='tight', facecolor='#0f1117')
plt.close()

# ═══════════════════════════════════════════════════════════════════════════════
# БЛОК 6 — сводный: θ_n при разных λ и m, по оси x = n  (μ=10 фикс)
# ═══════════════════════════════════════════════════════════════════════════════
fig6 = make_fig("Сводный: θ_n(n) при разных λ и m  (μ=10)")
gs6  = gridspec.GridSpec(1, 2, figure=fig6, wspace=0.4)
ax6a = fig6.add_subplot(gs6[0])
ax6b = fig6.add_subplot(gs6[1])

colors_lam = ['#4cc9f0', '#f72585', '#ffd166']
for i, lam in enumerate(LAMBDA_VALS):
    th = compute_theta(N, min(N_RANGE), lam, 2, 10.0)
    vals = [th[n] for n in N_RANGE]
    ax6a.semilogy(N_RANGE, vals, 'o-', color=colors_lam[i], lw=2, ms=5,
                  label=f'λ=10^{int(np.log10(lam))}')
ax6a.set_xlabel('n', fontsize=11)
ax6a.set_ylabel('θ_n, ч', fontsize=11)
ax6a.set_title('θ_n(n) при разных λ  (m=2, μ=10)', fontsize=11)
ax6a.legend(fontsize=9)

colors_m = ['#7209b7', '#4cc9f0', '#f72585']
for i, m_v in enumerate(M_VALS):
    th = compute_theta(N, min(N_RANGE), 1e-6, m_v, 10.0)
    vals = [th[n] for n in N_RANGE]
    ax6b.semilogy(N_RANGE, vals, 's-', color=colors_m[i], lw=2, ms=5,
                  label=f'm={m_v}')
ax6b.set_xlabel('n', fontsize=11)
ax6b.set_ylabel('θ_n, ч', fontsize=11)
ax6b.set_title('θ_n(n) при разных m  (λ=10⁻⁶, μ=10)', fontsize=11)
ax6b.legend(fontsize=9)

plt.savefig('./graphs/graph6_summary_theta.png', dpi=150,
            bbox_inches='tight', facecolor='#0f1117')
plt.close()

# ═══════════════════════════════════════════════════════════════════════════════
# БЛОК 7 — сводный: T_n при разных μ и m, по оси x = n  (λ=1e-6 фикс)
# ═══════════════════════════════════════════════════════════════════════════════
fig7 = make_fig("Сводный: T_n(n) при разных μ и m  (λ=10⁻⁶)")
gs7  = gridspec.GridSpec(1, 2, figure=fig7, wspace=0.4)
ax7a = fig7.add_subplot(gs7[0])
ax7b = fig7.add_subplot(gs7[1])

colors_mu = ['#4cc9f0', '#f72585', '#ffd166', '#80b918']
for i, mu_v in enumerate(MU_VALS):
    Tv = compute_T(N, 1e-6, 2, mu_v)
    vals = [Tv[n] for n in N_RANGE]
    ax7a.semilogy(N_RANGE, vals, 'o-', color=colors_mu[i], lw=2, ms=5,
                  label=f'μ={int(mu_v)}')
ax7a.set_xlabel('n', fontsize=11)
ax7a.set_ylabel('T_n, ч', fontsize=11)
ax7a.set_title('T_n(n) при разных μ  (m=2, λ=10⁻⁶)', fontsize=11)
ax7a.legend(fontsize=9)

for i, m_v in enumerate(M_VALS):
    Tv = compute_T(N, 1e-6, m_v, 10.0)
    vals = [Tv[n] for n in N_RANGE]
    ax7b.plot(N_RANGE, vals, 's-', color=colors_m[i], lw=2, ms=5,
              label=f'm={m_v}')
ax7b.set_xlabel('n', fontsize=11)
ax7b.set_ylabel('T_n, ч', fontsize=11)
ax7b.set_title('T_n(n) при разных m  (λ=10⁻⁶, μ=10)', fontsize=11)
ax7b.legend(fontsize=9)

plt.savefig('./graphs/graph7_summary_T.png', dpi=150,
            bbox_inches='tight', facecolor='#0f1117')
plt.close()

print("Все графики сохранены в /mnt/user-data/outputs/")
