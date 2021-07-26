
inline f64 clock_tick(Clock *clock, f64 time) {
    clock->time_increment = time - clock->time;
    clock->time = time;
    return clock->time_increment;
}

inline void update_frame_counter(Fps *fps, f64 time) {
    fps->counter += 1;
    f64 const time_interval = time - fps->last_update_time;
    if (time_interval >= 1.0) {
        fps->last_update_time = time;
        fps->frame_interval = (time_interval * 1000.0) / (f64) fps->counter;
        fps->counter = 0;
    }
}
