#include "dct.h"
#include <cmath>
#include <math.h>

void dct_1d(float *out, float *inp, int N)
{
    for (int k = 0; k < N; k++)
    {
        float Xk = 0.0f;

        float ak = (k == 0) ? sqrtf(1.0f / N) : sqrtf(2.0f / N);

        for (int n = 0; n < N; n++)
        {
            Xk += inp[n] * cosf((M_PI/N)*(n + 0.5f)*k);
        }

        out[k] = Xk * ak;
    }
}
