#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

vector<double> build_mu_l(int N, int m, double mu) {
  vector<double> mu_l(N + 1);
  for (int l = 0; l <= N; ++l) {
    if (l == 0) {
      mu_l[l] = m * mu;
    } else if (l < m) {
      mu_l[l] = l * mu;
    } else {
      mu_l[l] = m * mu;
    }
    if (mu_l[l] == 0)
      mu_l[l] = 1e-10;
  }
  return mu_l;
}

double compute_Theta(int N, int n, double lambda, const vector<double> &mu_l) {
  if (lambda <= 0)
    return numeric_limits<double>::infinity();

  double sum = 1.0 / (n * lambda);
  double prod = 1.0;

  for (int j = n + 1; j <= N; ++j) {
    prod *= (mu_l[j - 1] / ((j - 1) * lambda));
    if (prod > 1e100)
      prod = 1e100;
    sum += (1.0 / (j * lambda)) * prod;
    if (sum > 1e100)
      return 1e100;
  }

  return sum;
}

double compute_T_fast(int n, double lambda, double mu, int m) {
  if (m == 0 || mu == 0)
    return numeric_limits<double>::infinity();

  double repair_rate = m * mu;
  double failure_rate = n * lambda;

  if (failure_rate >= repair_rate) {
    return numeric_limits<double>::infinity();
  }

  return (n / repair_rate) / (1.0 - failure_rate / repair_rate);
}

int main() {
  int N = 65536;
  vector<double> lambda_vals = {1e-7, 1e-6, 1e-5};
  vector<double> mu_vals = {1.0, 10.0, 100.0, 1000.0};
  vector<int> m_vals = {1, 2, 3};
  vector<int> n_vals;

  for (int n0 = 65527; n0 <= 65536; ++n0) {
    n_vals.push_back(n0);
  }

  cout << fixed << setprecision(10);

  for (double lambda : lambda_vals) {
    for (double mu : mu_vals) {
      for (int m : m_vals) {
        vector<double> mu_l = build_mu_l(N, m, mu);

        for (int n : n_vals) {
          double theta_N = compute_Theta(N, n, lambda, mu_l);
          double T_0 = compute_T_fast(n, lambda, mu, m);

          cout << "λ=" << lambda << " μ=" << mu << " m=" << m << " n=" << n << endl;
          cout << "θ_N = " << theta_N << endl;
          cout << "T_0 = " << T_0 << endl;
          cout << "------------------------" << endl;
        }
      }
    }
  }

  return 0;
}
