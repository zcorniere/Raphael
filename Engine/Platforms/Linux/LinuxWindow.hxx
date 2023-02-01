#pragma once

#include "Engine/Platforms/Window.hxx"

#include <SDL.h>

namespace Raphael
{

namespace Windows
{
    class LinuxWindow : public Window
    {
    public:
        static void EnsureSDLInit();

    public:
        LinuxWindow();
        virtual ~LinuxWindow();

        void Initialize(const std::shared_ptr<WindowDefinition> &InDefinition, const Ref<LinuxWindow> &InParent);

        const Ref<LinuxWindow> &GetParent() const;

        virtual void ReshapeWindow(int32 X, int32 Y, int32 Width, int32 Height) override;
        virtual void MoveWindow(int32 X, int32 Y) override;
        virtual void BringToFront(bool bForce = false) override;

        virtual void Destroy() override;
        virtual void Minimize() override;
        virtual void Maximize() override;
        virtual void Restore() override;

        virtual void Show() override;
        virtual void Hide() override;

        virtual bool IsMaximized() const override;
        virtual bool IsMinimized() const override;
        virtual bool IsVisible() const override;

        virtual void AcceptInput(bool bEnable) override;
        virtual int32 GetWindowBorderSize() const override;
        virtual int32 GetWindowTitleBarSize() const override;

        virtual void SetText(const std::string_view Text) override;
        virtual void DrawAttention(bool bStop = false) override;

    private:
        SDL_Window *GetHandle();

        static bool InitializeSDL();

    private:
        SDL_Window *p_HWnd;

        bool bIsVisible;
        Ref<LinuxWindow> p_ParentWindow;
    };

}    // namespace Windows

using Window = Windows::LinuxWindow;

}    // namespace Raphael
