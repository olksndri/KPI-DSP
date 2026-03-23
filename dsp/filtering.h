#pragma once

#include <math.h>
#include "../utils/memops.h"

void pre_emphasis_filter(float *filt_out, float *filt_inp, int N, int alpha);
