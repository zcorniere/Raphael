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

float FrameLimiter::EndFrame()
{
    const std::chrono::high_resolution_clock::time_point EndTime = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> ElapsedTime = EndTime - StartTime;
    const std::chrono::duration<double> TargetFrameDuration(1.0 / FrameRate);

    if (ElapsedTime < TargetFrameDuration) {
        const std::chrono::duration<double> SleepDuration = TargetFrameDuration - ElapsedTime;
        std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(SleepDuration));
    } else {
        LOG(LogTimer, Warning, "Frame rate is too low! Frame time was {}, where it is expected to be {}",
            ElapsedTime.count(), TargetFrameDuration.count());
    }

    // Recalculate the total time elapsed including sleep time
    const std::chrono::high_resolution_clock::time_point FinalEndTime = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> TotalElapsedTime = FinalEndTime - StartTime;

    // Return total delta time in seconds
    return TotalElapsedTime.count();
}
