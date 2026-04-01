#!/usr/bin/gnuplot

# Устанавливаем стиль вывода
set terminal pngcairo enhanced font 'arial,12' size 1024,768
set grid
set key outside right top
set datafile missing "NaN"

# ===================== ЗАДАНИЕ 2.1 =====================
set output 'graph_theta_n_mu.png'
set title 'Зависимость Θ(n) для разных μ (N=65536, λ=10^{-5}, m=1)'
set xlabel 'n'
set ylabel 'Θ, часы'
set xrange [65527:65536]
set logscale y
plot 'data_theta_n_mu_1.dat' using 1:2 with linespoints title 'μ=1', \
     'data_theta_n_mu_10.dat' using 1:2 with linespoints title 'μ=10', \
     'data_theta_n_mu_100.dat' using 1:2 with linespoints title 'μ=100', \
     'data_theta_n_mu_1000.dat' using 1:2 with linespoints title 'μ=1000'

# ===================== ЗАДАНИЕ 2.2 =====================
set output 'graph_theta_n_lambda.png'
set title 'Зависимость Θ(n) для разных λ (N=65536, μ=1, m=1)'
set xlabel 'n'
set ylabel 'Θ, часы'
set xrange [65527:65536]
set logscale y
plot 'data_theta_n_lambda_1e-05.dat' using 1:2 with linespoints title 'λ=10^{-5}', \
     'data_theta_n_lambda_1e-06.dat' using 1:2 with linespoints title 'λ=10^{-6}', \
     'data_theta_n_lambda_1e-07.dat' using 1:2 with linespoints title 'λ=10^{-7}', \
     'data_theta_n_lambda_1e-08.dat' using 1:2 with linespoints title 'λ=10^{-8}', \
     'data_theta_n_lambda_1e-09.dat' using 1:2 with linespoints title 'λ=10^{-9}'

# ===================== ЗАДАНИЕ 2.3 =====================
set output 'graph_theta_n_m.png'
set title 'Зависимость Θ(n) для разных m (N=65536, μ=1, λ=10^{-5})'
set xlabel 'n'
set ylabel 'Θ, часы'
set xrange [65527:65536]
set logscale y
plot 'data_theta_n_m_1.dat' using 1:2 with linespoints title 'm=1', \
     'data_theta_n_m_2.dat' using 1:2 with linespoints title 'm=2', \
     'data_theta_n_m_3.dat' using 1:2 with linespoints title 'm=3', \
     'data_theta_n_m_4.dat' using 1:2 with linespoints title 'm=4'

# ===================== ЗАДАНИЕ 3.1 =====================
set output 'graph_T_n_mu.png'
set title 'Зависимость T(n) для разных μ (N=1000, λ=10^{-3}, m=1)'
set xlabel 'n'
set ylabel 'T, часы'
set xrange [900:1000]
set yrange [0:*]
set logscale y
plot 'data_T_n_mu_1.dat' using 1:2 with linespoints title 'μ=1', \
     'data_T_n_mu_2.dat' using 1:2 with linespoints title 'μ=2', \
     'data_T_n_mu_4.dat' using 1:2 with linespoints title 'μ=4', \
     'data_T_n_mu_6.dat' using 1:2 with linespoints title 'μ=6'

# ===================== ЗАДАНИЕ 3.2 =====================
set output 'graph_T_n_lambda.png'
set title 'Зависимость T(n) для разных λ (N=8192, μ=1, m=1)'
set xlabel 'n'
set ylabel 'T, часы'
set xrange [8092:8192]
set logscale y
plot 'data_T_n_lambda_1e-05.dat' using 1:2 with linespoints title 'λ=10^{-5}', \
     'data_T_n_lambda_1e-06.dat' using 1:2 with linespoints title 'λ=10^{-6}', \
     'data_T_n_lambda_1e-07.dat' using 1:2 with linespoints title 'λ=10^{-7}', \
     'data_T_n_lambda_1e-08.dat' using 1:2 with linespoints title 'λ=10^{-8}', \
     'data_T_n_lambda_1e-09.dat' using 1:2 with linespoints title 'λ=10^{-9}'

# ===================== ЗАДАНИЕ 3.3 =====================
set output 'graph_T_n_m.png'
set title 'Зависимость T(n) для разных m (N=8192, μ=1, λ=10^{-5})'
set xlabel 'n'
set ylabel 'T, часы'
set xrange [8092:8192]
set logscale y
plot 'data_T_n_m_1.dat' using 1:2 with linespoints title 'm=1', \
     'data_T_n_m_2.dat' using 1:2 with linespoints title 'm=2', \
     'data_T_n_m_3.dat' using 1:2 with linespoints title 'm=3', \
     'data_T_n_m_4.dat' using 1:2 with linespoints title 'm=4'

# Дополнительные графики с линейным масштабом для лучшего понимания
set terminal pngcairo enhanced font 'arial,12' size 1024,768

# Линейный масштаб для T (задание 3.1)
set output 'graph_T_n_mu_linear.png'
set title 'Зависимость T(n) для разных μ (линейный масштаб)'
set xlabel 'n'
set ylabel 'T, часы'
set xrange [900:1000]
unset logscale y
plot 'data_T_n_mu_1.dat' using 1:2 with linespoints title 'μ=1', \
     'data_T_n_mu_2.dat' using 1:2 with linespoints title 'μ=2', \
     'data_T_n_mu_4.dat' using 1:2 with linespoints title 'μ=4', \
     'data_T_n_mu_6.dat' using 1:2 with linespoints title 'μ=6'

# Линейный масштаб для T (задание 3.2) - для лучшего сравнения
set output 'graph_T_n_lambda_linear.png'
set title 'Зависимость T(n) для разных λ (линейный масштаб)'
set xlabel 'n'
set ylabel 'T, часы'
set xrange [8092:8192]
unset logscale y
plot 'data_T_n_lambda_1e-05.dat' using 1:2 with linespoints title 'λ=10^{-5}', \
     'data_T_n_lambda_1e-06.dat' using 1:2 with linespoints title 'λ=10^{-6}', \
     'data_T_n_lambda_1e-07.dat' using 1:2 with linespoints title 'λ=10^{-7}', \
     'data_T_n_lambda_1e-08.dat' using 1:2 with linespoints title 'λ=10^{-8}', \
     'data_T_n_lambda_1e-09.dat' using 1:2 with linespoints title 'λ=10^{-9}'

print "All graphs have been generated successfully!"
