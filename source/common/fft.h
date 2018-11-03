#pragma once

#include <vector>
#include <complex>

#include "matrix.h"

std::vector<std::complex<double>> FFT(const std::vector<std::complex<double>>& data);
matrix<std::complex<double>> FFT(const matrix<std::complex<double>>& data, matrix<std::complex<double>>& intermediate);
