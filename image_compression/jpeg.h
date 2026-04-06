#include <iostream>
#include <vector>
#include <stdint.h>
#include "../dsp/dct.h"
#include "../utils/memops.h"
#include <chrono>
#include <algorithm>
#include <cstdint>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef enum PAD_TYPE
{
	NO_PAD,
	ZERO,		// Pad with zero value
	REPLICATE, 	// Replicate last value
	REFLECT, 	// Reflects edge pixels
}PAD_TYPE;

typedef struct PADDING
{
	PAD_TYPE padding_type;

	uint32_t top;
	uint32_t bottom;
	uint32_t left;
	uint32_t right;
}PADDING;

typedef enum JPEG_CH_TYPE
{
	LUMINANCE,
	CHROMA,
}JPEG_CH_TYPE;

void jpeg_encode(std::vector<int16_t> &Y_zg, std::vector<int16_t> &Cb_zg, std::vector<int16_t> &Cr_zg,
	PADDING &pad_luminance, PADDING &pad_chroma, uint8_t *RGB, int h, int w);

void jpeg_decode(uint8_t *RGB, std::vector<int16_t> &Y_zg, std::vector<int16_t> &Cb_zg, std::vector<int16_t> &Cr_zg,
	PADDING &pad_luminance, PADDING &pad_chroma, int h, int w);
