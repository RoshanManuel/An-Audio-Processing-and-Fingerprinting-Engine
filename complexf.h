#ifndef SHAZAM_COMPLEXF_H
#define SHAZAM_COMPLEXF_H
struct cf{
    float real;
    float imag;
    constexpr cf(const float r=0.0f, const float i=0.0f):real(r),imag(i) {}
};
#endif //SHAZAM_COMPLEXF_H