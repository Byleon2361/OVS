#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using Matrix = std::vector<std::vector<double>>;
using Vec = std::vector<double>;

Matrix build_matrix(int n, double c1, double c2, double c3) {
  int sz = n + 1;
  Matrix C(sz, Vec(sz, 0.0));
  for (int i = 0; i < sz; ++i)
    for (int j = 0; j < sz; ++j) {
      if (i == j)
        C[i][j] = c1 * i;
      else if (i > j)
        C[i][j] = c2 * i - c1 * j;
      else
        C[i][j] = c3 * j - c1 * i;
    }
  return C;
}

static int argmax(const Vec &v) { return (int)(std::max_element(v.begin(), v.end()) - v.begin()); }
static int argmin(const Vec &v) { return (int)(std::min_element(v.begin(), v.end()) - v.begin()); }

struct BrownResult {
  Vec x;           // стратегия ВЦ (строк)
  Vec y;           // стратегия диспетчера (столбцов)
  double V;        // цена игры
  long iterations; // число итераций
};

BrownResult brown(const Matrix &C, double eps) {
  int m = (int)C.size();
  int n = (int)C[0].size();

  Vec A(n, 0.0);
  Vec B(m, 0.0);
  std::vector<long> x_cnt(m, 0L);
  std::vector<long> y_cnt(n, 0L);

  x_cnt[0]++;
  for (int j = 0; j < n; ++j)
    A[j] += C[0][j];

  int j0 = argmin(A);
  y_cnt[j0]++;
  for (int i = 0; i < m; ++i)
    B[i] += C[i][j0];

  long l = 1;
  double V_low = *std::min_element(A.begin(), A.end()) / l;
  double V_high = *std::max_element(B.begin(), B.end()) / l;

  while (V_high - V_low >= eps) {
    int i_star = argmax(B);
    x_cnt[i_star]++;
    for (int j = 0; j < n; ++j)
      A[j] += C[i_star][j];

    int j_star = argmin(A);
    y_cnt[j_star]++;
    for (int i = 0; i < m; ++i)
      B[i] += C[i][j_star];

    ++l;
    V_low = *std::min_element(A.begin(), A.end()) / l;
    V_high = *std::max_element(B.begin(), B.end()) / l;
  }

  BrownResult res;
  res.iterations = l;
  res.V = (V_low + V_high) / 2.0;
  res.x.resize(m);
  res.y.resize(n);
  for (int i = 0; i < m; ++i)
    res.x[i] = (double)x_cnt[i] / l;
  for (int j = 0; j < n; ++j)
    res.y[j] = (double)y_cnt[j] / l;
  return res;
}

void print_matrix(const Matrix &C) {
  for (auto &row : C) {
    for (double v : row)
      std::cout << std::setw(8) << std::fixed << std::setprecision(2) << v;
    std::cout << "\n";
  }
}

void print_vec(const Vec &v, int decimals = 2) {
  for (size_t i = 0; i < v.size(); ++i) {
    if (i)
      std::cout << "  ";
    std::cout << std::fixed << std::setprecision(decimals) << v[i];
  }
  std::cout << "\n";
}

struct Config {
  int n = 10;
  double c1 = 1.0;
  double c2 = 2.0;
  double c3 = 3.0;
  double eps = 0.01;
  int bench = 0;
  std::string file;
};

Config parse_args(int argc, char *argv[]) {
  Config cfg;
  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "--n" && i + 1 < argc)
      cfg.n = std::stoi(argv[++i]);
    else if (a == "--c1" && i + 1 < argc)
      cfg.c1 = std::stod(argv[++i]);
    else if (a == "--c2" && i + 1 < argc)
      cfg.c2 = std::stod(argv[++i]);
    else if (a == "--c3" && i + 1 < argc)
      cfg.c3 = std::stod(argv[++i]);
    else if (a == "--eps" && i + 1 < argc)
      cfg.eps = std::stod(argv[++i]);
    else if (a == "--bench" && i + 1 < argc)
      cfg.bench = std::stoi(argv[++i]);
    else if (a == "--file" && i + 1 < argc)
      cfg.file = argv[++i];
  }
  return cfg;
}

int main(int argc, char *argv[]) {
  Config cfg = parse_args(argc, argv);

  if (!cfg.file.empty()) {
    std::ifstream f(cfg.file);
    if (!f) {
      std::cerr << "Не удалось открыть файл: " << cfg.file << "\n";
      return 1;
    }
    f >> cfg.n >> cfg.c1 >> cfg.c2 >> cfg.c3;
    if (f >> cfg.eps) {
    }
  }

  if (cfg.bench > 0) {
    std::ofstream csv("time_data.csv");
    csv << "n,time_sec\n";
    std::cout << "\nБенчмарк: n от 2 до " << cfg.bench << "\n";
    for (int n = 2; n <= cfg.bench; ++n) {
      Matrix C = build_matrix(n, cfg.c1, cfg.c2, cfg.c3);
      auto t0 = std::chrono::high_resolution_clock::now();
      brown(C, cfg.eps);
      auto t1 = std::chrono::high_resolution_clock::now();
      double sec = std::chrono::duration<double>(t1 - t0).count();
      csv << n << "," << std::fixed << std::setprecision(6) << sec << "\n";
      std::cout << "  n=" << std::setw(3) << n << "  time=" << std::fixed << std::setprecision(4) << sec << " с\n";
    }
    std::cout << "Данные сохранены в time_data.csv\n";
  }

  std::cout << std::string(62, '=') << "\n";
  std::cout << "  ЗАДАЧА «ДИСПЕТЧЕР – ВЫЧИСЛИТЕЛЬНЫЙ ЦЕНТР»\n";
  std::cout << std::string(62, '=') << "\n";
  std::cout << "  n=" << cfg.n << "  c1=" << cfg.c1 << "  c2=" << cfg.c2 << "  c3=" << cfg.c3 << "  eps=" << cfg.eps
            << "\n\n";

  Matrix C = build_matrix(cfg.n, cfg.c1, cfg.c2, cfg.c3);

  std::cout << "Матрица платежей C:\n";
  print_matrix(C);

  std::cout << "\nЗапуск итеративного метода Брауна...\n";
  auto t0 = std::chrono::high_resolution_clock::now();
  BrownResult res = brown(C, cfg.eps);
  auto t1 = std::chrono::high_resolution_clock::now();
  double sec = std::chrono::duration<double>(t1 - t0).count();

  std::cout << "\nКоличество итераций  l = " << res.iterations << "\n";
  std::cout << "Время решения           = " << std::fixed << std::setprecision(4) << sec << " с\n";
  std::cout << "Цена игры            V = " << std::fixed << std::setprecision(3) << res.V << "\n";

  std::cout << "\nОптимальные смешанные стратегии ВЦ (строки):\n  ";
  print_vec(res.x);
  std::cout << "\nОптимальные смешанные стратегии диспетчера (столбцы):\n  ";
  print_vec(res.y);

  std::cout << "\n" << std::string(62, '=') << "\n";
  std::cout << "  ИНТЕРПРЕТАЦИЯ РЕЗУЛЬТАТОВ\n";
  std::cout << std::string(62, '=') << "\n";

  const double thr = 1e-3;
  std::cout << "\nВЦ использует смешанную стратегию:\n";
  for (int i = 0; i < (int)res.x.size(); ++i)
    if (res.x[i] > thr)
      std::cout << "  стратегия " << std::setw(2) << i << "  (выделить задаче " << i << " ЭМ)"
                << "  — вероятность " << std::fixed << std::setprecision(3) << res.x[i] << "\n";

  std::cout << "\nДиспетчер использует смешанную стратегию:\n";
  for (int j = 0; j < (int)res.y.size(); ++j)
    if (res.y[j] > thr)
      std::cout << "  стратегия " << std::setw(2) << j << "  (послать задачу ранга " << j << ")"
                << "  — вероятность " << std::fixed << std::setprecision(3) << res.y[j] << "\n";

  std::cout << "\nЦена игры V ≈ " << std::fixed << std::setprecision(4) << res.V
            << ": при оптимальной игре обеих сторон\n"
               "среднее значение платежа стремится к этому значению.\n\n";

  return 0;
}
