#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <time.h>
#include <sstream>
#include <chrono>
#include <iostream>
#include <thread>
#include <iomanip>

#include "math/complex_math.h"

#include "dsp/fourier_transform.h"
#include "dsp/spectrum_analysis.h"
#include "dsp/window_funcs.h"
#include "dsp/mel_filterbank.h"
#include "dsp/dct.h"

#include "libs/matplotlibcpp.h"

#include "utils/memops.h"

#include "utils/fileops.h"

#include "utils/timeops.h"

namespace plt = matplotlibcpp;


int use_logs = 0;
int use_time_profile = 0;

int lab = 2;


typedef struct s_signal
{
	int N; 					// Number of DFT / IDFT points

	int M; 					// Number of mel filterbank filters

	int Fs; 				// Sample rate

	WINDOW_T window; 	// Type of window to apply

	// float *input_signal;	// Pointer to input signal buffer
	complex *input_signal;	// Pointer to input signal buffer

	complex *dft_res;		// Pointer to DFT result

	complex *idft_res;  	// Pointer to IDFT result

	float *magnitudes;		// Pointer to calculated magnitudes

	float *phase_shifts;	// Pointer to calculated phase shifts

	float *power_spectrum;	// Pointer to calculated power spectrum

	float *amplitudes; 		// Pointer to calculated ampllitudes

	float *frequencies;		// Pointer to calculated frequencies

	float *mel_energies;	// Pointer to calculated mel energies

} s_signal;

void s_signal_init(s_signal *s, int N, int M, int sample_rate, WINDOW_T win)
{
	s->N = N;

	s->M = M;

	s->Fs = sample_rate;

	s->window = win;

	s->input_signal = (complex*)malloc_nc(N * sizeof(complex));

	s->dft_res = (complex*)malloc_nc(N * sizeof(complex));

	s->idft_res = (complex*)malloc_nc(N * sizeof(complex));

	s->magnitudes = (float*)malloc_nc(N * sizeof(float));

	s->phase_shifts = (float*)malloc_nc(N * sizeof(float));

	s->power_spectrum = (float*)malloc_nc(N * sizeof(float));

	s->amplitudes = (float*)malloc_nc(N * sizeof(float));

	s->frequencies = (float*)malloc_nc(N * sizeof(float));

	s->mel_energies = (float*)malloc_nc(M * sizeof(float));
}

void s_signal_deinit(s_signal *s)
{
	free_nc(s->input_signal);

	free_nc(s->dft_res);

	free_nc(s->idft_res);

	free_nc(s->magnitudes);

	free_nc(s->phase_shifts);

	free_nc(s->power_spectrum);

	free_nc(s->amplitudes);

	free_nc(s->frequencies);

	free_nc(s->mel_energies);
}

void display_peaks(float *x, float *y,
	const std::string plot_label, const std::string label_x, const std::string label_y, int peaks_num)
{

	// plt::figure_size(1200, 780);

	std::vector<float> x_v(x, x + peaks_num);
	std::vector<float> y_v(y, y + peaks_num);

	// draw vertical line from 0 to y[i]
	for (size_t i = 0; i < x_v.size(); ++i) {
        plt::plot({x_v[i], x_v[i]}, {0, y_v[i]}, "b-"); // "b-" = blue line
    }

	// draw circles on peaks
	plt::plot(x_v, y_v, "ro"); // "ro" = red circles

	plt::title(plot_label);
	plt::xlabel(label_x);
	plt::ylabel(label_y);

	// printf("showing graph\n");
    plt::show();
	// printf("ended showing graph\n");
    // plt::close();
}

void analyze_audio_0(float *sound_data, s_signal *signal_str)
{
	double t0, t1;

	float_to_complex(sound_data, signal_str->input_signal, signal_str->N);

	if(signal_str->window == NO_WINDOW)
	{
		// Calculate DFT and IDFT  on non-windowed signal
		t0 = current_time_ms();
		dft(signal_str->input_signal, signal_str->dft_res, signal_str->N);
		idft(signal_str->dft_res, signal_str->idft_res, signal_str->N);
		t1 = current_time_ms();
		printf("DFT + IDFT time: %.2lf [ms]\n", t1-t0);

		log_to_file("input_signal", signal_str->input_signal, signal_str->N, COMPLEX);
		log_to_file("dft_res", signal_str->dft_res, signal_str->N, COMPLEX);
		log_to_file("idft_res", signal_str->idft_res, signal_str->N, COMPLEX);

		t0 = current_time_ms();
		fft_recursive(signal_str->input_signal, signal_str->dft_res, signal_str->N);
		ifft_recursive(signal_str->dft_res, signal_str->idft_res, signal_str->N);
		ifft_recursive_scale(signal_str->idft_res, signal_str->N);
		t1 = current_time_ms();
		printf("DFFT + IDFFT time: %.2lf [ms]\n", t1-t0);

		log_to_file("dfft_recursive_res", signal_str->dft_res, signal_str->N, COMPLEX);
		log_to_file("idfft_recursive_res", signal_str->idft_res, signal_str->N, COMPLEX);
	}
	else
	{
		// Apply signal Windowing to avoid spectral leakage
		if(signal_str->window == HANN_WINDOW)
			hann_window_complex(signal_str->input_signal, signal_str->input_signal, signal_str->N);
		else
			perror("Not implemented! \n");

		// Calculate DFT on a signal
		t0 = current_time_ms();
		dft(signal_str->input_signal, signal_str->dft_res, signal_str->N);
		t1 = current_time_ms();
		printf("DFT time: %.2lf [ms]\n", t1-t0);

		t0 = current_time_ms();
		fft_recursive(signal_str->input_signal, signal_str->dft_res, signal_str->N);
		t1 = current_time_ms();
		printf("FFT time: %.2lf [ms]\n", t1-t0);
	}

	// Calculate magnitudes, phase shifts and power spectrum of a signal
	calc_magnitudes(signal_str->dft_res, signal_str->magnitudes, signal_str->N);
	calc_phase_shifts(signal_str->dft_res, signal_str->phase_shifts, signal_str->N);
	calc_power_spectrum(signal_str->dft_res, signal_str->power_spectrum, signal_str->N);

	float Fnyquist = (float)signal_str->Fs / 2; 			// Nyquist frequency [Hz]
	float delta_F = (float)signal_str->Fs / signal_str->N; 	// Frequency resolution - step between spectral bins, [Hz]
	float T = (float)signal_str->N / signal_str->Fs; 		// Time window (time duration of analyzed signal interval), [s]
	float T_ms = T * 1000 ; // Time window in [ms]

	printf("Nyquist frequency: %.2f [Hz]\n", Fnyquist);
	printf("Frequency resolution: %.2f [Hz]\n", delta_F);
	printf("Analysis time window: %.2f [ms]\n", T_ms);

	// Calculate frequency of each bin
	calc_bin_frequencies(signal_str->frequencies, signal_str->Fs, signal_str->N);

	// Calculate amplitude of each bin
	calc_bin_amplitudes(signal_str->magnitudes, signal_str->amplitudes, signal_str->N);

	int display_points = signal_str->N;
	std::string plot_title = "Plot title" ;
	display_peaks(signal_str->frequencies, signal_str->amplitudes, plot_title, "Frequency", "Amplitude", display_points);
}


// LAB 1
// Розробка програмного забезпечення для швидкого
// перетворення Фур’є голосового сигналу.

// Завдання: розробити програмне забезпечення для
// зчитування аудіоданних із звукового
// файлу, їх фрагментації та швидкого перетворення Фур’є кожного із
// фрагментів.
void analyze_audio_1(float *sound_data, s_signal *signal_str, int use_fft, int frame_num)
{
	double t0, t1;

	float_to_complex(sound_data, signal_str->input_signal, signal_str->N);

	// Apply signal Windowing to avoid spectral leakage
	if(signal_str->window == HANN_WINDOW)
		hann_window_complex(signal_str->input_signal, signal_str->input_signal, signal_str->N);

	// Calculate DFT and IDFT  on non-windowed signal
	t0 = current_time_ms();
	if(use_fft)
		fft_recursive(signal_str->input_signal, signal_str->dft_res, signal_str->N);
	else
		dft(signal_str->input_signal, signal_str->dft_res, signal_str->N);
	t1 = current_time_ms();

	printf("Fourier transform time: %.2lf [ms]\n", t1-t0);

	// Calculate magnitudes of a signal
	calc_magnitudes(signal_str->dft_res, signal_str->magnitudes, signal_str->N);

	float Fnyquist = (float)signal_str->Fs / 2; 			// Nyquist frequency [Hz]
	float delta_F = (float)signal_str->Fs / signal_str->N; 	// Frequency resolution - step between spectral bins, [Hz]
	float T = (float)signal_str->N / signal_str->Fs; 		// Time window (time duration of analyzed signal interval), [s]
	float T_ms = T * 1000 ; // Time window in [ms]

	printf("Nyquist frequency: %.2f [Hz]\n", Fnyquist);
	printf("Frequency resolution: %.2f [Hz]\n", delta_F);
	printf("Analysis time window: %.2f [ms]\n", T_ms);

	// Calculate frequency and amplitude of each bin
	calc_bin_frequencies(signal_str->frequencies, signal_str->Fs, signal_str->N);
	calc_bin_amplitudes(signal_str->magnitudes, signal_str->amplitudes, signal_str->N);


	std::stringstream ss;

	float time_window_beg = frame_num * T_ms;
	float time_window_end = time_window_beg + T_ms;
	std::string time_mes_unit = "[ms]";

	if(time_window_beg >= 1000)
	{
		time_window_beg /= 1000;
		time_window_end /= 1000;
		time_mes_unit = "[s]";
	}
	ss << std::fixed << std::setprecision(2);
	ss << "Analyzed time window: " << time_window_beg << "..." << time_window_end << " " << time_mes_unit;

	std::string plot_title = ss.str();

	int display_points = signal_str->N;
	display_peaks(signal_str->frequencies, signal_str->amplitudes, plot_title, "Frequency", "Amplitude", display_points);
}

FILE *gp;

void plot_current_mfcc(float* mfcc, int M, int frame_idx) {
    gp = popen("gnuplot", "w");
    if (!gp) return;

    fprintf(gp, "set title 'MFCC Frame: %d'\n", frame_idx);
    fprintf(gp, "set ylabel 'Value (dB/Amplitude)'\n");
    fprintf(gp, "set xlabel 'Coefficient Index'\n");

    fprintf(gp, "set style fill solid 1.0 border -1\n");
    fprintf(gp, "set boxwidth 0.8\n");
    fprintf(gp, "set grid y\n");

    fprintf(gp, "set autoscale y\n");
    fprintf(gp, "set autoscale x\n");

    fprintf(gp, "plot '-' with boxes title 'MFCC Coefficients'\n");

    for (int m = 0; m < M; m++) {
        fprintf(gp, "%d %f\n", m, mfcc[m]);
    }

    fprintf(gp, "e\n");
    fflush(gp);

    // pclose(gp);
}

void wait_for_1_key() {
    int x = 0;
    while (x != 1)
        std::cin >> x;
}

// LAB 2
// Розробка програмного забезпечення для розрахунку
// мел-кепстральних коефіцієнтів голосового сигналу.

// Завдання:
// розробити програмне забезпечення для розрахунку мел-кепстральних коефіцієнтів
// стаціонарних фрагментів голосового сигналу.
void calc_mfcc(float *sound_data, float *mel_filterbank, float *scratch, s_signal *signal_str, int samples)
{
    // Next steps need to be followed to calculate MFCCs:

    // 1. Pre-emphasize the signal: Amplify higher frequencies to balance the spectrum.
    float alpha = 0.96; // filter coefficient
    pre_emphasis_filter(scratch, sound_data, samples, alpha);

    float *mfcc = (float *)malloc_nc(signal_str->M * sizeof(float));

    int frame_count = 0;

    // 2. Framing: Break the signal into small, overlapping frames.
    int stride = signal_str->N / 2; // 50% overlap
    for(int i = 0; i <= samples - signal_str->N; i += stride)
    {
        float *cur_frame = scratch + i;

        float_to_complex(cur_frame, signal_str->input_signal, signal_str->N);

        // 3. Windowing: To soften the edges of each frame, apply a Hann window.
        hann_window_complex(signal_str->input_signal, signal_str->input_signal, signal_str->N);

       	// 4. FFT: Convert each frame from the time domain to the frequency domain.
        fft_recursive(signal_str->input_signal, signal_str->dft_res, signal_str->N);

        // 5. Mel-filterbank: Apply overlapping triangular filters spaced according to the Mel-scale on signals power spectrum.
        calc_power_spectrum(signal_str->dft_res, signal_str->power_spectrum, signal_str->N);

        apply_mel_filterbank(signal_str->mel_energies, signal_str->power_spectrum, mel_filterbank, signal_str->M, signal_str->N);

        // 6. Logarithm: To replicate the way a human ear reacts to sound strength take the logarithm of the filterbank outputs.
        for(int m = 0; m < signal_str->M; m++) // Convert mel energies into dB
            signal_str->mel_energies[m] = 10 * log10f(signal_str->mel_energies[m] + 1e-10);

        // 7. DCT: Apply the DCT to the log Mel-spectrum to obtain the Mel-frequency Cepstral Coefficients.
        dct_1d(mfcc, signal_str->mel_energies, signal_str->M);

        // 8. Visualize current frame MFCC using GNUplot
        plot_current_mfcc(mfcc, signal_str->M, frame_count);

        // 9. Pause until user enters "1" in console
        wait_for_1_key();
        pclose(gp);
        frame_count++;
    }

    free_nc(mfcc);
}



void log_sf_info(SF_INFO *sf_info)
{
	printf("\n");
	printf("Sound file info: \n");
	printf("\t frames: %d\n", sf_info->frames); // total samples count in audiofile
	printf("\t samplerate: %d\n", sf_info->samplerate);
	printf("\t channels: %d\n", sf_info->channels);
	printf("\t format: %d\n", sf_info->format);
	printf("\t sections: %d\n", sf_info->sections);
	printf("\t seekable: %d\n", sf_info->seekable);

	float sf_time = (float)sf_info->frames / sf_info->samplerate;
	printf("\t time: %.2f seconds\n", sf_time);

	printf("\n\n");
}

SF_INFO sf_info;

s_signal signal;

int main(int argc, char **argv)
{
	try {
        plt::backend("Qt5Agg");
    } catch (const std::exception& e) {
        printf("Backend error: %s\n", e.what());
    }

	SNDFILE* f_sound;
	if(argc > 1)
	{
 		f_sound = sf_open(argv[1], SFM_READ, &sf_info);

		if(f_sound == NULL)
		{
			perror("Error occured while opening file: ");
			printf("%s\n", argv[1]);
			abort();
		}
	}
	else
	{
		perror("Please provide path to sound file to be analyzed!\n");
		abort();
	}


	log_sf_info(&sf_info);
	int total_samples = sf_info.frames * sf_info.channels;

	int16_t *sound_data = (int16_t*)malloc_nc(total_samples * sizeof(int16_t));
	int16_t *sound_data_ch0 = (int16_t*)malloc_nc(sf_info.frames * sizeof(int16_t));
	float *sound_data_ch0_fl = (float*)malloc_nc(sf_info.frames * sizeof(float));
	float *scratch = (float*)malloc_nc(sf_info.frames * sizeof(float));

	sf_read_short(f_sound, (int16_t*)sound_data, sf_info.frames*sf_info.channels);

	for(int i = 0, k = 0; i < total_samples; k++, i += sf_info.channels)
		sound_data_ch0[k] = sound_data[i];

	for(int i = 0; i < sf_info.frames; i++)
		sound_data_ch0_fl[i] = (float)sound_data_ch0[i] / INT16_MAX;


	int sample_rate = sf_info.samplerate;
	int N = 0;
	int M = 0;


	if(lab == 1)
	{
    	int win_flag = 0;
    	int use_fft = 0;

    	std::cout << "Enter number of DFT points: \t";
    	std::cin >> N;
    	std::cout << "\n";
    	std::cout << "Should we use FFT? Print 1 or 0: \t";
    	std::cin >> use_fft;
    	std::cout << "\n";
    	std::cout << "Should we use hann window? Print 1 or 0: \t";
    	std::cin >> win_flag;
    	std::cout << "\n";

    	printf("Number of DFT points: %d\n", N);
    	printf("Used window: %s\n", (win_flag) ? "HANN_WINDOW" :"NO_WINDOW");

    	s_signal_init(&signal, N, M, sample_rate, (win_flag) ? HANN_WINDOW : NO_WINDOW);
    	int frames_to_process = sf_info.frames / N;
    	for(int i = 0; i < frames_to_process; i++)
    	{
    		analyze_audio_1(&sound_data_ch0_fl[N*i], &signal, use_fft, i);
    	}

    	s_signal_deinit(&signal);
	}
	else if (lab == 2)
	{
	    N = 1024; // FFT pointsя
	    M = 40; // filters number
		int Fs = sample_rate;

		s_signal_init(&signal, N, M, sample_rate, HANN_WINDOW);

        float* mel_filterbank = create_mel_filterbank(Fs, N, M);

	    calc_mfcc(sound_data_ch0_fl, mel_filterbank, scratch, &signal, sf_info.frames);

		destroy_mel_filterbank(mel_filterbank);

		s_signal_deinit(&signal);
	}
	else
	{
	    std::cout   << "Error, there is no lab " << lab << "implementation."
					<< "Exiting program\n\n";
		exit(-1);
	}

	free_nc(scratch);
	free_nc(sound_data_ch0_fl);
	free_nc(sound_data_ch0);
	free_nc(sound_data);

	sf_close(f_sound);

	return 0;
}
