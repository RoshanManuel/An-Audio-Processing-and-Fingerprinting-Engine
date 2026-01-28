#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include <cstring>
#include <complex>
#include <time.h>
#include <stdlib.h>
#include "downsampler.h"
#include "Spectrogram.h"
static_assert(true);
#pragma pack(push,1)
using cd = std::complex<double>;
typedef struct wav_header {
    // RIFF CHUNK
    char riff[4];
    uint32_t chunkSize;
    char format[4];
    //FMT SUBCHUNK
    char subchunk1_id[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    //DATA
    char subchunk2_id[4];
    uint32_t subchunk2_size;
}wav_hdr;
#pragma pack(pop)

bool writeWAV(const char* outputFilename,const std::vector<double>& pcm,const uint16_t bps,uint32_t samr) {
    std::ofstream file(outputFilename,std::ios::binary);
    if (!file.is_open()) {
        std::cerr<< "Error opening file" << std::endl;
        return false;
    }
    wav_header header{};
    uint32_t dataSize{};
    if (bps==16) {
        dataSize = pcm.size() * sizeof(int16_t);
    }
    else if (bps==8) {
        dataSize = pcm.size() * sizeof(uint8_t);
    }
    else {
        std::cerr<<"Unsupported bits per sample"<<std::endl;
        return false;
    }
    memcpy(header.riff,"RIFF",4);
    header.chunkSize = 36+dataSize;
    memcpy(header.format,"WAVE",4);
    memcpy(header.subchunk1_id,"fmt ",4);
    header.subchunk1_size = 16;
    header.audio_format = 1;
    header.num_channels = 1;
    header.sample_rate = samr;
    header.byte_rate = samr * 1 * (bps/8); //num is the middle term so i'm leaving it one since i'm taking mono
    header.block_align = 1 * (bps/8); // same as above
    header.bits_per_sample = bps;
    memcpy(header.subchunk2_id,"data",4);
    header.subchunk2_size = dataSize;
    file.write(reinterpret_cast<const char*>(&header),sizeof(wav_hdr));

    if (bps==16) {
    for (double sample:pcm) {
        sample = std::max(-1.0,std::min(1.0,sample));
        auto data = static_cast<int16_t>(sample*32767);
        file.write(reinterpret_cast<const char*>(&data),sizeof(int16_t));
    }
    }
    else if (bps==8){
        for (double sample:pcm) {
            sample = std::max(-1.0,std::min(1.0,sample));
            auto data = static_cast<uint8_t>(sample*127.5 + 127.5);
            file.write(reinterpret_cast<const char*>(&data),sizeof(uint8_t));
        }
    }
    return true;
}

int main() {
    clock_t start = clock();
    wav_hdr mywav;
    constexpr size_t headerSize = sizeof(wav_hdr);
    const char *filename = "C:\\Audio_Spectrogram\\samples\\sample2.wav";
    std::ifstream file(filename,std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }
    file.read(reinterpret_cast<char*>(&mywav),sizeof(wav_hdr));
    if (const bool success = file.good() && file.gcount()==headerSize; !success) {
        if (file.eof()) {
            std::cerr << "EOF reached!" << std::endl;
            return 1;
        }
        else if (file.bad()) {
            std::cerr << "Error reading file" << std::endl;
            return 1;
        }
        else {
            std::cerr << "Incomplete read of only " <<file.gcount() << " bytes out of " << headerSize << std::endl;
            return 1;
        }
    }

    if (strncmp(mywav.riff,"RIFF",4)!=0 || strncmp(mywav.format,"WAVE",4)!=0 || strncmp(mywav.subchunk1_id,"fmt ",4)!=0 || strncmp(mywav.subchunk2_id,"data",4)!=0) {
        std::cerr << "Incomplete wav signature" << std::endl;
        return 1;
    }
    if (mywav.audio_format!=1) {
        std::cerr << "Not PCM(1) format" << std::endl;
        return 1;
    }
    uint32_t dataSize = mywav.subchunk2_size;
    uint16_t numChannels = mywav.num_channels;
    uint32_t sampleRate = mywav.sample_rate;
    uint16_t blockAlign = mywav.block_align;
    uint16_t bitsPerSample = mywav.bits_per_sample;
    const long long number_frames = dataSize / blockAlign;
    std::cout<<"The audio format is: " << mywav.audio_format<<std::endl;
    std::cout<<"The number of channels is: "<<numChannels<<std::endl;
    std::cout<<"The block align is: "<<blockAlign<<std::endl;
    std::cout<<"The sample rate is: "<<sampleRate<<std::endl;
    std::cout<<"The data size is: "<< dataSize<<std::endl;
    std::cout<<"The bits/sample is: " << bitsPerSample<<std::endl;
    std::cout<<"The duration is: " << number_frames/mywav.sample_rate<<"s"<<std::endl;

    std::vector<double> normalisedPCM;
    normalisedPCM.reserve(number_frames*numChannels);
    file.seekg(headerSize,std::ios::beg);
    if (bitsPerSample==8) {
      std::vector<uint8_t> raw(dataSize);
        file.read(reinterpret_cast<char*>(raw.data()), dataSize);
        if (file.gcount() != number_frames * blockAlign) {
            std::cerr << "Incomplete PCM data to be read" << std::endl;
            return 1;
        }
        for (const uint8_t sample:raw) {
                normalisedPCM.push_back((static_cast<double>(sample)-127.5)/127.5);
        }
    }
    else if (bitsPerSample==16) {
        std::vector<int16_t> raw(dataSize/sizeof(int16_t));
        file.read(reinterpret_cast<char*>(raw.data()), dataSize);
        if (file.gcount()!=number_frames*blockAlign) {
            std::cerr << "Incomplete PCM data to be read" << std::endl;
            return 1;
        }
        for (const int16_t sample:raw) {
            normalisedPCM.push_back(sample*1.0/32767);
        }
    }
    else {
        std::cerr << "Invalid bits/sample " << std::endl;
        return 1;
    }

    int originalRate = 44100;
    int targetRate = 11025; // 1/4th
    int filterlength = 65;
    int cutoff = 5000; //from Nyquist theorem(<=5512.5)
    int Ratio = originalRate/targetRate;
    std::vector<double> downsampledPCM;
    downsampledPCM.reserve(number_frames);
    if (numChannels==1) {
        auto lowpass = lowpassfir(cutoff,originalRate,filterlength);
        downsampledPCM = polyphaseDecimate(normalisedPCM,lowpass,Ratio);
    }
    else if (numChannels==2) {
        std::vector<double> mono;
        mono.reserve(number_frames);
        for (size_t i=0;i<number_frames;i++) {
           double left = (normalisedPCM[2*i]);
            double right = (normalisedPCM[2*i+1]);
            mono.push_back(left/2.0+right/2.0);
        }
        auto lowpass = lowpassfir(cutoff,originalRate,filterlength);
        downsampledPCM = polyphaseDecimate(mono,lowpass,Ratio);
    }
    else {
        std::cerr<<"Only mono and stereo audio formats supported!"<<std::endl;
        return 1;
    }

    const char* outputWAV = "C:\\Audio_Spectrogram\\decimated\\sample2decimated.wav";
    //changing downsampledPCM to normal PCM and target rate to 11025
    if (writeWAV(outputWAV,downsampledPCM,bitsPerSample,targetRate))
        std::cout<<"Decimated file written successfully"<<std::endl;
    else {
        std::cerr<<"Failure to write wav file"<<std::endl;
        return 1;
    }

    /*Spectrogram section*/

    int windowSize = 1024;
    int hopSize = windowSize/2;
    std::vector<std::vector<cd>> STFTframes = signalFramer(downsampledPCM,windowSize,hopSize);
    if (STFTframes.empty()) {
        std::cerr<<"No STFTframes found!"<<std::endl;
        return 1;
    }
    std::vector<std::vector<double>> spec = SpectroCalc(STFTframes);
    if (spec.empty()) {
        std::cerr<<"No spectrogram found!"<<std::endl;
        return 1;
    }
    std::vector<std::vector<double>> magnitude = magnitudeConvert(spec);
    if (magnitude.empty()) {
        std::cerr<<"No magnitude found!"<<std::endl;
        return 1;
    }
    std::vector<std::vector<uint8_t>> normalised = normalize(magnitude);
    if (normalised.empty()) {
        std::cerr<<"No normalised magnitude found!"<<std::endl;
        return 1;
    }
    if (writePPM(normalised,"C:\\Audio_Spectrogram\\spectrogram\\sample2.ppm"))
    {
        std::cout<<"PPM File written successfully"<<std::endl;
    }
    else {
        std::cerr<<"Failure to write ppm file"<<std::endl;
        return 1;
    }
    clock_t stop = clock();
    double elapsed = static_cast<double>(stop-start)/CLOCKS_PER_SEC;
    std::cout<<"The time elapsed is: "<<elapsed<<std::endl;
    return 0;
}
