#include <iostream>
#include <cmath>
#include <vector>
#include <array>
#include "downsampler.h"
#include <complex>
constexpr cf twiddle(const int k,const int N) {
    const float angle = -2.0f * PI * k /N;
    return {
    __builtin_cosf(angle),__builtin_sinf(angle)};
}
inline constexpr std::array<std::array<cf,levels>,stages> buildTable() {
    std::array<std::array<cf,levels>,stages> table{};
    for (int i=0;i<stages;i++) {
        const int N = 1 << (i+1);
        for (int j=0;j<N/2;j++) {
            table[i][j] = twiddle(j,N);
        }
    }
    return table;
}
static constexpr std::array<std::array<cf,levels>,stages> table = buildTable();
inline constexpr std::array<float,windowSize> generateHannWindowforSTFT() {
    std::array<float,1024> hannWindow{};
    for (int i{}; i < windowSize; ++i) {
        const float w = 0.5f*(1.0f-__builtin_cosf((2*PI*i)/(windowSize-1.0f)));
        hannWindow[i] = w;
    }
    return hannWindow;
}
static constexpr auto hannWindow1024 = generateHannWindowforSTFT();
inline constexpr std::array<float,65> generateHannWindowforLPF() {
    std::array<float,filterTaps> hannWindow{};
    for (int i{}; i <filterTaps; ++i) {
        const float w = 0.5f*(1.0f-__builtin_cosf((2*PI*i)/(filterTaps-1.0f)));
        hannWindow[i] = w;
    }
    return hannWindow;
}
static constexpr auto hannWindow65 = generateHannWindowforLPF();
std::vector<float> lowpassfir(int cutoff,int originalRate,int filLen) {
std::vector<float> h(filLen);
const float fc = (cutoff*1.0f)/originalRate;
    const int N = filLen-1;
    for (int n{};n<=N;n++) {
        const int idx = n - N/2;
        float h_ideal = (idx==0)?2*fc:(sin(2.0f*PI*fc*idx)/(PI*idx));
        h[n] = h_ideal * hannWindow65[n];
    }
    float sum = 0.0;
    for (const auto& x : h ) {
        sum += x;
    }
    for (auto& x : h ) {
        x/=sum;
    }
return h;
}
std::vector<float> polyphaseDecimate(const std::vector<float>& signal,const std::vector<float>& h,int M) {
    const int N = signal.size();
    const int P  = h.size();
    int outSize = N/M;
    std::vector<float> output(outSize,0.0);
    for (int i{}; i < outSize; ++i) {
        int n = i * M; // input index for this output sample
        float sum = 0.0;
        const int k_start = std::max(0, n - N + 1);
        const int k_end = std::min(P, n + 1);
        for (int k = k_start; k < k_end; ++k) { // Convolve:
            sum += h[k] * signal[n - k];
        }
        output[i] = sum;
    }
    return output;
}
void iterativeFFT(std::vector<cf>& signal) {
    int n = signal.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;//carry bit adder to find 0
        j ^= bit; //set that 0 to 1

        if (i < j)
            std::swap(signal[i], signal[j]);//only when not equal
    }
    int stage = 0;
    for (int len = 2;len<=n;len<<=1) {
        const cf* w = table[stage].data();
        const int half = len >> 1;
        for (int i =0;i<n;i+=len) {
            for (int j=0;j<half;j++) { //butterfly step
                const cf a = signal[i+j];
                const cf b = signal[i+j+half];
                const cf wj = w[j];
                const float c_real = b.real * wj.real - b.imag * wj.imag;
                const float c_imag  = b.real * wj.imag + b.imag * wj.real;
                signal[i+j] =  {a.real+c_real,a.imag+c_imag};
                signal[i+j+half] =  {a.real-c_real,a.imag-c_imag};
            }
        }
        stage++;
    }
}
std::vector<std::vector<float>> signalFramer(const std::vector<float>& signal,const int frameSize,const int hopSize) {

    const size_t numframes = (signal.size() - frameSize) / hopSize + 1;
    const int halfSize = frameSize / 2 +1;
    std::vector<std::vector<float>> powerSpec;
    powerSpec.reserve(numframes);
     for (int start = 0;start+frameSize<=signal.size();start+=hopSize) {
       std::vector<cf> output(frameSize,0.0f);
        for (int i = 0; i < frameSize; ++i) {
            output[i] = static_cast<float>(signal[start+i]);
        }
       for (int j = 0;j < frameSize; ++j) {
        output[j].real *= static_cast<float>(hannWindow1024[j]);
       }
         iterativeFFT(output);
         std::vector<float> power_frame(halfSize);
         for (int p=0;p<halfSize;p++) {
             const float re = output[p].real;
             const float im = output[p].imag;
             power_frame[p] = re*re+im*im;
         }
         powerSpec.push_back(power_frame);
   }
    return powerSpec;
}