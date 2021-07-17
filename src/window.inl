
inline f64 clock_tick(Clock *clock, f64 time) {
    clock->time_increment = time - clock->time;
    clock->time = time;
    return clock->time_increment;
}

inline int update_frame_counter(Fps *fps, f64 time) {
    fps->counter += 1;
    f64 const time_interval = time - fps->last_update_time;
    if (time_interval >= 1.0) {
        fps->rate = (int) round(fps->counter / time_interval);
        fps->counter = 0;
        fps->last_update_time = time;
    }
    return fps->rate;
}
