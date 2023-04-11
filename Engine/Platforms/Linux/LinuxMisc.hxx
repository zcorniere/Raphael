#pragma once

#include "Engine/Platforms/PlatformMisc.hxx"

class LinuxMisc : public GenericMisc
{
public:
    static EBoxReturnType DisplayMessageBox(EBoxMessageType MsgType, const std::string_view Text,
                                     const std::string_view Caption);
};

using PlatformMisc = LinuxMisc;
