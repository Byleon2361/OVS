#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

// Функция для форматирования числа в научной нотации для имени файла
std::string format_lambda_filename(double lambda) {
  std::stringstream ss;
  if (lambda == 1e-5)
    ss << "1e-05";
  else if (lambda == 1e-6)
    ss << "1e-06";
  else if (lambda == 1e-7)
    ss << "1e-07";
  else if (lambda == 1e-8)
    ss << "1e-08";
  else if (lambda == 1e-9)
    ss << "1e-09";
  else
    ss << std::scientific << std::setprecision(0) << lambda;
  return ss.str();
}

// Функция для вычисления суммы для Theta
double sum_theta(int N, int n, double lambda, const std::vector<double> &mu_l) {
  double sum = 0.0;
  for (int j = n + 1; j <= N; ++j) {
    double prod = 1.0;
    for (int l = n; l <= j - 1; ++l) {
      prod *= (mu_l[l] / (l * lambda));
      if (std::isinf(prod) || prod > 1e100) {
        return std::numeric_limits<double>::infinity();
      }
    }
    double term = (1.0 / (j * lambda)) * prod;
    if (std::isinf(term) || term > 1e100) {
      return std::numeric_limits<double>::infinity();
    }
    sum += term;
    if (std::isinf(sum)) {
      return std::numeric_limits<double>::infinity();
    }
  }
  return sum;
}

// Функция для вычисления суммы для T (исправленная формула)
double sum_T(int N, int n, double lambda, const std::vector<double> &mu_l) {
  double sum = 0.0;
  for (int j = 1; j <= n; ++j) {
    double prod = 1.0;
    for (int l = j; l <= n - 1; ++l) {
      prod *= ((l * lambda) / mu_l[l]);
      if (std::isinf(prod) || prod > 1e100) {
        return std::numeric_limits<double>::infinity();
      }
    }
    double term = (1.0 / (j * lambda)) * prod;
    if (std::isinf(term) || term > 1e100) {
      return std::numeric_limits<double>::infinity();
    }
    sum += term;
    if (std::isinf(sum)) {
      return std::numeric_limits<double>::infinity();
    }
  }
  return sum;
}

// Функция для вычисления T по правильной формуле
double compute_T(int N, int n, double lambda, const std::vector<double> &mu_l) {
  if (n == 1) {
    return 1.0 / mu_l[0];
  }

  // Первое слагаемое: (1/μ₀) * ∏_{l=1}^{n-1} (λ·l / μ_l)
  double term1 = 1.0 / mu_l[0];
  for (int l = 1; l <= n - 1; ++l) {
    term1 *= (lambda * l) / mu_l[l];
  }

  // Второе слагаемое: Σ_{j=1}^{n-1} (1/(jλ)) * ∏_{l=j}^{n-1} (λ·l / μ_l)
  double term2 = 0.0;
  for (int j = 1; j <= n - 1; ++j) {
    double prod = 1.0;
    for (int l = j; l <= n - 1; ++l) {
      prod *= (lambda * l) / mu_l[l];
    }
    term2 += (1.0 / (j * lambda)) * prod;
  }

  return term1 + term2;
}
// Функция для вычисления Theta
double compute_Theta(int N, int n, double lambda, const std::vector<double> &mu_l) {
  double sum = sum_theta(N, n, lambda, mu_l);
  if (std::isinf(sum)) {
    return std::numeric_limits<double>::infinity();
  }
  return sum + 1.0 / (n * lambda);
}

// Функция для построения mu_l
std::vector<double> build_mu_l(int N, int m, double mu) {
  std::vector<double> mu_l(N + 1);
  for (int l = 0; l <= N; ++l) {
    if (l < N - m) {
      mu_l[l] = m * mu;
    } else {
      mu_l[l] = (N - l) * mu;
    }
  }
  return mu_l;
}

// Функция для сохранения данных в файл
void save_data(const std::string &filename, const std::vector<int> &n_vals, const std::vector<double> &results) {
  std::ofstream out(filename);
  if (!out.is_open()) {
    std::cerr << "Error: Cannot open file " << filename << std::endl;
    return;
  }
  out << std::scientific << std::setprecision(10);
  for (size_t i = 0; i < n_vals.size(); ++i) {
    if (std::isinf(results[i])) {
      out << n_vals[i] << " NaN\n";
      std::cout << "Warning: Infinity detected for " << filename << " at n=" << n_vals[i] << std::endl;
    } else if (results[i] == 0.0) {
      out << n_vals[i] << " 0.0\n";
    } else {
      out << n_vals[i] << " " << results[i] << "\n";
    }
  }
  out.close();
  std::cout << "Saved: " << filename << " (" << n_vals.size() << " points)" << std::endl;
}

int main() {
  std::cout << "=== Starting calculations for Laboratory Work 1 ===" << std::endl;

  // ===================== ЗАДАНИЕ 2.1 =====================
  std::cout << "\n--- Task 2.1: Theta(n) for different mu ---" << std::endl;
  int N = 65536;
  double lambda = 1e-5;
  int m = 1;
  std::vector<double> mu_vals = {1, 10, 100, 1000};
  std::vector<int> n_vals;
  for (int n = 65527; n <= N; ++n) {
    n_vals.push_back(n);
  }
  std::cout << "Computing for " << n_vals.size() << " values of n from " << n_vals.front() << " to " << n_vals.back()
            << std::endl;

  for (double mu : mu_vals) {
    std::cout << "  mu = " << mu << std::endl;
    std::vector<double> theta_vals;
    auto mu_l = build_mu_l(N, m, mu);
    for (int n : n_vals) {
      double theta = compute_Theta(N, n, lambda, mu_l);
      theta_vals.push_back(theta);
    }
    std::string filename = "data_theta_n_mu_" + std::to_string((int)mu) + ".dat";
    save_data(filename, n_vals, theta_vals);
  }

  // ===================== ЗАДАНИЕ 2.2 =====================
  std::cout << "\n--- Task 2.2: Theta(n) for different lambda ---" << std::endl;
  std::vector<double> lambda_vals = {1e-5, 1e-6, 1e-7, 1e-8, 1e-9};
  double mu = 1.0;
  m = 1;
  auto mu_l_fixed = build_mu_l(N, m, mu);

  for (double lambda_val : lambda_vals) {
    std::cout << "  lambda = " << lambda_val << std::endl;
    std::vector<double> theta_vals;
    for (int n : n_vals) {
      double theta = compute_Theta(N, n, lambda_val, mu_l_fixed);
      theta_vals.push_back(theta);
    }
    std::string lambda_str = format_lambda_filename(lambda_val);
    std::string filename = "data_theta_n_lambda_" + lambda_str + ".dat";
    save_data(filename, n_vals, theta_vals);
  }

  // ===================== ЗАДАНИЕ 2.3 =====================
  std::cout << "\n--- Task 2.3: Theta(n) for different m ---" << std::endl;
  std::vector<int> m_vals = {1, 2, 3, 4};
  lambda = 1e-5;
  mu = 1.0;

  for (int m_val : m_vals) {
    std::cout << "  m = " << m_val << std::endl;
    std::vector<double> theta_vals;
    auto mu_l_m = build_mu_l(N, m_val, mu);
    for (int n : n_vals) {
      double theta = compute_Theta(N, n, lambda, mu_l_m);
      theta_vals.push_back(theta);
    }
    std::string filename = "data_theta_n_m_" + std::to_string(m_val) + ".dat";
    save_data(filename, n_vals, theta_vals);
  }

  // ===================== ЗАДАНИЕ 3.1 =====================
  std::cout << "\n--- Task 3.1: T(n) for different mu ---" << std::endl;
  N = 1000;
  lambda = 1e-3;
  m = 1;
  std::vector<double> mu_vals_T = {1, 2, 4, 6};
  std::vector<int> n_vals_T;
  for (int n = 900; n <= 1000; n += 10) {
    n_vals_T.push_back(n);
  }
  std::cout << "Computing for " << n_vals_T.size() << " values of n from " << n_vals_T.front() << " to "
            << n_vals_T.back() << std::endl;

  for (double mu_val : mu_vals_T) {
    std::cout << "  mu = " << mu_val << std::endl;
    std::vector<double> T_vals;
    auto mu_l_T = build_mu_l(N, m, mu_val);

    std::cout << "    mu_l[0] = " << mu_l_T[0] << ", mu_l[500] = " << mu_l_T[500] << ", mu_l[999] = " << mu_l_T[999]
              << std::endl;

    for (int n : n_vals_T) {
      double T = compute_T(N, n, lambda, mu_l_T);
      T_vals.push_back(T);
    }
    std::string filename = "data_T_n_mu_" + std::to_string((int)mu_val) + ".dat";
    save_data(filename, n_vals_T, T_vals);
  }

  // ===================== ЗАДАНИЕ 3.2 =====================
  std::cout << "\n--- Task 3.2: T(n) for different lambda ---" << std::endl;
  N = 8192;
  mu = 1.0;
  m = 1;
  std::vector<int> n_vals_T2;
  for (int n = 8092; n <= N; n += 10) {
    n_vals_T2.push_back(n);
  }
  std::cout << "Computing for " << n_vals_T2.size() << " values of n from " << n_vals_T2.front() << " to "
            << n_vals_T2.back() << std::endl;
  auto mu_l_T2 = build_mu_l(N, m, mu);

  for (double lambda_val : lambda_vals) {
    std::cout << "  lambda = " << lambda_val << std::endl;
    std::vector<double> T_vals;
    for (int n : n_vals_T2) {
      double T = compute_T(N, n, lambda_val, mu_l_T2);
      T_vals.push_back(T);
    }
    std::string lambda_str = format_lambda_filename(lambda_val);
    std::string filename = "data_T_n_lambda_" + lambda_str + ".dat";
    save_data(filename, n_vals_T2, T_vals);
  }

  // ===================== ЗАДАНИЕ 3.3 =====================
  std::cout << "\n--- Task 3.3: T(n) for different m ---" << std::endl;
  lambda = 1e-5;
  mu = 1.0;
  m_vals = {1, 2, 3, 4};

  for (int m_val : m_vals) {
    std::cout << "  m = " << m_val << std::endl;
    std::vector<double> T_vals;
    auto mu_l_T3 = build_mu_l(N, m_val, mu);
    for (int n : n_vals_T2) {
      double T = compute_T(N, n, lambda, mu_l_T3);
      T_vals.push_back(T);
    }
    std::string filename = "data_T_n_m_" + std::to_string(m_val) + ".dat";
    save_data(filename, n_vals_T2, T_vals);
  }

  std::cout << "\n=== All calculations completed! ===" << std::endl;
  std::cout << "Run 'gnuplot plot.gp' to generate all graphs." << std::endl;

  return 0;
}
