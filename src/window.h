#pragma once

#include "prelude.h"

typedef struct Clock {
    f64 time; // time of the last clock tick (in seconds)
    f64 time_increment; // time between consecutive ticks
} Clock;

inline f64 clock_tick(Clock *clock, f64 time);

typedef struct Fps {
    int rate;
    int counter;
    f64 last_update_time; // (in seconds)
} Fps;

inline int update_frame_counter(Fps *fps, f64 time);

#include "window.inl"
