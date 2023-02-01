#pragma once

#include "Engine/Core/Log.hxx"
#include "Engine/Core/RObject.hxx"
#include "Engine/Platforms/Platform.hxx"

#include <SDL3/SDL.h>

namespace Raphael
{

class GenericApplication : public RObject
{
public:
    GenericApplication()
    {
        Log::Init();
    }
    virtual ~GenericApplication()
    {
        Log::Shutdown();
    }

    // Called on init, return false if there was an error
    virtual bool Initialize()
    {
        return true;
    }
    virtual void Shutdown()
    {
    }

    virtual void ProcessEvent(SDL_Event)
    {
    }

    virtual void Tick(const float DeltaTime) = 0;
    virtual bool ShouldExit() const
    {
        return false;
    }
};

};    // namespace Raphael

#if defined(PLATFORM_WINDOWS)
    #include "Engine/Platforms/Windows/WindowsApplication.hxx"
#elif defined(PLATFORM_LINUX)
    #include "Engine/Platforms/Linux/LinuxApplication.hxx"
#else
static_assert(false, "Unsuported Platform !");
#endif
