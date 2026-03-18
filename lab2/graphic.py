import random
import time
import math
import statistics
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import numpy as np

# ───────────────────────────────────────────
#  Алгоритмы (скопированы из основной программы)
# ───────────────────────────────────────────

def counting_sort_desc(indexed_tasks):
    if not indexed_tasks:
        return []
    t_max = max(t for _, (r, t) in indexed_tasks)
    buckets = [[] for _ in range(t_max + 1)]
    for item in indexed_tasks:
        _, (r, t) = item
        buckets[t].append(item)
    result = []
    for i in range(t_max, -1, -1):
        result.extend(buckets[i])
    return result


def nfdh(tasks, n):
    m = len(tasks)
    if m == 0:
        return [], 0, 0.0
    indexed = list(enumerate(tasks))
    sorted_tasks = counting_sort_desc(indexed)
    schedule = [None] * m
    shelves = []   # [start_time, height, next_col]
    cumtime = 0
    for orig_idx, (r, t) in sorted_tasks:
        placed = False
        if shelves:
            shelf = shelves[-1]
            free = n - shelf[2] + 1
            if free >= r:
                schedule[orig_idx] = (shelf[0], list(range(shelf[2], shelf[2] + r)))
                shelf[2] += r
                placed = True
        if not placed:
            shelves.append([cumtime, t, r + 1])
            schedule[orig_idx] = (cumtime, list(range(1, r + 1)))
            cumtime += t
    T_S = cumtime
    T_prime = sum(r * t for r, t in tasks) / n
    eps = (T_S - T_prime) / T_prime if T_prime > 0 else 0.0
    return schedule, T_S, eps


class TournamentTree:
    def __init__(self, capacity):
        self.n_leaves = 1
        while self.n_leaves < capacity:
            self.n_leaves <<= 1
        self.tree = [0] * (2 * self.n_leaves)

    def update(self, idx, value):
        pos = self.n_leaves + idx
        self.tree[pos] = value
        pos >>= 1
        while pos >= 1:
            self.tree[pos] = max(self.tree[2 * pos], self.tree[2 * pos + 1])
            pos >>= 1

    def find_first_fit(self, required, num_active):
        if self.tree[1] < required:
            return -1
        pos = 1
        while pos < self.n_leaves:
            left = 2 * pos
            right = 2 * pos + 1
            pos = left if self.tree[left] >= required else right
        leaf_idx = pos - self.n_leaves
        return leaf_idx if leaf_idx < num_active else -1


def ffdh(tasks, n):
    m = len(tasks)
    if m == 0:
        return [], 0, 0.0
    indexed = list(enumerate(tasks))
    sorted_tasks = counting_sort_desc(indexed)
    schedule = [None] * m
    shelf_data = []
    cumtime = 0
    num_shelves = 0
    tree = TournamentTree(m)
    for orig_idx, (r, t) in sorted_tasks:
        idx = tree.find_first_fit(r, num_shelves)
        if idx == -1:
            shelf_data.append([cumtime, t, r + 1])
            schedule[orig_idx] = (cumtime, list(range(1, r + 1)))
            tree.update(num_shelves, n - r)
            num_shelves += 1
            cumtime += t
        else:
            shelf = shelf_data[idx]
            schedule[orig_idx] = (shelf[0], list(range(shelf[2], shelf[2] + r)))
            shelf[2] += r
            tree.update(idx, n - shelf[2] + 1)
    T_S = cumtime
    T_prime = sum(r * t for r, t in tasks) / n
    eps = (T_S - T_prime) / T_prime if T_prime > 0 else 0.0
    return schedule, T_S, eps


def generate_tasks(m, n, seed=42):
    rng = random.Random(seed)
    return [(rng.randint(1, n), rng.randint(1, 100)) for _ in range(m)]


def run(algo, tasks, n):
    t0 = time.perf_counter()
    if algo == 'NFDH':
        _, T_S, eps = nfdh(tasks, n)
    else:
        _, T_S, eps = ffdh(tasks, n)
    return T_S, eps, time.perf_counter() - t0


# ───────────────────────────────────────────
#  Сбор данных
# ───────────────────────────────────────────

M_VALUES = list(range(500, 5001, 500))   # 500, 1000, ..., 5000

# --- Эксперимент: время (n=1024 и n=4096) ---
data_time = {}
for n in [1024, 4096]:
    t_nfdh, t_ffdh = [], []
    for i, m in enumerate(M_VALUES):
        tasks = generate_tasks(m, n, seed=i)
        _, _, elapsed_n = run('NFDH', tasks, n)
        _, _, elapsed_f = run('FFDH', tasks, n)
        t_nfdh.append(elapsed_n)
        t_ffdh.append(elapsed_f)
    data_time[n] = (t_nfdh, t_ffdh)

# --- Эксперимент: качество (n=1024) ---
N_QUALITY = 1024
eps_nfdh, eps_ffdh, ts_nfdh, ts_ffdh = [], [], [], []
for i, m in enumerate(M_VALUES):
    tasks = generate_tasks(m, N_QUALITY, seed=i)
    T_n, e_n, _ = run('NFDH', tasks, N_QUALITY)
    T_f, e_f, _ = run('FFDH', tasks, N_QUALITY)
    eps_nfdh.append(e_n)
    eps_ffdh.append(e_f)
    ts_nfdh.append(T_n)
    ts_ffdh.append(T_f)


# ───────────────────────────────────────────
#  Цвета и стиль
# ───────────────────────────────────────────

CLR_N = '#E74C3C'   # красный — NFDH
CLR_F = '#2980B9'   # синий   — FFDH
CLR_GRID = '#ECEFF1'

plt.rcParams.update({
    'font.family': 'DejaVu Sans',
    'axes.facecolor': '#FAFAFA',
    'figure.facecolor': 'white',
    'axes.grid': True,
    'grid.color': CLR_GRID,
    'grid.linewidth': 1.2,
    'axes.spines.top': False,
    'axes.spines.right': False,
    'axes.titlesize': 13,
    'axes.labelsize': 11,
    'legend.fontsize': 10,
    'xtick.labelsize': 9,
    'ytick.labelsize': 9,
})


# ───────────────────────────────────────────
#  РИСУНОК 1 — Время выполнения vs m
# ───────────────────────────────────────────

fig1, axes = plt.subplots(1, 2, figsize=(13, 5))
fig1.suptitle('Время выполнения алгоритмов (сек) vs количество задач m',
              fontsize=14, fontweight='bold', y=1.01)

for ax, n in zip(axes, [1024, 4096]):
    t_n, t_f = data_time[n]
    ax.plot(M_VALUES, t_n, 'o-', color=CLR_N, linewidth=2,
            markersize=6, label='NFDH', zorder=3)
    ax.plot(M_VALUES, t_f, 's-', color=CLR_F, linewidth=2,
            markersize=6, label='FFDH', zorder=3)
    ax.fill_between(M_VALUES, t_n, t_f,
                    where=[f > n_ for f, n_ in zip(t_f, t_n)],
                    alpha=0.08, color=CLR_F, label='FFDH медленнее')
    ax.fill_between(M_VALUES, t_n, t_f,
                    where=[n_ > f for f, n_ in zip(t_f, t_n)],
                    alpha=0.08, color=CLR_N, label='NFDH медленнее')
    ax.set_title(f'n = {n} машин')
    ax.set_xlabel('Количество задач m')
    ax.set_ylabel('Время (сек)')
    ax.set_xticks(M_VALUES)
    ax.set_xticklabels([str(m) for m in M_VALUES], rotation=45)
    ax.legend()

plt.tight_layout()
plt.savefig('graph1_time.png', dpi=150, bbox_inches='tight')
plt.show()
print("Сохранён graph1_time.png")


# ───────────────────────────────────────────
#  РИСУНОК 2 — Отклонение ε vs m
# ───────────────────────────────────────────

fig2, ax = plt.subplots(figsize=(10, 5))
fig2.suptitle('Отклонение ε от нижней границы vs количество задач m (n=1024)',
              fontsize=14, fontweight='bold')

ax.plot(M_VALUES, eps_nfdh, 'o-', color=CLR_N, linewidth=2,
        markersize=7, label='NFDH', zorder=3)
ax.plot(M_VALUES, eps_ffdh, 's-', color=CLR_F, linewidth=2,
        markersize=7, label='FFDH', zorder=3)

# Горизонтальные линии средних
mean_n = statistics.mean(eps_nfdh)
mean_f = statistics.mean(eps_ffdh)
ax.axhline(mean_n, color=CLR_N, linestyle='--', linewidth=1.2,
           alpha=0.6, label=f'E[ε] NFDH = {mean_n:.3f}')
ax.axhline(mean_f, color=CLR_F, linestyle='--', linewidth=1.2,
           alpha=0.6, label=f'E[ε] FFDH = {mean_f:.3f}')

# Область между кривыми
ax.fill_between(M_VALUES, eps_ffdh, eps_nfdh,
                alpha=0.08, color='gray', label='Разница')

ax.set_xlabel('Количество задач m')
ax.set_ylabel('ε = (T(S) − T\') / T\'')
ax.set_xticks(M_VALUES)
ax.set_xticklabels([str(m) for m in M_VALUES], rotation=45)
ax.legend()

plt.tight_layout()
plt.savefig('graph2_eps.png', dpi=150, bbox_inches='tight')
plt.show()
print("Сохранён graph2_eps.png")


# ───────────────────────────────────────────
#  РИСУНОК 3 — T(S) vs m
# ───────────────────────────────────────────

fig3, ax = plt.subplots(figsize=(10, 5))
fig3.suptitle('Значение целевой функции T(S) vs количество задач m (n=1024)',
              fontsize=14, fontweight='bold')

ax.plot(M_VALUES, ts_nfdh, 'o-', color=CLR_N, linewidth=2,
        markersize=7, label='NFDH', zorder=3)
ax.plot(M_VALUES, ts_ffdh, 's-', color=CLR_F, linewidth=2,
        markersize=7, label='FFDH', zorder=3)
ax.fill_between(M_VALUES, ts_ffdh, ts_nfdh,
                alpha=0.08, color='gray', label='Выигрыш FFDH')

ax.set_xlabel('Количество задач m')
ax.set_ylabel('T(S)')
ax.set_xticks(M_VALUES)
ax.set_xticklabels([str(m) for m in M_VALUES], rotation=45)
ax.legend()

plt.tight_layout()
plt.savefig('graph3_ts.png', dpi=150, bbox_inches='tight')
plt.show()
print("Сохранён graph3_ts.png")


# ───────────────────────────────────────────
#  РИСУНОК 4 — Итоговая сводная таблица
# ───────────────────────────────────────────

fig4, axes = plt.subplots(2, 2, figsize=(13, 9))
fig4.suptitle('Сводный анализ NFDH vs FFDH', fontsize=15, fontweight='bold')

# — 4.1: Время n=1024 —
ax = axes[0][0]
t_n, t_f = data_time[1024]
ax.plot(M_VALUES, t_n, 'o-', color=CLR_N, linewidth=2, markersize=5, label='NFDH')
ax.plot(M_VALUES, t_f, 's-', color=CLR_F, linewidth=2, markersize=5, label='FFDH')
ax.set_title('Время выполнения, n=1024')
ax.set_xlabel('m'); ax.set_ylabel('Время (сек)')
ax.legend()

# — 4.2: Время n=4096 —
ax = axes[0][1]
t_n, t_f = data_time[4096]
ax.plot(M_VALUES, t_n, 'o-', color=CLR_N, linewidth=2, markersize=5, label='NFDH')
ax.plot(M_VALUES, t_f, 's-', color=CLR_F, linewidth=2, markersize=5, label='FFDH')
ax.set_title('Время выполнения, n=4096')
ax.set_xlabel('m'); ax.set_ylabel('Время (сек)')
ax.legend()

# — 4.3: ε —
ax = axes[1][0]
ax.plot(M_VALUES, eps_nfdh, 'o-', color=CLR_N, linewidth=2, markersize=5, label='NFDH')
ax.plot(M_VALUES, eps_ffdh, 's-', color=CLR_F, linewidth=2, markersize=5, label='FFDH')
ax.axhline(mean_n, color=CLR_N, linestyle='--', linewidth=1, alpha=0.5)
ax.axhline(mean_f, color=CLR_F, linestyle='--', linewidth=1, alpha=0.5)
ax.set_title('Отклонение ε от нижней границы, n=1024')
ax.set_xlabel('m'); ax.set_ylabel('ε')
ax.legend()

# — 4.4: Столбчатая диаграмма E[ε] и σ[ε] —
ax = axes[1][1]
std_n = statistics.stdev(eps_nfdh)
std_f = statistics.stdev(eps_ffdh)
bars = ax.bar(['NFDH', 'FFDH'], [mean_n, mean_f],
              color=[CLR_N, CLR_F], width=0.4,
              yerr=[std_n, std_f], capsize=8,
              error_kw={'linewidth': 2, 'ecolor': 'black'})
# Подписи на столбцах
for bar, val, std in zip(bars, [mean_n, mean_f], [std_n, std_f]):
    ax.text(bar.get_x() + bar.get_width() / 2,
            val + std + 0.003,
            f'E={val:.3f}\nσ={std:.3f}',
            ha='center', va='bottom', fontsize=9)
ax.set_title('E[ε] и σ[ε] (n=1024, все m)')
ax.set_ylabel('ε')
ax.set_ylim(0, max(mean_n, mean_f) * 1.5)

plt.tight_layout()
plt.savefig('graph4_summary.png', dpi=150, bbox_inches='tight')
plt.show()
print("Сохранён graph4_summary.png")
