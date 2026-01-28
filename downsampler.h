#ifndef DOWNSAMPLER_H
#define DOWNSAMPLER_H
#include <vector>
#include <complex>
using cd = std::complex<double>;
std::vector<double> lowpassfir(int,int,int);
std::vector<double> polyphaseDecimate(const std::vector<double>&,const std::vector<double>&,int);
std::vector<double> generateHannWindow(int);
void iterativeFFT(std::vector<cd>&);
std::vector<std::vector<cd>> signalFramer(const std::vector<double>&,const int,const int);
#endif
