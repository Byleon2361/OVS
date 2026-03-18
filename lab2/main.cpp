#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

struct Task {
  int r;
  int t;
};

struct ScheduleEntry {
  long long tau;
  std::vector<int> machines;
};

struct Result {
  std::vector<ScheduleEntry> schedule;
  long long T_S;
  double eps;
  double elapsed_sec;
};

std::vector<int> counting_sort_desc(const std::vector<Task> &tasks) {
  if (tasks.empty())
    return {};

  int t_max = 0;
  for (auto &task : tasks)
    t_max = std::max(t_max, task.t);

  std::vector<std::vector<int>> buckets(t_max + 1);
  for (int i = 0; i < (int)tasks.size(); ++i)
    buckets[tasks[i].t].push_back(i);

  std::vector<int> order;
  order.reserve(tasks.size());
  for (int v = t_max; v >= 0; --v)
    for (int idx : buckets[v])
      order.push_back(idx);

  return order;
}

class TournamentTree {
public:
  explicit TournamentTree(int capacity) {
    n_leaves = 1;
    while (n_leaves < capacity)
      n_leaves <<= 1;
    tree.assign(2 * n_leaves, 0);
  }

  void update(int idx, int value) {
    int pos = n_leaves + idx;
    tree[pos] = value;
    for (pos >>= 1; pos >= 1; pos >>= 1)
      tree[pos] = std::max(tree[2 * pos], tree[2 * pos + 1]);
  }

  int find_first_fit(int required, int num_active) const {
    if (tree[1] < required)
      return -1;
    int pos = 1;
    while (pos < n_leaves) {
      int left = 2 * pos;
      int right = 2 * pos + 1;
      pos = (tree[left] >= required) ? left : right;
    }
    int leaf_idx = pos - n_leaves;
    return (leaf_idx < num_active) ? leaf_idx : -1;
  }

private:
  int n_leaves;
  std::vector<int> tree;
};

Result nfdh(const std::vector<Task> &tasks, int n) {
  int m = (int)tasks.size();
  Result res;
  res.schedule.resize(m);

  if (m == 0) {
    res.T_S = 0;
    res.eps = 0;
    return res;
  }

  auto order = counting_sort_desc(tasks);

  struct Shelf {
    long long start;
    int height;
    int next_col;
  };
  std::vector<Shelf> shelves;
  long long cumtime = 0;

  for (int orig_idx : order) {
    int r = tasks[orig_idx].r;
    int t = tasks[orig_idx].t;

    bool placed = false;
    if (!shelves.empty()) {
      auto &shelf = shelves.back();
      int free = n - shelf.next_col + 1;
      if (free >= r) {
        std::vector<int> mach(r);
        std::iota(mach.begin(), mach.end(), shelf.next_col);
        res.schedule[orig_idx] = {shelf.start, std::move(mach)};
        shelf.next_col += r;
        placed = true;
      }
    }
    if (!placed) {
      long long start = cumtime;
      cumtime += t;
      shelves.push_back({start, t, r + 1});
      std::vector<int> mach(r);
      std::iota(mach.begin(), mach.end(), 1);
      res.schedule[orig_idx] = {start, std::move(mach)};
    }
  }

  res.T_S = cumtime;
  double T_prime = 0;
  for (auto &tk : tasks)
    T_prime += (double)tk.r * tk.t;
  T_prime /= n;
  res.eps = (T_prime > 0) ? (res.T_S - T_prime) / T_prime : 0.0;
  return res;
}

Result ffdh(const std::vector<Task> &tasks, int n) {
  int m = (int)tasks.size();
  Result res;
  res.schedule.resize(m);

  if (m == 0) {
    res.T_S = 0;
    res.eps = 0;
    return res;
  }

  auto order = counting_sort_desc(tasks);

  struct Shelf {
    long long start;
    int height;
    int next_col;
  };
  std::vector<Shelf> shelf_data;
  long long cumtime = 0;
  int num_shelves = 0;
  TournamentTree tree(m);

  for (int orig_idx : order) {
    int r = tasks[orig_idx].r;
    int t = tasks[orig_idx].t;

    int idx = tree.find_first_fit(r, num_shelves);

    if (idx == -1) {

      long long start = cumtime;
      cumtime += t;
      shelf_data.push_back({start, t, r + 1});
      std::vector<int> mach(r);
      std::iota(mach.begin(), mach.end(), 1);
      res.schedule[orig_idx] = {start, std::move(mach)};
      tree.update(num_shelves, n - r);
      ++num_shelves;
    } else {
      auto &shelf = shelf_data[idx];
      std::vector<int> mach(r);
      std::iota(mach.begin(), mach.end(), shelf.next_col);
      res.schedule[orig_idx] = {shelf.start, std::move(mach)};
      shelf.next_col += r;
      tree.update(idx, n - shelf.next_col + 1);
    }
  }

  res.T_S = cumtime;
  double T_prime = 0;
  for (auto &tk : tasks)
    T_prime += (double)tk.r * tk.t;
  T_prime /= n;
  res.eps = (T_prime > 0) ? (res.T_S - T_prime) / T_prime : 0.0;
  return res;
}

std::vector<Task> read_tasks(const std::string &filename) {
  std::ifstream f(filename);
  if (!f)
    throw std::runtime_error("Не удалось открыть файл: " + filename);

  std::vector<Task> tasks;
  std::string line;
  while (std::getline(f, line)) {
    if (line.empty() || line[0] == '#')
      continue;
    std::istringstream ss(line);
    Task tk;
    if (ss >> tk.r >> tk.t)
      tasks.push_back(tk);
  }
  return tasks;
}

void write_tasks(const std::string &filename, const std::vector<Task> &tasks, int n) {
  std::ofstream f(filename);
  f << "# m=" << tasks.size() << " n=" << n << "\n";
  for (auto &tk : tasks)
    f << tk.r << " " << tk.t << "\n";
}

std::vector<Task> generate_tasks(int m, int n, unsigned seed) {
  std::mt19937 rng(seed);
  std::uniform_int_distribution<int> dr(1, n), dt(1, 100);
  std::vector<Task> tasks(m);
  for (auto &tk : tasks) {
    tk.r = dr(rng);
    tk.t = dt(rng);
  }
  return tasks;
}

Result run_algorithm(const std::string &algo, const std::vector<Task> &tasks, int n) {
  auto t0 = std::chrono::high_resolution_clock::now();
  Result res = (algo == "NFDH") ? nfdh(tasks, n) : ffdh(tasks, n);
  auto t1 = std::chrono::high_resolution_clock::now();
  res.elapsed_sec = std::chrono::duration<double>(t1 - t0).count();
  return res;
}

void print_schedule(const Result &res, const std::vector<Task> &tasks, const std::string &algo, bool verbose = true) {
  std::cout << "\n" << std::string(52, '=') << "\n";
  std::cout << "Алгоритм  : " << algo << "\n";
  std::cout << "T(S)      : " << res.T_S << "\n";
  std::cout << "ε         : " << std::fixed << std::setprecision(6) << res.eps << "\n";
  std::cout << "Время     : " << std::fixed << std::setprecision(6) << res.elapsed_sec << " с\n";

  if (verbose) {
    std::cout << "\nРасписание (j, τ_j, r, t, ЭМ):\n";
    for (int j = 0; j < (int)res.schedule.size(); ++j) {
      auto &e = res.schedule[j];
      std::cout << "  j=" << std::setw(4) << j + 1 << "  τ=" << std::setw(6) << e.tau << "  r=" << std::setw(4)
                << tasks[j].r << "  t=" << std::setw(4) << tasks[j].t << "  ЭМ=[";
      int lim = std::min((int)e.machines.size(), 5);
      for (int k = 0; k < lim; ++k) {
        if (k)
          std::cout << ", ";
        std::cout << e.machines[k];
      }
      if ((int)e.machines.size() > 5)
        std::cout << ", ...";
      std::cout << "]\n";
    }
  }

  double T_prime = 0;
  for (auto &tk : tasks)
    T_prime += (double)tk.r * tk.t;
  T_prime /= /* n from eps */ 1;
  std::cout << std::string(52, '=') << "\n";
}

void experiment_time() {
  const std::vector<int> M_VALUES = {500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000};
  const std::vector<int> N_VALUES = {1024, 4096};

  std::cout << "\n" << std::string(70, '=') << "\n";
  std::cout << "ЭКСПЕРИМЕНТ 2 — Зависимость времени от m и n\n";
  std::cout << std::string(70, '=') << "\n";
  std::cout << std::setw(6) << "m" << std::setw(6) << "n" << std::setw(14) << "NFDH (с)" << std::setw(14) << "FFDH (с)"
            << "\n";
  std::cout << std::string(42, '-') << "\n";

  for (int n : N_VALUES) {
    for (int i = 0; i < (int)M_VALUES.size(); ++i) {
      int m = M_VALUES[i];
      auto tasks = generate_tasks(m, n, (unsigned)i);

      auto r_n = run_algorithm("NFDH", tasks, n);
      auto r_f = run_algorithm("FFDH", tasks, n);

      std::cout << std::setw(6) << m << std::setw(6) << n << std::fixed << std::setprecision(6) << std::setw(14)
                << r_n.elapsed_sec << std::setw(14) << r_f.elapsed_sec << "\n";
    }
  }

  std::cout << "\nВывод: NFDH — O(m + T_max), FFDH — O(m log m).\n"
               "При росте m оба алгоритма замедляются линейно (FFDH — с log-множителем).\n"
               "При росте n время увеличивается: больше ЭМ → обычно больше полок.\n";
}

void experiment_quality_random() {
  const std::vector<int> M_VALUES = {500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000};
  const int N = 1024;

  std::cout << "\n" << std::string(70, '=') << "\n";
  std::cout << "ЭКСПЕРИМЕНТ 3 — Качество расписаний (случайные задачи, n=1024)\n";
  std::cout << std::string(70, '=') << "\n";
  std::cout << std::setw(6) << "m" << std::setw(12) << "ε NFDH" << std::setw(12) << "ε FFDH" << "\n";
  std::cout << std::string(32, '-') << "\n";

  std::vector<double> eps_n_list, eps_f_list;
  for (int i = 0; i < (int)M_VALUES.size(); ++i) {
    int m = M_VALUES[i];
    auto tasks = generate_tasks(m, N, (unsigned)i);

    auto r_n = run_algorithm("NFDH", tasks, N);
    auto r_f = run_algorithm("FFDH", tasks, N);

    eps_n_list.push_back(r_n.eps);
    eps_f_list.push_back(r_f.eps);

    std::cout << std::setw(6) << m << std::fixed << std::setprecision(6) << std::setw(12) << r_n.eps << std::setw(12)
              << r_f.eps << "\n";
  }

  auto mean_std = [](const std::vector<double> &v) -> std::pair<double, double> {
    double mean = 0;
    for (double x : v)
      mean += x;
    mean /= v.size();
    double var = 0;
    for (double x : v)
      var += (x - mean) * (x - mean);
    var /= (v.size() - 1);
    return {mean, std::sqrt(var)};
  };

  auto [mn, sn] = mean_std(eps_n_list);
  auto [mf, sf] = mean_std(eps_f_list);

  std::cout << std::string(32, '-') << "\n";
  std::cout << std::setw(6) << "E[ε]" << std::fixed << std::setprecision(6) << std::setw(12) << mn << std::setw(12)
            << mf << "\n";
  std::cout << std::setw(6) << "σ[ε]" << std::setw(12) << sn << std::setw(12) << sf << "\n";

  std::string better = (mf < mn) ? "FFDH" : "NFDH";
  std::cout << "\nБолее точные расписания формирует: " << better << "\n";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Использование:\n"
              << "  " << argv[0] << " <файл_задач> <n> <NFDH|FFDH>\n"
              << "  " << argv[0] << " --gen <m> <n> <seed>\n"
              << "  " << argv[0] << " --exp-time\n"
              << "  " << argv[0] << " --exp-quality\n";
    return 0;
  }

  std::string cmd = argv[1];

  if (cmd == "--gen") {
    if (argc < 5) {
      std::cerr << "Нужно: --gen <m> <n> <seed>\n";
      return 1;
    }
    int m = std::stoi(argv[2]);
    int n = std::stoi(argv[3]);
    unsigned seed = (unsigned)std::stoul(argv[4]);
    auto tasks = generate_tasks(m, n, seed);
    std::string fname = "tasks_m" + std::to_string(m) + "_n" + std::to_string(n) + "_s" + std::to_string(seed) + ".txt";
    write_tasks(fname, tasks, n);
    std::cout << "Набор задач сохранён в " << fname << " (m=" << m << ", n=" << n << ")\n";
    return 0;
  }

  if (cmd == "--exp-time") {
    experiment_time();
    return 0;
  }
  if (cmd == "--exp-quality") {
    experiment_quality_random();
    return 0;
  }

  if (argc < 4) {
    std::cerr << "Нужно: " << argv[0] << " <файл_задач> <n> <NFDH|FFDH>\n";
    return 1;
  }
  std::string filename = argv[1];
  int n = std::stoi(argv[2]);
  std::string algo = argv[3];

  for (char &c : algo)
    c = (char)toupper((unsigned char)c);

  if (algo != "NFDH" && algo != "FFDH") {
    std::cerr << "Неизвестный алгоритм: " << algo << ". Допустимые: NFDH, FFDH\n";
    return 1;
  }

  std::vector<Task> tasks;
  try {
    tasks = read_tasks(filename);
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
  if (tasks.empty()) {
    std::cerr << "Набор задач пуст.\n";
    return 1;
  }

  for (int j = 0; j < (int)tasks.size(); ++j) {
    if (tasks[j].r > n) {
      std::cerr << "ОШИБКА: задача " << j + 1 << " требует r=" << tasks[j].r << " ЭМ, но n=" << n << "\n";
      return 1;
    }
  }

  auto res = run_algorithm(algo, tasks, n);

  bool verbose = (int)tasks.size() <= 50;
  print_schedule(res, tasks, algo, verbose);

  double T_prime = 0;
  for (auto &tk : tasks)
    T_prime += (double)tk.r * tk.t;
  T_prime /= n;
  std::cout << std::fixed << std::setprecision(4);
  std::cout << "\nНижняя граница T' = " << T_prime << "\n";
  std::cout << "T(S) = " << res.T_S << ",  ε = (T(S) - T') / T' = " << std::setprecision(6) << res.eps << "\n";

  return 0;
}
