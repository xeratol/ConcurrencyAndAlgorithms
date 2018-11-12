#pragma once

#include <vector>
#include <complex>

#include "matrix.h"

bool isEqual(double a, double b, double epsilon = std::numeric_limits<double>::epsilon());
unsigned int RoundUpPowerOf2(unsigned int v);
std::vector<unsigned int> GenBitReversal(unsigned int size);
std::vector<std::complex<double>> GenTwiddleFactors(unsigned int halfN);
std::vector<std::complex<double>> ApplyButterfly(const std::vector<std::complex<double>>& data, const std::vector<std::complex<double>>& twiddle);

std::vector<std::complex<double>> FFT(const std::vector<std::complex<double>>& data, const std::vector<std::complex<double>>& twiddle);
matrix<std::complex<double>> FFT(const matrix<std::complex<double>>& data, matrix<std::complex<double>>& intermediate);
