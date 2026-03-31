#include <stdint.h>

void RGBA_to_YCbCr(uint8_t *YCbCr, uint8_t *A, uint8_t *RGBA, int h, int w)
{

	uint8_t *p_out = YCbCr;
	uint8_t *p_alpha = A;
	uint8_t *p_inp = RGBA;

	for(int i = 0; i < h * w; i++)
	{
		float R = p_inp[i * 4 + 0];
		float G = p_inp[i * 4 + 1];
		float B = p_inp[i * 4 + 2];
		float A = p_inp[i * 4 + 3];

		uint8_t Y	= 16 + (65.481 * R + 128.553 * G + 24.966 * B) / 256;
		uint8_t Cb	= 128 + (-37.797 * R - 74.203 * G + 112 * B) / 256;
		uint8_t Cr 	= 128 + (112 * R - 93.786 * G - 18.214 * B) / 256;

		p_out[i * 3 + 0] = Y;
		p_out[i * 3 + 1] = Cb;
		p_out[i * 3 + 2] = Cr;

		p_alpha[i] = A;
	}
}


void YCbCr_to_RGBA(uint8_t *RGBA, uint8_t *YCbCr, uint8_t *A, int h, int w)
{

	uint8_t *p_out = RGBA;
	uint8_t *p_alpha = A;
	uint8_t *p_inp = YCbCr;

	for(int i = 0; i < h * w; i++)
	{
		float Y 	= p_inp[i * 3 + 0];
		float Cb 	= p_inp[i * 3 + 1];
		float Cr	= p_inp[i * 3 + 2];
		uint8_t A 	= p_alpha[i];

		uint8_t R = 298.082 * (Y - 16) / 256.0 + 408.583 * (Cr - 128.0) / 256.0;
		uint8_t G = 298.082 * (Y - 16) / 256.0 - 100.291 * (Cb - 128.0) / 256.0 - 208.120 * (Cr - 128) / 256.0;
		uint8_t B = 298.082 * (Y - 16) / 256.0 + 516.411 * (Cb - 128.0) / 256.0;

		p_out[i * 4 + 0] = R;
		p_out[i * 4 + 1] = G;
		p_out[i * 4 + 2] = B;
		p_out[i * 4 + 3] = A;
	}
}

// void jpeg_encode()
// {


// }
