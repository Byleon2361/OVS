#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace chrono;

struct Task {
  int r;
  int t;
  int original_index;
};

struct TournamentNode {
  int value;
  int level_index;
};

class TournamentTree {
private:
  vector<TournamentNode> tree;
  int size;
  int capacity;

public:
  TournamentTree(int max_levels) {

    size = 1;
    while (size < max_levels) {
      size <<= 1;
    }
    capacity = 2 * size;
    tree.resize(capacity);

    for (int i = 0; i < size; i++) {
      if (i < max_levels) {
        tree[size + i].value = 0;
        tree[size + i].level_index = i;
      } else {
        tree[size + i].value = -1;
        tree[size + i].level_index = i;
      }
    }

    for (int i = size - 1; i > 0; i--) {
      tree[i].value = max(tree[2 * i].value, tree[2 * i + 1].value);
      tree[i].level_index = -1;
    }
  }

  int find_first_fit(int required_em) {
    if (tree[1].value < required_em) {
      return -1;
    }

    int node = 1;
    while (node < size) {
      if (tree[2 * node].value >= required_em) {
        node = 2 * node;
      } else {
        node = 2 * node + 1;
      }
    }

    return tree[node].level_index;
  }

  void update_level(int level_index, int new_free_em) {
    int node = size + level_index;
    tree[node].value = new_free_em;

    node /= 2;
    while (node >= 1) {
      tree[node].value = max(tree[2 * node].value, tree[2 * node + 1].value);
      node /= 2;
    }
  }

  int get_level_value(int level_index) { return tree[size + level_index].value; }
};

struct Schedule {
  vector<vector<Task>> levels;
  double T;
};

void counting_sort_by_time(vector<Task> &tasks, int max_time) {
  if (tasks.empty())
    return;

  vector<vector<Task>> buckets(max_time + 1);

  for (const auto &task : tasks) {
    buckets[task.t].push_back(task);
  }

  tasks.clear();
  for (int t = max_time; t >= 1; t--) {
    for (const auto &task : buckets[t]) {
      tasks.push_back(task);
    }
  }
}

void counting_sort_by_time_with_indices(vector<Task> &tasks, int max_time) {
  if (tasks.empty())
    return;

  vector<vector<Task>> buckets(max_time + 1);

  for (const auto &task : tasks) {
    buckets[task.t].push_back(task);
  }

  tasks.clear();
  for (int t = max_time; t >= 1; t--) {
    for (const auto &task : buckets[t]) {
      tasks.push_back(task);
    }
  }
}

double compute_lower_bound(const vector<Task> &tasks) {
  double sum = 0.0;
  for (const auto &task : tasks) {
    sum += task.r * task.t;
  }
  return sum;
}

double compute_epsilon(double T, double T_prime) { return (T - T_prime) / T_prime; }

Schedule NFDH(vector<Task> tasks, int n_em) {
  auto start_time = high_resolution_clock::now();

  Schedule schedule;
  double T_prime = compute_lower_bound(tasks);

  counting_sort_by_time(tasks, 100);

  vector<vector<Task>> levels;
  vector<int> free_em;

  for (const auto &task : tasks) {
    bool placed = false;

    if (!levels.empty()) {
      int last_level = levels.size() - 1;
      if (free_em[last_level] >= task.r) {
        levels[last_level].push_back(task);
        free_em[last_level] -= task.r;
        placed = true;
      }
    }

    if (!placed) {
      levels.push_back({task});
      free_em.push_back(n_em - task.r);
    }
  }

  double T_value = 0.0;
  for (size_t i = 0; i < levels.size(); i++) {

    int max_t = 0;
    for (const auto &task : levels[i]) {
      max_t = max(max_t, task.t);
    }
    T_value += max_t;
  }

  auto end_time = high_resolution_clock::now();
  double elapsed_time = duration<double>(end_time - start_time).count();

  schedule.levels = levels;
  schedule.T = T_value;

  cout << "Алгоритм: NFDH" << endl;
  cout << "T(S) = " << T_value << endl;
  cout << "T' = " << T_prime << endl;
  cout << "ε = " << compute_epsilon(T_value, T_prime) * 100 << "%" << endl;
  cout << "Время выполнения: " << elapsed_time << " сек" << endl;
  cout << "Количество уровней: " << levels.size() << endl;
  cout << "------------------------" << endl;

  return schedule;
}

Schedule FFDH(vector<Task> tasks, int n_em) {
  auto start_time = high_resolution_clock::now();

  Schedule schedule;
  double T_prime = compute_lower_bound(tasks);

  counting_sort_by_time(tasks, 100);

  vector<vector<Task>> levels;
  vector<int> free_em;
  vector<int> level_max_t;

  TournamentTree tree(tasks.size());

  for (const auto &task : tasks) {

    int level_index = tree.find_first_fit(task.r);

    if (level_index != -1 && level_index < static_cast<int>(levels.size())) {

      levels[level_index].push_back(task);
      free_em[level_index] -= task.r;

      level_max_t[level_index] = max(level_max_t[level_index], task.t);

      tree.update_level(level_index, free_em[level_index]);
    } else {

      levels.push_back({task});
      free_em.push_back(n_em - task.r);
      level_max_t.push_back(task.t);

      tree.update_level(levels.size() - 1, n_em - task.r);
    }
  }

  double T_value = 0.0;
  for (size_t i = 0; i < levels.size(); i++) {
    T_value += level_max_t[i];
  }

  auto end_time = high_resolution_clock::now();
  double elapsed_time = duration<double>(end_time - start_time).count();

  schedule.levels = levels;
  schedule.T = T_value;

  cout << "Алгоритм: FFDH" << endl;
  cout << "T(S) = " << T_value << endl;
  cout << "T' = " << T_prime << endl;
  cout << "ε = " << compute_epsilon(T_value, T_prime) * 100 << "%" << endl;
  cout << "Время выполнения: " << elapsed_time << " сек" << endl;
  cout << "Количество уровней: " << levels.size() << endl;
  cout << "------------------------" << endl;

  return schedule;
}

vector<Task> generate_random_tasks(int m, int n_em, int max_t, mt19937 &gen) {
  uniform_int_distribution<> r_dist(1, n_em);
  uniform_int_distribution<> t_dist(1, max_t);

  vector<Task> tasks(m);
  for (int i = 0; i < m; i++) {
    tasks[i].r = r_dist(gen);
    tasks[i].t = t_dist(gen);
    tasks[i].original_index = i;
  }

  return tasks;
}

vector<Task> read_tasks_from_file(const string &filename) {
  vector<Task> tasks;
  ifstream file(filename);

  if (!file.is_open()) {
    cerr << "Ошибка: не удалось открыть файл " << filename << endl;
    return tasks;
  }

  int r, t;
  int index = 0;
  while (file >> r >> t) {
    tasks.push_back({r, t, index++});
  }

  file.close();
  return tasks;
}

void save_schedule_to_file(const Schedule &schedule, const string &filename) {
  ofstream file(filename);

  if (!file.is_open()) {
    cerr << "Ошибка: не удалось создать файл " << filename << endl;
    return;
  }

  file << "Расписание (уровни задач):" << endl;
  file << "Количество уровней: " << schedule.levels.size() << endl;
  file << "T(S) = " << schedule.T << endl;
  file << "------------------------" << endl;

  for (size_t i = 0; i < schedule.levels.size(); i++) {
    file << "Уровень " << i + 1 << ": ";
    for (const auto &task : schedule.levels[i]) {
      file << "(" << task.r << "," << task.t << ") ";
    }
    file << endl;
  }

  file.close();
}

void study_time_dependence(int n_em) {
  cout << "\n==========================================" << endl;
  cout << "Исследование времени выполнения (n = " << n_em << ")" << endl;
  cout << "==========================================" << endl;

  random_device rd;
  mt19937 gen(rd());

  vector<int> m_values = {500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000};

  cout << setw(10) << "m" << setw(15) << "NFDH время" << setw(15) << "FFDH время" << endl;
  cout << "------------------------------------------------" << endl;

  for (int m : m_values) {
    vector<Task> tasks = generate_random_tasks(m, n_em, 100, gen);

    auto start = high_resolution_clock::now();
    NFDH(tasks, n_em);
    auto end = high_resolution_clock::now();
    double nfdh_time = duration<double>(end - start).count();

    start = high_resolution_clock::now();
    FFDH(tasks, n_em);
    end = high_resolution_clock::now();
    double ffdh_time = duration<double>(end - start).count();

    cout << setw(10) << m << setw(15) << nfdh_time << setw(15) << ffdh_time << endl;
  }
}

void study_accuracy_random(int n_em, int num_experiments = 10) {
  cout << "\n==========================================" << endl;
  cout << "Сравнительный анализ точности (n = " << n_em << ", случайные данные)" << endl;
  cout << "==========================================" << endl;

  random_device rd;
  mt19937 gen(rd());

  vector<int> m_values = {500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000};

  cout << setw(10) << "m" << setw(15) << "NFDH ε ср" << setw(15) << "NFDH σ" << setw(15) << "FFDH ε ср" << setw(15)
       << "FFDH σ" << setw(15) << "Лучший" << endl;
  cout << "----------------------------------------------------------------------------------------" << endl;

  for (int m : m_values) {
    vector<double> nfdh_epsilons;
    vector<double> ffdh_epsilons;

    for (int exp = 0; exp < num_experiments; exp++) {
      vector<Task> tasks = generate_random_tasks(m, n_em, 100, gen);

      Schedule nfdh_sched = NFDH(tasks, n_em);
      Schedule ffdh_sched = FFDH(tasks, n_em);

      double T_prime = compute_lower_bound(tasks);

      double nfdh_eps = compute_epsilon(nfdh_sched.T, T_prime);
      double ffdh_eps = compute_epsilon(ffdh_sched.T, T_prime);

      nfdh_epsilons.push_back(nfdh_eps);
      ffdh_epsilons.push_back(ffdh_eps);
    }

    double nfdh_sum = 0.0;
    for (double eps : nfdh_epsilons)
      nfdh_sum += eps;
    double nfdh_mean = nfdh_sum / num_experiments;

    double nfdh_var = 0.0;
    for (double eps : nfdh_epsilons)
      nfdh_var += (eps - nfdh_mean) * (eps - nfdh_mean);
    double nfdh_std = sqrt(nfdh_var / num_experiments);

    double ffdh_sum = 0.0;
    for (double eps : ffdh_epsilons)
      ffdh_sum += eps;
    double ffdh_mean = ffdh_sum / num_experiments;

    double ffdh_var = 0.0;
    for (double eps : ffdh_epsilons)
      ffdh_var += (eps - ffdh_mean) * (eps - ffdh_mean);
    double ffdh_std = sqrt(ffdh_var / num_experiments);

    string best = (nfdh_mean < ffdh_mean) ? "NFDH" : "FFDH";

    cout << setw(10) << m << setw(15) << nfdh_mean * 100 << "%" << setw(15) << nfdh_std * 100 << "%" << setw(15)
         << ffdh_mean * 100 << "%" << setw(15) << ffdh_std * 100 << "%" << setw(15) << best << endl;
  }
}

void study_accuracy_real(const string &system_name, const vector<int> &m_values) {
  cout << "\n==========================================" << endl;
  cout << "Сравнительный анализ точности (система: " << system_name << ")" << endl;
  cout << "==========================================" << endl;

  random_device rd;
  mt19937 gen(rd());

  cout << setw(10) << "m" << setw(15) << "NFDH ε ср" << setw(15) << "NFDH σ" << setw(15) << "FFDH ε ср" << setw(15)
       << "FFDH σ" << setw(15) << "Лучший" << endl;
  cout << "----------------------------------------------------------------------------------------" << endl;

  for (int m : m_values) {
    vector<double> nfdh_epsilons;
    vector<double> ffdh_epsilons;

    uniform_int_distribution<> r_dist_big(512, 1024);
    uniform_int_distribution<> t_dist_big(50, 100);

    for (int exp = 0; exp < 10; exp++) {
      vector<Task> tasks(m);
      for (int i = 0; i < m; i++) {

        if (gen() % 100 < 30) {
          tasks[i].r = r_dist_big(gen);
          tasks[i].t = t_dist_big(gen);
        } else {
          tasks[i].r = uniform_int_distribution<>(1, 256)(gen);
          tasks[i].t = uniform_int_distribution<>(1, 50)(gen);
        }
        tasks[i].original_index = i;
      }

      Schedule nfdh_sched = NFDH(tasks, 1024);
      Schedule ffdh_sched = FFDH(tasks, 1024);

      double T_prime = compute_lower_bound(tasks);

      double nfdh_eps = compute_epsilon(nfdh_sched.T, T_prime);
      double ffdh_eps = compute_epsilon(ffdh_sched.T, T_prime);

      nfdh_epsilons.push_back(nfdh_eps);
      ffdh_epsilons.push_back(ffdh_eps);
    }

    double nfdh_mean = 0.0, ffdh_mean = 0.0;
    for (double eps : nfdh_epsilons)
      nfdh_mean += eps;
    for (double eps : ffdh_epsilons)
      ffdh_mean += eps;
    nfdh_mean /= 10;
    ffdh_mean /= 10;

    double nfdh_var = 0.0, ffdh_var = 0.0;
    for (double eps : nfdh_epsilons)
      nfdh_var += (eps - nfdh_mean) * (eps - nfdh_mean);
    for (double eps : ffdh_epsilons)
      ffdh_var += (eps - ffdh_mean) * (eps - ffdh_mean);
    double nfdh_std = sqrt(nfdh_var / 10);
    double ffdh_std = sqrt(ffdh_var / 10);

    string best = (nfdh_mean < ffdh_mean) ? "NFDH" : "FFDH";

    cout << setw(10) << m << setw(15) << nfdh_mean * 100 << "%" << setw(15) << nfdh_std * 100 << "%" << setw(15)
         << ffdh_mean * 100 << "%" << setw(15) << ffdh_std * 100 << "%" << setw(15) << best << endl;
  }
}

int main(int argc, char *argv[]) {
  cout << "==========================================" << endl;
  cout << "Программа для решения задачи упаковки задач (NFDH и FFDH)" << endl;
  cout << "==========================================" << endl;

  if (argc == 4) {

    string filename = argv[1];
    int n_em = atoi(argv[2]);
    string algorithm = argv[3];

    cout << "Чтение задач из файла: " << filename << endl;
    cout << "Количество ЭМ: " << n_em << endl;
    cout << "Алгоритм: " << algorithm << endl;
    cout << "------------------------" << endl;

    vector<Task> tasks = read_tasks_from_file(filename);

    if (tasks.empty()) {
      cerr << "Нет задач для обработки" << endl;
      return 1;
    }

    Schedule result;
    if (algorithm == "NFDH" || algorithm == "nfdh") {
      result = NFDH(tasks, n_em);
    } else if (algorithm == "FFDH" || algorithm == "ffdh") {
      result = FFDH(tasks, n_em);
    } else {
      cerr << "Неизвестный алгоритм. Используйте NFDH или FFDH" << endl;
      return 1;
    }

    string output_filename = "schedule_" + algorithm + "_" + filename;
    save_schedule_to_file(result, output_filename);
    cout << "Расписание сохранено в файл: " << output_filename << endl;

  } else {

    cout << "\nИсследовательский режим" << endl;
    cout << "------------------------" << endl;

    study_time_dependence(1024);
    study_time_dependence(4096);

    study_accuracy_random(1024, 10);

    vector<int> real_m_values = {500, 1000, 1500};
    study_accuracy_real("LLNL Thunder", real_m_values);
  }

  cout << "\nПрограмма завершена." << endl;

  return 0;
}
