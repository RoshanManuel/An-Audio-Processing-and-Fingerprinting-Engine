#ifndef SHAZAM_SPECTROGRAM_H
#define SHAZAM_SPECTROGRAM_H
#include <cstdint>
#include <vector>
#include <complex>
using cd = std::complex<double>;
typedef struct {
    uint8_t red, green, blue;
}RGB;
std::vector<std::vector<double>> SpectroCalc(const std::vector<std::vector<cd>>&);
std::vector<std::vector<double>> magnitudeConvert(const std::vector<std::vector<double>>&);
std::vector<std::vector<uint8_t>> normalize(const std::vector<std::vector<double>>&);
RGB jet(const uint8_t value);
bool writePPM(const std::vector<std::vector<uint8_t>>& ,const char*);
#endif
