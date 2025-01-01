#pragma once

class FrameLimiter
{
public:
    FrameLimiter();

    // Mark the beginning of the frame
    void BeginFrame();
    // Mark the end of the frame, sleep the current thread to fit with asked framerate and return the delta time
    float EndFrame();

    int GetFrameRate() const
    {
        return FrameRate;
    }
    void SetFrameRate(int InFrameRate)
    {
        FrameRate = InFrameRate;
    }

private:
    int FrameRate = 0.0f;
    std::chrono::high_resolution_clock::time_point StartTime;
};
