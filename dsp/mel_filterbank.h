#pragma once

#include <math.h>
#include "../utils/memops.h"

float* create_mel_filterbank(int Fs, int N, int M);

void destroy_mel_filterbank(float *mel_filterbank);

void pre_emphasis_filter(float *filt_out, float *filt_inp, int N, float alpha);

void apply_mel_filterbank(float *logmel_energies, float *power_spectrum, float *mel_filterbank, int M, int N);
