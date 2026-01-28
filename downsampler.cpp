#include <iostream>
#include <cmath>
#include <vector>
#include "downsampler.h"
#include <complex>
# define PI  3.14159265358979323846
using cd  = std::complex<double>;

std::vector<double> generateHannWindow(int windowSize) {
    std::vector<double> hannWindow(windowSize,0.0);
    for (int i{}; i < windowSize; ++i) {
        const double w = 0.5*(1.0-cos((2*PI*i)/(windowSize-1)));
        hannWindow[i] = w;
    }
    return hannWindow;
}
std::vector<double> lowpassfir(int cutoff,int originalRate,int filLen) {
std::vector<double> h(filLen);
    std::vector<double> w = generateHannWindow(filLen);
const double fc = (cutoff*1.0)/originalRate;
    const int N = filLen-1;
    for (int n{};n<=N;n++) {
        const int idx = n - N/2;
        double h_ideal = (idx==0)?2*fc:(sin(2*PI*fc*idx)/(PI*idx));
        h[n] = h_ideal * w[n];
    }
    double sum = 0.0;
    for (const auto& x : h ) {
        sum += x;
    }
    for (auto& x : h ) {
        x/=sum;
    }
return h;
}
std::vector<double> polyphaseDecimate(const std::vector<double>& signal,const std::vector<double>& h,int M) {
    const int N = signal.size();
    const int P  = h.size();
    int outSize = N/M;
    std::vector<double> output(outSize,0.0);
    for (int i{}; i < outSize; ++i) {
        int n = i * M; // input index for this output sample
        double sum = 0.0;
        const int k_start = std::max(0, n - N + 1);
        const int k_end = std::min(P, n + 1);
        for (int k = k_start; k < k_end; ++k) { // Convolve:
            sum += h[k] * signal[n - k];
        }
        output[i] = sum;
    }
    return output;
}
void iterativeFFT(std::vector<cd>& signal) {
    int n = signal.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;//carry bit adder to find 0
        j ^= bit; //set that 0 to 1

        if (i < j)
            std::swap(signal[i], signal[j]);//only when not equal
    }
    for (int len = 2;len<=n;len<<=1) {
        const double theta = -(2*PI)/len;
        cd wl(cos(theta),sin(theta));
        for (int i =0;i<n;i+=len) {
            cd w(1); //phase rotate twiddle factor
            for (int j=0;j<len/2;j++) { //butterfly step
                cd u = signal[i+j];
                cd v = signal[i+j+len/2]*w;
                signal[i+j] = u+v;
                signal[i+j+len/2] = u-v;
                w*=wl;
            }
        }
    }
}
std::vector<std::vector<cd>> signalFramer(const std::vector<double>& signal,const int frameSize,const int hopSize) {
    std::vector<double> hannWindow = generateHannWindow(frameSize);
    const int numframes = (signal.size() - frameSize) / hopSize + 1;
    std::vector<std::vector<cd>> spec;
    spec.reserve(numframes);
     for (int start = 0;start+frameSize<=signal.size();start+=hopSize) {
       std::vector<cd> output(frameSize,0.0);
        for (int i = 0; i < frameSize; ++i) {
            output[i] = signal[start+i];
        }
       for (int j = 0;j < frameSize; ++j) {
        output[j]*=hannWindow[j];
       }
         iterativeFFT(output);
         spec.push_back(output);
   }
    return spec;
}
