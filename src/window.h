#pragma once

#include "prelude.h"

typedef struct Clock {
    f64 time; // time of last clock tick
    f64 time_increment; // time between consecutive ticks
} Clock;

inline f64 clock_tick(Clock *clock, f64 time) {
    clock->time_increment = time - clock->time;
    clock->time = time;
    return clock->time_increment;
}

typedef struct Fps {
    int rate;
    int counter;
    f64 last_update_time;
} Fps;

inline int update_fps(Fps *fps, GLFWwindow *window, f64 time) {
    fps->counter += 1;
    f64 const time_interval = time - fps->last_update_time;
    if (time_interval >= 1.0) {
        fps->rate = (int) roundf(fps->counter / time_interval);
        fps->counter = 0;
        fps->last_update_time = time;
    }
    return fps->rate;
}
