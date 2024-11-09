#include "Engine/Misc/CommandLine.hxx"

static char CommandLineStorage[16384] = {0};

void FCommandLine::Set(const char* CommandLine)
{
    std::strncpy(CommandLineStorage, CommandLine, sizeof(CommandLineStorage));
}

void FCommandLine::Set(const int argc, const char* const* const argv)
{
    std::memset(CommandLineStorage, 0, sizeof(CommandLineStorage));
    for (int i = 1; i < argc; ++i) {
        std::strncat(CommandLineStorage, argv[i], sizeof(CommandLineStorage) - 1);
        std::strncat(CommandLineStorage, " ", sizeof(CommandLineStorage) - 1);
    }
}

void FCommandLine::Reset()
{
    std::memset(CommandLineStorage, 0, sizeof(CommandLineStorage));
}

const char* FCommandLine::Get()
{
    return CommandLineStorage;
}

bool FCommandLine::Param(const char* Key)
{
    return std::strstr(CommandLineStorage, Key) != nullptr;
}

bool FCommandLine::Parse(const char* Key, int& Value)
{
    const char* const FoundKey = std::strstr(CommandLineStorage, Key);
    if (FoundKey == nullptr) {
        return false;
    }
    Value = std::atoi(FoundKey + std::strlen(Key));
    return true;
}
bool FCommandLine::Parse(const char* Key, std::string& Value)
{
    const char* const FoundKey = std::strstr(CommandLineStorage, Key);
    if (FoundKey == nullptr) {
        return false;
    }

    const char* ValueStartInStream = FoundKey + std::strlen(Key);
    const char* ValueEndInStream = nullptr;
    if (ValueStartInStream == nullptr) {
        return false;
    }

    const bool bArgumentQuoted = *ValueStartInStream == '"';
    if (bArgumentQuoted) {
        ValueStartInStream += 1;
        ValueEndInStream = std::strchr(ValueStartInStream, '"');
        if (ValueEndInStream == nullptr) {
            // Unterminated quote, so we just take the rest of the string
            ValueEndInStream = ValueStartInStream + std::strlen(ValueStartInStream);
        }
    } else {
        ValueEndInStream = std::strchr(ValueStartInStream, ' ');
    }
    const int Length = std::max(0, int(ValueEndInStream - ValueStartInStream));

    Value = std::string(ValueStartInStream, Length);
    return true;
}
