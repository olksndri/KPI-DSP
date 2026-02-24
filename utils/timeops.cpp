#include "timeops.h"

double current_time_ms() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    return (double)duration.count() / 1000.0;
}
