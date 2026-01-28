# An-Audio-Processing-and-Fingerprinting-Engine  
Shazam from scratch. Made to be as optimized I could get using c++17 and concurrent practices. Tried tinkering and building everything myself excluding audio callback.  
Simple and straightforward implementation of a tried and tested algorithm with inclusion of good DSP techniques and low level optimisation.  
It follows a thorough understanding of Shazam's internal working[https://www.ee.columbia.edu/~dpwe/papers/Wang03-shazam.pdf] that lays out the following pipeline:  
The pre-requisite for this entire alogirthm or for any audio processing/study is to convert it into its frequency components. A normal audio recordinng is always in Amplitude x Time domain.This follows as it shows how the audio's strength is at that given point in time(loose definition). This is great for normal visualization or messing around with gain etc but if this were used as, it would be highly corrupted by noise and identifying this clip of audio would prove challenging. Thus, we take its respective frequency components by applying fft to convert it into Frequency x Time domain and work with that.  
Step 1  
i)Read input audio file. There are so many file formats that range from lossy, lossless, compressed, uncompressed, high res. I decided to go with wav format as it is both lossless and uncompressed.This meant that I didn't need any overhead of calling another library to do all the hardwork of converting it into PCM data for me and actually learn how the audio file structure is written internally  
ii)Hence, i looked up the canonical wav file format from this beautiful paper[https://ccrma.stanford.edu/courses/422-winter-2014/projects/WaveFormat/] that lays out the following format of the wav file  
<img width="767" height="688" alt="image" src="https://github.com/user-attachments/assets/8855a6e4-255d-4d54-af39-85aa195e3ab4" />
<img width="845" height="849" alt="image" src="https://github.com/user-attachments/assets/a9d64673-90b6-4a3f-83cc-e6d1fdf5497e" />


The corresponding sample rate, number of channels and all that good stuff were read and processed directly  
iii)I just need the frequency points of the signal and hence, I converted into it mono(Phase is not that needed here, this tradeoff is always very very benefical as processing it as mono rather than stero cuts down the processing time by almost double!). I also couldn't directly work with this sample rate as it is too huge and majorly tank my performance on processing it, thus had to downsample it.  
iv)Given that the wav file supports all the way upto 192khz (and even more now!) and the fact that most audio is sampled at 44100hz, I chose this as the standard and downsampled it to 11025 hz and decimate every 1 in 4th sample  
Step 2  
i)Actual DSP starts here.  
ii)Since downsampling to 11025hz, the cutoff frequency to prevent aliasing is thus <=5512.5hz(Nyquist theorem)  
iii)I went with low pass FIR filter instead of just averaging or for IIR filter as the former is just a cheap trick(lol) and the latter provides no linear phase and hence, it was paired with a windowed sinc function that employs a rectangular symmetric box window function with it(Hann window) and normalized to DC gain.  
iv)Next to decimate for every 1 in 4 samples, I first applied normal decimation, but after doing some digging, I stumbled across a better technique called polyphase decimation that keeps only the samples we want and processes them in many phases(of 2/4/8). \[I dont really know how this works, i've just written it as i've found it on the dspguru site\]  
v)The next part is to apply STFT and this was in turn done by using overlapping frames of size 1024 and 50% hopsize. Before that the edges are tapered off by using Hann window to prevent spectral leakage and then framed one after the other  
vi)I applied the iterative version of FFT on my own that i learnt for CP by optimising naive dft. This version handles bit reversal for grouping and live twiddle factor update, all in O(nlogn).  
vii)Next was to calculate the power and convert into its corressponding log(dB) scale and had it normalised to plot the spectrogram.  
Step 3  
i)I am currently here and have yet to filter based on log bins,find threshold, find local maxima above given threshold and take anchor points within the given radius.  
ii)Then i have to hash and store each of these for every song  
Step 4  
i)Still working and testing out portAudio, audiocallback and loopback works, optimising and building the ring buffer, will commit updates later  
