#ifndef FRAG_DEBUG_H_
#define FRAG_DEBUG_H_

#include <chrono>

#define DEBUG_TIME_DECLARE(name) \
     std::chrono::time_point<std::chrono::high_resolution_clock> start_##name; \
     std::chrono::time_point<std::chrono::high_resolution_clock> end_##name; \
     double count_##name = 0; \
     double total_##name = 0; \
     double max_##name = 0;

#define DEBUG_TIME_START(name) \
    if (debug_time) { \
        start_##name = std::chrono::high_resolution_clock::now(); \
    }

#define DEBUG_TIME_END(name) \
    if (debug_time) { \
        end_##name = std::chrono::high_resolution_clock::now(); \
        double elapsed_##name = std::chrono::duration<double, std::milli>(end_##name - start_##name).count(); \
        if (elapsed_##name > max_##name) { \
            max_##name = elapsed_##name; \
        } \
        total_##name += elapsed_##name; \
        count_##name += 1; \
        std::cout << "debug-timer " # name << " (max: " << max_##name << "ms, avg: " << total_##name / count_##name << "): " << elapsed_##name << "ms" << std::endl; \
    }

#endif
