#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np

files_config = [
    {'file': 'output_sm_pine.txt', 'label': 'RAM (Pine)', 'color': 'red', 'marker': 'o'},
    {'file': 'output_sm_oak.txt', 'label': 'RAM (Oak)', 'color': 'lightcoral', 'marker': 's'},
    
    {'file': 'output_qpi_pine.txt', 'label': 'QPI (Pine)', 'color': 'green', 'marker': '^'},
    {'file': 'output_qpi_oak.txt', 'label': 'QPI (Oak)', 'color': 'limegreen', 'marker': 'v'},
    
    {'file': 'output_net_pine.txt', 'label': 'Ethernet (Pine)', 'color': 'blue', 'marker': 'D'},
    {'file': 'output_net_oak.txt', 'label': 'Ethernet (Oak)', 'color': 'purple', 'marker': '*'}
]

plt.figure(figsize=(14, 8))

for config in files_config:
    try:
        data = np.loadtxt(config['file'], comments='#')
        sizes = data[:, 0]
        times = data[:, 1] * 1e6
        
        plt.loglog(sizes, times, 
                  color=config['color'],
                  marker=config['marker'],
                  markersize=8,
                  markevery=max(1, len(sizes)//8),
                  linewidth=2,
                  label=config['label'])
        
        print(f"Загружен {config['file']}: {len(sizes)} точек -> {config['label']}")
        
    except Exception as e:
        print(f"Ошибка в {config['file']}: {e}")

plt.xlabel('Размер сообщения (байт)', fontsize=14)
plt.ylabel('Время (микросекунды)', fontsize=14)
plt.title('Зависимость времени передачи от размера сообщения\n(разные уровни коммуникационной среды)', 
          fontsize=16, pad=20)

plt.grid(True, which='both', alpha=0.3, linestyle='-', linewidth=0.5)
plt.legend(loc='upper left', fontsize=11, framealpha=0.9)

plt.tight_layout()
plt.savefig('time_dependence.png', dpi=300, bbox_inches='tight')
print("График сохранен: time_dependence.png")

plt.show()
