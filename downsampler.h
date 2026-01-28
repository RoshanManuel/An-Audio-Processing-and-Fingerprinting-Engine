#ifndef SHAZAM_DOWNSAMPLER_H
#define SHAZAM_DOWNSAMPLER_H
#include <vector>
#include <array>
#include "complexf.h"
constexpr float PI  =   3.14159265358979323846f;
constexpr int stages = 10;
constexpr int levels = 512;
constexpr int windowSize = 1024;
constexpr int filterTaps = 65;
constexpr cf twiddle(const int ,const int );
inline constexpr std::array<std::array<cf,levels>,stages> buildTable();
inline constexpr std::array<float,windowSize> generateHannWindowforSTFT();
inline constexpr std::array<float,65> generateHannWindowforLPF();
std::vector<float> lowpassfir(int,int,int);
std::vector<float> polyphaseDecimate(const std::vector<float>&,const std::vector<float>&,int);
std::vector<float> generateHannWindow(int);
void iterativeFFT(std::vector<cf>&);
std::vector<std::vector<float>> signalFramer(const std::vector<float>&,const int,const int);
#endif