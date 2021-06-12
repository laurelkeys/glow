#pragma once

#include "prelude.h"

#include <GLFW/glfw3.h>

typedef struct Clock {
    f64 time; // time of last clock tick
    f64 time_increment; // time between consecutive ticks
} Clock;

inline f64 clock_tick(Clock *clock) {
    clock->time_increment = glfwGetTime() - clock->time;
    clock->time += clock->time_increment;
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
        int const new_frame_rate = (int) roundf(fps->counter / time_interval);
        if (new_frame_rate != fps->rate) {
            fps->rate = new_frame_rate;
            char title[32];
            snprintf(title, sizeof(title), "glow | %d fps", fps->rate);
            glfwSetWindowTitle(window, title);
        }
        fps->last_update_time = time;
        fps->counter = 0;
    }
    return fps->rate;
}
