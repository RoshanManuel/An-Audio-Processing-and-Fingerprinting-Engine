#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include <cstring>
#include <complex>
#include <chrono>
#include "downsampler.h"
static_assert(true);
#pragma pack(push,1)
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

bool writeWAV(const char* outputFilename,const std::vector<float>& pcm,const uint16_t bps,uint32_t samr) {
    std::ofstream file(outputFilename,std::ios::binary);
    if (!file.is_open()) {
        std::cerr<< "Error opening file\n";
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
        std::cerr<<"Unsupported bits per sample\n";
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
    for (float sample:pcm) {
        sample = std::max(-1.0f,std::min(1.0f,sample));
        auto data = static_cast<int16_t>(sample*32767);
        file.write(reinterpret_cast<const char*>(&data),sizeof(int16_t));
    }
    }
    else if (bps==8){
        for (float sample:pcm) {
            sample = std::max(-1.0f,std::min(1.0f,sample));
            auto data = static_cast<uint8_t>(sample*127.5 + 127.5);
            file.write(reinterpret_cast<const char*>(&data),sizeof(uint8_t));
        }
    }
    return true;
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    wav_hdr mywav;
    constexpr size_t headerSize = sizeof(wav_hdr);
    const char *filename = "C:\\Audio_Spectrogram\\samples\\sample2.wav";
    std::ifstream file(filename,std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file\n";
        return 1;
    }
    file.read(reinterpret_cast<char*>(&mywav),sizeof(wav_hdr));
    if (const bool success = file.good() && file.gcount()==headerSize; !success) {
        if (file.eof()) {
            std::cerr << "EOF reached!\n";
            return 1;
        }
        else if (file.bad()) {
            std::cerr << "Error reading file\n";
            return 1;
        }
        else {
            std::cerr << "Incomplete read of only " << file.gcount() << " bytes out of " << headerSize << "\n";
            return 1;
        }
    }

    if (strncmp(mywav.riff,"RIFF",4)!=0 || strncmp(mywav.format,"WAVE",4)!=0 || strncmp(mywav.subchunk1_id,"fmt ",4)!=0 || strncmp(mywav.subchunk2_id,"data",4)!=0) {
        std::cerr << "Incomplete wav signature\n";
        return 1;
    }
    if (mywav.audio_format!=1) {
        std::cerr << "Not PCM(1) format\n";
        return 1;
    }
    uint32_t dataSize = mywav.subchunk2_size;
    uint16_t numChannels = mywav.num_channels;
    uint32_t sampleRate = mywav.sample_rate;
    uint16_t blockAlign = mywav.block_align;
    uint16_t bitsPerSample = mywav.bits_per_sample;
    const long long number_frames = dataSize / blockAlign;
    std::cout<<"The audio format is: " << mywav.audio_format<<"\n";
    std::cout<<"The number of channels is: "<<numChannels<<"\n";
    std::cout<<"The block align is: "<<blockAlign<<"\n";
    std::cout<<"The sample rate is: "<<sampleRate<<"\n";
    std::cout<<"The data size is: "<< dataSize<<"\n";
    std::cout<<"The bits/sample is: " << bitsPerSample<<"\n";
    std::cout<<"The duration is: " << number_frames/mywav.sample_rate<<"s"<<"\n";

    std::vector<float> normalisedPCM;
    normalisedPCM.reserve(number_frames*numChannels);
    file.seekg(headerSize,std::ios::beg);
    if (bitsPerSample==8) {
      std::vector<uint8_t> raw(dataSize);
        file.read(reinterpret_cast<char*>(raw.data()), dataSize);
        if (file.gcount() != number_frames * blockAlign) {
            std::cerr << "Incomplete PCM data to be read\n";
            return 1;
        }
        for (const uint8_t sample:raw) {
                normalisedPCM.push_back((sample-128.0f)/128.0f);
        }
    }
    else if (bitsPerSample==16) {
        std::vector<int16_t> raw(dataSize/sizeof(int16_t));
        file.read(reinterpret_cast<char*>(raw.data()), dataSize);
        if (file.gcount()!=number_frames*blockAlign) {
            std::cerr << "Incomplete PCM data to be read\n";
            return 1;
        }
        for (const int16_t sample:raw) {
            normalisedPCM.push_back(sample*1.0f/32768.0f);
        }
    }
    else {
        std::cerr << "Invalid bits/sample\n";
        return 1;
    }

    constexpr int originalRate = 44100;
    constexpr int targetRate = 11025; // 1/4th
    constexpr int filterlength = 65;
    constexpr int cutoff = 5000; //from Nyquist theorem(<=5512.5)
    constexpr int Ratio = originalRate/targetRate;
    std::vector<float> downsampledPCM;
    downsampledPCM.reserve(number_frames);
    if (numChannels==1) {
        auto lowpass = lowpassfir(cutoff,originalRate,filterlength);
        downsampledPCM = polyphaseDecimate(normalisedPCM,lowpass,Ratio);
    }
    else if (numChannels==2) {
        std::vector<float> mono;
        mono.reserve(number_frames);
        for (size_t i=0;i<number_frames;i++) {
            mono.push_back(0.5f*(normalisedPCM[2*i]+normalisedPCM[2*i+1]));
        }
        auto lowpass = lowpassfir(cutoff,originalRate,filterlength);
        downsampledPCM = polyphaseDecimate(mono,lowpass,Ratio);
    }
    else {
        std::cerr<<"Only mono and stereo audio formats supported!\n";
        return 1;
    }
    /*
    const char* outputWAV = "C:\\Audio_Spectrogram\\decimated\\sample2leooo.wav";
    //changing downsampledPCM to normal PCM and target rate to 11025
    if (writeWAV(outputWAV,downsampledPCM,bitsPerSample,targetRate))
        std::cout<<"Decimated file written successfully\n";
    else {
        std::cerr<<"Failure to write wav file\n";
        return 1;
    }
    */
    /*Spectrogram section*/
    //windowSize in global namespace
    constexpr int hopSize = windowSize/2; //50%
    std::vector<std::vector<float>> Powerframes = signalFramer(downsampledPCM,windowSize,hopSize);
    if (Powerframes.empty()) {
        std::cerr<<"No Powerframes found!\n";
        return 1;
    }
    for (auto& frame : Powerframes) {
        for (float& p : frame) {
            p = 10.0f * std::log10f(p + 1e-10f);
        }
    }
    /*
    std::vector<std::vector<double>> spec = SpectroCalc(STFTframes);
    if (spec.empty()) {
        std::cerr<<"No spectrogram found!\n";
        return 1;
    }
    std::vector<std::vector<double>> magnitude = magnitudeConvert(spec);
    if (magnitude.empty()) {
        std::cerr<<"No magnitude found!\n";
        return 1;
    }

    std::vector<std::vector<uint8_t>> normalised = normalize(Powerframes);
    if (normalised.empty()) {
        std::cerr<<"No normalised magnitude found!\n";
        return 1;
    }
    if (writePPM(normalised,"C:\\Audio_Spectrogram\\spectrogram\\sample2leoo.ppm"))
    {
        std::cout<<"PPM File written successfully\n";
    }
    else {
        std::cerr<<"Failure to write ppm file\n";
        return 1;
    }
    */
    auto stop = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(stop-start);
    std::cout<<"The time elapsed is: "<<elapsed.count()/1000000.0<<"s\n";
    return 0;
}