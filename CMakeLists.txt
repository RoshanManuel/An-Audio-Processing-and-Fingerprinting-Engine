cmake_minimum_required(VERSION 3.20)
project(Audio_Spectrogram)

set(CMAKE_CXX_STANDARD 17)

add_executable(ImageGenerator
        main.cpp
        downsampler.cpp
        Spectrogram.cpp)
