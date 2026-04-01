import matplotlib.pyplot as plt
import numpy as np
import os
import re

output_dir = "graphs"
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

m_styles = {
    1: {'marker': 'o', 'linestyle': '-', 'linewidth': 2, 'markersize': 7},
    2: {'marker': 's', 'linestyle': '--', 'linewidth': 2, 'markersize': 7},
    3: {'marker': '^', 'linestyle': '-.', 'linewidth': 2, 'markersize': 7}
}

lambda_colors = {
    1e-7: 'blue',
    1e-6: 'green',
    1e-5: 'red'
}

mu_colors = {
    1: 'blue',
    10: 'green',
    100: 'orange',
    1000: 'red'
}

mu_linestyles = {
    1: '-',
    10: '--',
    100: '-.',
    1000: ':'
}

def load_results(filename):
    """Загружает результаты из output.txt"""
    results = []
    try:
        with open(filename, 'r') as f:
            content = f.read()
    except FileNotFoundError:
        print(f"Файл {filename} не найден")
        return []
    
    blocks = content.split('------------------------')
    for block in blocks:
        if not block.strip():
            continue
        lines = block.strip().split('\n')
        if len(lines) < 3:
            continue
        
        first_line = lines[0].strip()
        if not first_line.startswith('λ='):
            continue
        
        parts = first_line.split()
        lam = float(parts[0].split('=')[1])
        mu = float(parts[1].split('=')[1])
        m = int(parts[2].split('=')[1])
        n = int(parts[3].split('=')[1])
        
        theta_line = lines[1].strip()
        theta_N = float(theta_line.split('=')[1].strip())
        
        T_line = lines[2].strip()
        T_str = T_line.split('=')[1].strip()
        try:
            T_0 = float(T_str)
        except:
            T_0 = float('inf')
        
        results.append((lam, mu, m, n, theta_N, T_0))
    
    return results

def plot_theta_combined(results):
    lambda_values = sorted(set(r[0] for r in results))
    mu_values = sorted(set(r[1] for r in results))
    m_values = sorted(set(r[2] for r in results))
    
    fig, ax = plt.subplots(figsize=(14, 8))
    fig.suptitle('Среднее время безотказной работы θ_N', fontsize=16, fontweight='bold')
    
    for lam in lambda_values:
        for mu in mu_values:
            for m in m_values:
                data = [r for r in results if r[0] == lam and r[1] == mu and r[2] == m]
                if not data:
                    continue
                
                data.sort(key=lambda x: x[3])
                n_vals = [r[3] for r in data]
                theta_vals = [r[4] for r in data]
                
                label = f'λ={lam:.0e}, μ={mu}, m={m}'
                
                ax.plot(n_vals, theta_vals,
                       color=lambda_colors[lam],
                       marker=m_styles[m]['marker'],
                       linestyle=mu_linestyles[mu],
                       linewidth=2,
                       markersize=6,
                       label=label, alpha=0.8)
    
    ax.set_xlabel('n (минимальное число машин)', fontsize=12)
    ax.set_ylabel('θ_N (часы)', fontsize=12)
    ax.legend(loc='center left', bbox_to_anchor=(1, 0.5), fontsize=9)
    ax.grid(True, alpha=0.3)
    ax.set_yscale('log')
    ax.set_xlim(65526, 65537)
    ax.set_xticks(range(65527, 65537, 2))
    ax.tick_params(axis='x', rotation=45)
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/theta_combined.png", dpi=300, bbox_inches='tight')
    plt.close()
    print("Сохранен: theta_combined.png")

def plot_theta_by_lambda(results):
    lambda_values = sorted(set(r[0] for r in results))
    mu_values = sorted(set(r[1] for r in results))
    m_values = sorted(set(r[2] for r in results))
    
    fig, axes = plt.subplots(1, 3, figsize=(18, 6))
    fig.suptitle('Среднее время безотказной работы θ_N (группировка по λ)', fontsize=16, fontweight='bold')
    
    for idx, lam in enumerate(lambda_values):
        ax = axes[idx]
        
        for mu in mu_values:
            for m in m_values:
                data = [r for r in results if r[0] == lam and r[1] == mu and r[2] == m]
                if not data:
                    continue
                
                data.sort(key=lambda x: x[3])
                n_vals = [r[3] for r in data]
                theta_vals = [r[4] for r in data]
                
                label = f'μ={mu}, m={m}'
                
                ax.plot(n_vals, theta_vals,
                       color=mu_colors[mu],
                       marker=m_styles[m]['marker'],
                       linestyle=m_styles[m]['linestyle'],
                       linewidth=2,
                       markersize=6,
                       label=label, alpha=0.8)
        
        ax.set_xlabel('n (минимальное число машин)', fontsize=12)
        ax.set_ylabel('θ_N (часы)', fontsize=12)
        ax.set_title(f'λ = {lam:.0e} 1/ч', fontsize=14, fontweight='bold')
        ax.legend(loc='best', fontsize=9, ncol=2)
        ax.grid(True, alpha=0.3)
        ax.set_yscale('log')
        ax.set_xlim(65526, 65537)
        ax.set_xticks(range(65527, 65537, 2))
        ax.tick_params(axis='x', rotation=45)
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/theta_by_lambda.png", dpi=300, bbox_inches='tight')
    plt.close()
    print("Сохранен: theta_by_lambda.png")

def plot_T_combined(results):
    lambda_values = sorted(set(r[0] for r in results))
    mu_values = sorted(set(r[1] for r in results))
    m_values = sorted(set(r[2] for r in results))
    
    fig, ax = plt.subplots(figsize=(14, 8))
    fig.suptitle('Среднее время восстановления T₀', fontsize=16, fontweight='bold')
    
    for lam in lambda_values:
        for mu in mu_values:
            for m in m_values:
                data = [r for r in results if r[0] == lam and r[1] == mu and r[2] == m]
                if not data:
                    continue
                
                data.sort(key=lambda x: x[3])
                n_vals = [r[3] for r in data]
                T_vals = [r[5] for r in data]
                
                T_plot = [1e10 if np.isinf(v) or v > 1e10 else v for v in T_vals]
                
                label = f'λ={lam:.0e}, μ={mu}, m={m}'
                
                ax.plot(n_vals, T_plot,
                       color=lambda_colors[lam],
                       marker=m_styles[m]['marker'],
                       linestyle=mu_linestyles[mu],
                       linewidth=2,
                       markersize=6,
                       label=label, alpha=0.8)
    
    ax.set_xlabel('n (минимальное число машин)', fontsize=12)
    ax.set_ylabel('T₀ (часы)', fontsize=12)
    ax.legend(loc='center left', bbox_to_anchor=(1, 0.5), fontsize=9)
    ax.grid(True, alpha=0.3)
    ax.set_yscale('log')
    ax.set_xlim(65526, 65537)
    ax.set_xticks(range(65527, 65537, 2))
    ax.tick_params(axis='x', rotation=45)
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/T_combined.png", dpi=300, bbox_inches='tight')
    plt.close()
    print("Сохранен: T_combined.png (теперь T₀ зависит от λ!)")

def plot_T_by_lambda(results):
    lambda_values = sorted(set(r[0] for r in results))
    mu_values = sorted(set(r[1] for r in results))
    m_values = sorted(set(r[2] for r in results))
    
    fig, axes = plt.subplots(1, 3, figsize=(18, 6))
    fig.suptitle('Среднее время восстановления T₀ (группировка по λ)', fontsize=16, fontweight='bold')
    
    for idx, lam in enumerate(lambda_values):
        ax = axes[idx]
        
        for mu in mu_values:
            for m in m_values:
                data = [r for r in results if r[0] == lam and r[1] == mu and r[2] == m]
                if not data:
                    continue
                
                data.sort(key=lambda x: x[3])
                n_vals = [r[3] for r in data]
                T_vals = [r[5] for r in data]
                T_plot = [1e10 if np.isinf(v) or v > 1e10 else v for v in T_vals]
                
                label = f'μ={mu}, m={m}'
                
                ax.plot(n_vals, T_plot,
                       color=mu_colors[mu],
                       marker=m_styles[m]['marker'],
                       linestyle=m_styles[m]['linestyle'],
                       linewidth=2,
                       markersize=6,
                       label=label, alpha=0.8)
        
        ax.set_xlabel('n (минимальное число машин)', fontsize=12)
        ax.set_ylabel('T₀ (часы)', fontsize=12)
        ax.set_title(f'λ = {lam:.0e} 1/ч', fontsize=14, fontweight='bold')
        ax.legend(loc='best', fontsize=9, ncol=2)
        ax.grid(True, alpha=0.3)
        ax.set_yscale('log')
        ax.set_xlim(65526, 65537)
        ax.set_xticks(range(65527, 65537, 2))
        ax.tick_params(axis='x', rotation=45)
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/T_by_lambda.png", dpi=300, bbox_inches='tight')
    plt.close()
    print("Сохранен: T_by_lambda.png")

if __name__ == "__main__":
    results = load_results('output.txt')
    
    if not results:
        print("Нет данных. Сначала запустите C++ программу: ./lab3_fixed > output.txt")
    else:
        print(f"Загружено {len(results)} записей")
        
        plot_theta_combined(results)
        plot_theta_by_lambda(results)
        plot_T_combined(results)
        plot_T_by_lambda(results)
        
        print(f"\nВсе графики сохранены в папку '{output_dir}/'")
        print("  - theta_combined.png    (один график, все λ, μ, m)")
        print("  - theta_by_lambda.png   (3 подграфика по λ)")
        print("  - T_combined.png        (один график, T₀ зависит от λ!)")
        print("  - T_by_lambda.png       (3 подграфика по λ)")
