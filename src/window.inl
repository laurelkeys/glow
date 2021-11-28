
static inline f64 clock_tick(Clock *clock, f64 time) {
    clock->time_increment = time - clock->time;
    clock->time = time;
    return clock->time_increment;
}

static inline void update_frame_counter(FrameCounter *frame_counter, f64 time) {
    frame_counter->counter += 1;
    f64 const time_interval = time - frame_counter->last_update_time;
    if (time_interval >= 1.0) {
        frame_counter->last_update_time = time;
        frame_counter->frame_interval = (time_interval * 1000.0) / (f64) frame_counter->counter;
        frame_counter->counter = 0;
    }
}
