import argparse
import subprocess
import sys
import os
import csv
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np


def run_benchmark(bench: int, c1: float, c2: float, c3: float,
                  eps: float, exe: str = "./lab3",
                  out_csv: str = "time_data.csv") -> str:
    """Запускает C++-программу в режиме бенчмарка и возвращает путь к CSV."""
    cmd = [exe,
           "--bench", str(bench),
           "--c1", str(c1), "--c2", str(c2), "--c3", str(c3),
           "--eps", str(eps)]
    print(f"Запуск: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=False)
    if result.returncode != 0:
        sys.exit(f"Ошибка при запуске {exe}: код возврата {result.returncode}")
    return out_csv

def load_csv(path: str):
    """Читает CSV с колонками n,time_sec → два списка."""
    ns, times = [], []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            ns.append(int(row["n"]))
            times.append(float(row["time_sec"]))
    return ns, times

def plot(ns, times, c1, c2, c3, eps, save_path: str = "time_vs_n.png"):
    fig, ax = plt.subplots(figsize=(10, 5))

    ax.plot(ns, times, color="steelblue", linewidth=1.8,
            marker="o", markersize=5, zorder=3, label="Время (с)")

    if len(times) >= 5:
        window = 3
        kernel = np.ones(window) / window
        trend  = np.convolve(times, kernel, mode="same")
        ax.plot(ns[1:-1], trend[1:-1],
                color="tomato", linewidth=1.4, linestyle="--",
                zorder=2, label=f"Скользящее среднее (окно={window})")

    ax.set_xlabel("Количество элементарных машин  n", fontsize=13)
    ax.set_ylabel("Время работы алгоритма, с", fontsize=13)
    ax.set_title(
        f"Зависимость времени работы метода Брауна от n\n"
        f"(c₁={c1}, c₂={c2}, c₃={c3}, ε={eps})",
        fontsize=13
    )
    ax.xaxis.set_major_locator(ticker.MaxNLocator(integer=True))
    ax.grid(True, linestyle="--", alpha=0.55)
    ax.legend(fontsize=11)
    fig.tight_layout()
    fig.savefig(save_path, dpi=150)
    print(f"График сохранён: {save_path}")
    plt.close(fig)

def main():
    parser = argparse.ArgumentParser(description="Построение графика времени")
    parser.add_argument("--csv",   default="time_data.csv", help="Путь к CSV-файлу")
    parser.add_argument("--out",   default="time_vs_n.png",  help="Путь к выходному PNG")
    parser.add_argument("--run",   action="store_true",      help="Запустить ./lab3 --bench ...")
    parser.add_argument("--bench", type=int,   default=30,   help="Максимальное n для бенчмарка")
    parser.add_argument("--c1",    type=float, default=1.0)
    parser.add_argument("--c2",    type=float, default=2.0)
    parser.add_argument("--c3",    type=float, default=3.0)
    parser.add_argument("--eps",   type=float, default=0.01)
    parser.add_argument("--exe",   default="./lab3",         help="Путь к исполняемому файлу")
    args = parser.parse_args()

    csv_path = args.csv

    if args.run:
        csv_path = run_benchmark(
            args.bench, args.c1, args.c2, args.c3, args.eps,
            exe=args.exe, out_csv=args.csv
        )

    if not os.path.exists(csv_path):
        sys.exit(f"Файл не найден: {csv_path}\n"
                 "Запустите сначала: ./lab3 --bench 30  (или используйте --run)")

    ns, times = load_csv(csv_path)
    plot(ns, times, args.c1, args.c2, args.c3, args.eps, save_path=args.out)


if __name__ == "__main__":
    main()
