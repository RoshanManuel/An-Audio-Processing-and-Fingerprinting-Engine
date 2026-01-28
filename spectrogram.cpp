/*
#include "Spectrogram.h"
#include <cmath>
#include<iostream>
#include <cstdint>
#include <fstream>
#include <vector>
std::vector<std::vector<double>> SpectroCalc(const std::vector<std::vector<cf>> &fftframes) {
    std::vector<std::vector<double> > spectrogram;
    spectrogram.reserve(fftframes.size());
    long long limit = fftframes[0].size() / 2 + 1;
    for (const auto &frame: fftframes) {
        std::vector<double> plot(limit);
        for (long long k = 0; k < limit; ++k) {
            const cf fram = frame[k];
            const double re = static_cast<double>(fram.real);
            const double im = static_cast<double>(fram.imag);
            plot[k] = std::sqrt(re * re + im * im);
        }
        spectrogram.push_back(plot);
    }
    return spectrogram;
}

std::vector<std::vector<double>> magnitudeConvert(const std::vector<std::vector<double>>& spectrogram) {
    std::vector<std::vector<double>> magniDB;
    magniDB.reserve(spectrogram.size());
    for (const auto& frame : spectrogram) {
        std::vector<double> plot;
        plot.reserve(frame.size());
        for (double val:frame) {
            plot.push_back((20*log10(std::max(val,1e-10))));
        }
        magniDB.push_back(plot);
    }
    return magniDB;
}

std::vector<std::vector<uint8_t>> normalize(const std::vector<std::vector<float>>& magniLog) {
     float minDb = std::numeric_limits<float>::max();
     float maxDb = std::numeric_limits<float>::lowest();

    for (const auto& frame : magniLog) {
        for (const auto& val:frame) {

            minDb = std::min(val,minDb);
            maxDb = std::max(val,maxDb);
        }
    }
    std::vector<std::vector<uint8_t>> normalisedMap;
    normalisedMap.reserve(magniLog.size());
    for (const auto& frame : magniLog) {
        std::vector<uint8_t> normFrame;
        normFrame.reserve(frame.size());
        for (const auto& val:frame) {
            uint8_t data = static_cast<uint8_t>(255*(val-minDb)/(maxDb-minDb));
            normFrame.push_back(data);
        }
        normalisedMap.push_back(normFrame);
    }
    return normalisedMap;
}
RGB jet(const uint8_t value) {
    RGB color;
    const float t = value / 255.0f;

    float r = 1.5f - 4.0f * std::abs(t - 0.75f);
    float g = 1.5f - 4.0f * std::abs(t - 0.5f);
    float b = 1.5f - 4.0f * std::abs(t - 0.25f);

    color.red = static_cast<uint8_t>(std::max(0.0f, std::min(1.0f, r)) * 255);
    color.green = static_cast<uint8_t>(std::max(0.0f, std::min(1.0f, g)) * 255);
    color.blue = static_cast<uint8_t>(std::max(0.0f, std::min(1.0f, b)) * 255);

    return color;
}

bool writePPM(const std::vector<std::vector<uint8_t>>& heatmap,const char* filename) {
    if (heatmap.empty() || heatmap[0].empty()) {
        std::cerr << "writePPM: heatmap is empty" << std::endl;
        return false;
    }
    std::ofstream outfile(filename,std::ios::binary);
    if (!outfile.is_open()) {
        std::cerr << "Could not open file " << filename << std::endl;
        return false;
    }
    const int width = heatmap.size();
    const int height = heatmap[0].size();
    outfile<<"P6\n";
    outfile<<width<<" "<<height<<"\n";
    outfile<<"255\n";
    for (int y = height-1;y>=0;--y) {
        for (int x=0;x<width;++x) {
            uint8_t data = heatmap[x][y];
            auto [red, green, blue]{jet(data)};
            outfile.write(reinterpret_cast<const char*>(&red),1);
            outfile.write(reinterpret_cast<const char*>(&green),1);
            outfile.write(reinterpret_cast<const char*>(&blue),1);
        }
    }
    outfile.close();
    return true;
}
*/
