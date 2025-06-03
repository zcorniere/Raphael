#include "Engine/Misc/Timer.hxx"

#include "Engine/Misc/CommandLine.hxx"

#include <chrono>

DECLARE_LOGGER_CATEGORY(Core, LogTimer, Warning)

FrameLimiter::FrameLimiter(): FrameRate(60)
{
    FCommandLine::Parse("-framerate", FrameRate);
    LOG(LogTimer, Info, "Frame rate: {} FPS", FrameRate);
}

void FrameLimiter::BeginFrame()
{
    StartTime = std::chrono::high_resolution_clock::now();
}

double FrameLimiter::EndFrame()
{
    const std::chrono::high_resolution_clock::time_point EndTime = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> ElapsedTime = EndTime - StartTime;
    const std::chrono::duration<double> TargetFrameDuration(1.0 / FrameRate);

    if (ElapsedTime < TargetFrameDuration)
    {
        RPH_PROFILE_FUNC()
        const std::chrono::duration<double> SleepDuration = TargetFrameDuration - ElapsedTime;
        std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(SleepDuration));
    }
    else if (!FPlatform::isDebuggerPresent())
    {    // Having a debugger attached will make the frame rate slower
         // because, you know, breakpoints
        LOG(LogTimer, Warning, "Frame rate is too low! Frame time was {:.3f} ms, where it is expected to be {:.3f} ms",
            ElapsedTime.count() * 1000, TargetFrameDuration.count() * 1000);
    }

    // Recalculate the total time elapsed including sleep time
    const std::chrono::high_resolution_clock::time_point FinalEndTime = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> TotalElapsedTime = FinalEndTime - StartTime;

    // Return total delta time in seconds
    return TotalElapsedTime.count();
}
