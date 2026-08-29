#pragma once

#include "Core/Window/Public/Window.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Sora
{

    class SORA_API WindowsWindow : public Window
    {
    public:
        explicit WindowsWindow(const WindowProperties& InProperties);
        virtual ~WindowsWindow() override;

        void OnUpdate() override;

        SORA_NO_DISCARD inline uint32_t GetWidth() const override { return WindowData.Width; }
        SORA_NO_DISCARD inline uint32_t GetHeight() const override { return WindowData.Height; }

        inline void SetEventCallback(const EventCallbackFn& InCallback) override { WindowData.EventCallback = InCallback; }
        void SetVSync(bool InEnabled) override;
        SORA_NO_DISCARD bool IsVSync() const override { return WindowData.VSync; }

        SORA_NO_DISCARD inline void* GetNativeWindow() const override { return WindowHandle; }

    private:
        virtual void Initialize(const WindowProperties& InProps);
        virtual void Shutdown();

        static LRESULT CALLBACK WindowProc(HWND InHwnd, UINT InMsg, WPARAM InWParam, LPARAM InLParam);

    private:
        HWND WindowHandle = nullptr;
        HINSTANCE InstanceHandle = nullptr;

        struct WindowDataInternal
        {
            std::string Title;
            uint32_t Width = 0;
            uint32_t Height = 0;
            bool VSync = false;

            EventCallbackFn EventCallback;
        };

        WindowDataInternal WindowData;
    };

} // namespace Sora