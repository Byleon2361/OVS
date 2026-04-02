#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>
using namespace std;

// μ_l — интенсивность восстановления при l работоспособных ЭМ
double mu_l(int l, int N, int m, double mu) {
  int threshold = N - m;
  if (l >= threshold)
    return (double)(N - l) * mu; // меньше ЭМ на ремонте, чем устройств
  else
    return (double)m * mu; // все m устройств заняты
}

int main() {
  const int N = 65536;
  const int n_min = 65527;
  const int n_max = 65536;

  vector<double> lambda_vals = {1e-7, 1e-6, 1e-5};
  vector<double> mu_vals = {1.0, 10.0, 100.0, 1000.0};
  vector<int> m_vals = {1, 2, 3};

  cout << fixed << setprecision(10);

  for (double lambda : lambda_vals) {
    for (double mu : mu_vals) {
      for (int m : m_vals) {

        // ── Предвычисление θ_k (обратная рекуррентность) ──────────────
        // Формула (из задания):
        //   θ_k = 1/(kλ) + Σ_{j=k+1}^{N} [1/(jλ)] · Π_{l=k}^{j-1} [μ_l/(lλ)]
        //
        // Рекуррентность (эквивалентная, без двойного цикла):
        //   θ_N   = 1/(N·λ)
        //   θ_k   = 1/(k·λ) + μ_k/(k·λ) · θ_{k+1}
        //
        // Вывод: θ_{k+1} уже учитывает все переходы выше k+1,
        // поэтому достаточно одного умножения.
        //
        vector<double> theta(N + 1, 0.0);
        theta[N] = 1.0 / (N * lambda);
        for (int k = N - 1; k >= n_min; --k) {
          double mu_k = mu_l(k, N, m, mu);
          theta[k] = 1.0 / (k * lambda) + (mu_k / (k * lambda)) * theta[k + 1];
        }

        // ── Предвычисление T_k (прямая рекуррентность) ────────────────
        // Формула (из задания, при n > 1):
        //   T_n = (1/μ_0)·Π_{l=1}^{n-1}[lλ/μ_l]
        //        + Σ_{j=1}^{n-1} [1/(jλ)] · Π_{l=j}^{n-1}[lλ/μ_l]
        //
        // Прямая рекуррентность (выводится из формулы подстановкой):
        //   T_1     = 1/μ_0
        //   T_{k+1} = 1/μ_k + k·λ/μ_k · T_k
        //
        // Смысл: T_k — среднее время перехода из состояния k−1 в k
        // с учётом возможных повторных отказов.
        //
        vector<double> T(N + 1, 0.0);
        {
          double mu0 = mu_l(0, N, m, mu);
          T[1] = (mu0 > 0.0) ? 1.0 / mu0 : 0.0;
        }
        for (int k = 1; k < N; ++k) {
          double mu_k = mu_l(k, N, m, mu);
          if (mu_k > 0.0)
            T[k + 1] = 1.0 / mu_k + (k * lambda / mu_k) * T[k];
          // при k == N μ_N = 0; но k < N всегда в цикле → деление на 0 исключено
        }

        // ── Вывод результатов ─────────────────────────────────────────
        for (int n = n_min; n <= n_max; ++n) {
          cout << "λ=" << lambda << " μ=" << mu << " m=" << m << " n=" << n << "\n";

          // θ = {θ_n, θ_{n+1}, …, θ_N}  (не более 10 элементов)
          cout << "θ = {";
          for (int k = n; k <= N; ++k) {
            cout << theta[k];
            if (k < N)
              cout << ", ";
          }
          cout << "}\n";

          // T = {T_n, T_{n+1}, …, T_N}  (не более 10 элементов)
          cout << "T = {";
          for (int k = n; k <= N; ++k) {
            cout << T[k];
            if (k < N)
              cout << ", ";
          }
          cout << "}\n";

          cout << "------------------------\n";
        }
      }
    }
  }
  return 0;
}
