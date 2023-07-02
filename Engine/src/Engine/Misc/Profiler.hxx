#pragma once

#if RPH_ENABLE_PROFILING == 1
    #include <tracy/Tracy.hpp>

    #define RPH_PROFILE_MARK_FRAME FrameMark;
    #define RPH_PROFILE_FUNC(...) ZoneScoped##__VA_OPT__(N(__VA_ARGS__))
    #define RPH_PROFILE_SCOPE_DYNAMIC(Name) \
        ZoneScoped;                         \
        ZoneName(Name, strlen(Name))
    #define RPH_PROFILE_THREAD(...) tracy::SetThreadName(__VA_ARGS__)

#else
    #define RPH_PROFILE_MARK_FRAME
    #define RPH_PROFILE_FUNC(...)
    #define RPH_PROFILE_SCOPE_DYNAMIC(Name)
    #define RPH_PROFILE_THREAD(...)
#endif
