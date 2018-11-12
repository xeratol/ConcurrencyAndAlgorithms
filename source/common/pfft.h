#pragma once

#include <vector>
#include <complex>

#include "matrix.h"

matrix<std::complex<double>> PFFT(const matrix<std::complex<double>>& data,
    matrix<std::complex<double>>& intermediate);
