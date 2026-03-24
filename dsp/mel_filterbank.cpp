#include "mel_filterbank.h"
#include <cstdio>
#include <cstdlib>

/* Pre-emphasis is a high-pass filter that boosts the energy in higher frequencies.
 * This serves two main purposes: it balances the frequency spectrum, making it less tilted,
 * and it can improve the signal-to-noise ratio (SNR) by amplifying important high-frequency formants.
 * The filter is typically implemented as a first-order difference equation.
 *
 * filt_out - pointer to filtered signal array;
 * filt_inp - pointer to signal to be filtered array;
 * N - inout length;
 * alpha - filter coefficient. Typical value is in range 0.95...0.97.
 */
void pre_emphasis_filter(float *filt_out, float *filt_inp, int N, float alpha)
{
    filt_out[0] = filt_inp[0];

    for(int n = 1; n < N; n++)
        filt_out[n] = filt_inp[n] - alpha * filt_inp[n - 1];
}

float freq_to_mel(float freq)
{
    return 2595.0f * log10f(1.0f + (freq / 700.f));
}

float mel_to_freq(float mel)
{
    return 700.0f * (powf(10.0, (mel / 2595.0f)) - 1.0f);
}

void freq_to_mel_vec(float *mels, float *freqs, int freqs_num)
{
    for(int i = 0; i < freqs_num; i++)
        mels[i] = freq_to_mel(freqs[i]);
}

void mel_to_freq_vec(float *freqs, float *mels, int freqs_num)
{
    for(int i = 0; i < freqs_num; i++)
        freqs[i] = mel_to_freq(mels[i]);
}


// Calculates bin numbers corresponding to each frequency
// freqs_num
// Fs - sampling rate
// N - number of fft points
void hz_to_fft_bins(int *bins, float *freqs, int Fs, int freqs_num, int N)
{
    for(int i = 0; i < freqs_num; i++)
        bins[i] = ((N + 1.0f) * freqs[i]) / Fs;
}

// Fs - sammpling rate
// N - number of FFT points
// M - desired number of filters, typically
//   M = 40, for audio processing
float* create_mel_filterbank(int Fs, int N, int M)
{
    // Usable fourier transform bins
    // In real signal all frequencies above N/2 are aliased ("mirrored")
    int K = N / 2 + 1;

    // Determine the minimum and maximum frequencies
    float freq_min = 0;
    float freq_max = Fs / 2;

    // Convert these frequencies to mel scale
    float mel_min = freq_to_mel(freq_min);
    float mel_max = freq_to_mel(freq_max);

    // Create equally spaced points in the mel scale
    float mel_points_num = M + 2;
    float mel_step = (mel_max - mel_min) / (M + 1);

    float *mel_points   = (float*)malloc_nc(mel_points_num * sizeof(float));
    float *freq_points  = (float*)malloc_nc(mel_points_num * sizeof(float));
    int *ft_bin_indices = (int*)malloc_nc(mel_points_num * sizeof(int));

    for(int i = 0; i < mel_points_num; i++)
        mel_points[i] = i * mel_step;

    // Convert mel points back to Hz
    mel_to_freq_vec(freq_points, mel_points, mel_points_num);

    // Calculate corresponding fourier transform bins
    for(int i = 0; i < mel_points_num; i++)
        ft_bin_indices[i] = (N * freq_points[i]) / Fs;

    // Generate triangular filters based on these points
    int filterbank_size = M * K;

    float *mel_filterbank = (float*)malloc_nc(filterbank_size * sizeof(float));

    for(int m = 0; m < M; m++)  // Iterate through filters
    {
        int f_left      = ft_bin_indices[m];
        int f_center    = ft_bin_indices[m+1];
        int f_right     = ft_bin_indices[m+2];

        for(int k = 0; k < K; k++) // Iterate through bins
        {
            int idx = m * K + k;

            float val;

            if(k < f_left)
            {
                val = 0.0f;
            }
            else if(f_left <= k && k <= f_center)
            {
                if(f_center == f_left)
                    val = 1; // signal doesn't have slope -> rises immediately
                else
                    val = ((float)(k - f_left)) / (f_center - f_left);
            }
            else if(f_center <= k && k <= f_right)
            {
                if(f_center == f_right)
                    val = 0; // signal doesn't have slope -> falls immediately
                else
                    val = ((float)(f_right - k)) / (f_right - f_center);
            }
            else if(k > f_right)
            {
                val = 0.0f;
            }

            mel_filterbank[idx] = val;
        }
    }

    free_nc(ft_bin_indices);
    free_nc(freq_points);
    free_nc(mel_points);

    return mel_filterbank;
}

void destroy_mel_filterbank(float* mel_filterbank)
{
    free_nc(mel_filterbank);
}

void apply_mel_filterbank(float *mel_energies, float *power_spectrum, float *mel_filterbank, int M, int N)
{
    int K = N / 2 + 1;

    for(int m = 0; m < M; m++)
    {
        float Em = 0.0f;

        for(int k = 0; k < K; k++)
        {
            Em += power_spectrum[k] * mel_filterbank[m * K + k];
        }

        mel_energies[m] = Em;
    }
}
