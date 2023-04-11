#include "Engine/Platforms/Windows/WindowsMisc.hxx"

#include <winuser.h>

EBoxReturnType WindowsMisc::DisplayMessageBox(EBoxMessageType MsgType, const std::string_view Text,
                                       const std::string_view Caption)
{
    unsigned WindowFlags = MB_ICONWARNING;
    switch (MsgType) {
        case EBoxMessageType::Ok: WindowFlags |= MB_OK; break;

        case EBoxMessageType::YesNo: WindowFlags |= MB_YESNO; break;
        default: checkNoEntry(); break;
    }

    int Result = ::MessageBox(nullptr, Text.data(), Caption.data(), WindowFlags);

    switch (Result) {
        case IDYES: return EBoxReturnType::Yes;
        case IDNO: return EBoxReturnType::No;
        case IDOK: return EBoxReturnType::Ok;
        case IDCANCEL: return EBoxReturnType::Cancel;
    }
    return EBoxReturnType::Cancel;
}
