#ifndef SHAZAM_SPECTROGRAM_H
#define SHAZAM_SPECTROGRAM_H
#include <cstdint>
#include <vector>
#include "complexf.h"
typedef struct {
    uint8_t red, green, blue;
}RGB;
//std::vector<std::vector<double>> SpectroCalc(const std::vector<std::vector<cf>>&);
//std::vector<std::vector<double>> magnitudeConvert(const std::vector<std::vector<double>>&);
std::vector<std::vector<uint8_t>> normalize(const std::vector<std::vector<float>>&);
RGB jet(const uint8_t value);
bool writePPM(const std::vector<std::vector<uint8_t>>& ,const char*);
#endif
