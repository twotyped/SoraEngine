#pragma once

#include "Core/Core.h"
#include "Core/SECI/EventLayer/Public/SECIEvent.h"

#include <string>
#include <functional>
#include <memory>

namespace Sora
{

	struct WindowProperties {
		std::string Title;
		uint32_t Width;
		uint32_t Height;
        bool ShowCursor;
        bool CursorClipped;

		explicit WindowProperties(const std::string& InTitle = "Sora 0.1.0d",
			uint32_t InWidth = 1280,
			uint32_t InHeight = 720,
            bool InShowCursor = true,
            bool InCursorClipped = false)
			: Title(InTitle), Width(InWidth), Height(InHeight), ShowCursor(InShowCursor), CursorClipped(InCursorClipped)
        {}
    };

	/**
     * @brief Platform-independent interface for desktop windowing systems.
     */
    class SORA_API Window {
    public:
        using EventCallbackFn = std::function<void(SECI::SECIEvent&)>;

        virtual ~Window() = default;

        virtual void OnUpdate() = 0;

        SORA_NO_DISCARD virtual uint32_t GetWidth() const = 0;
        SORA_NO_DISCARD virtual uint32_t GetHeight() const = 0;

        virtual void SetEventCallback(const EventCallbackFn& InCallback) = 0;
        virtual void SetVSync(bool InEnabled) = 0;
        SORA_NO_DISCARD virtual bool IsVSync() const = 0;

        SORA_NO_DISCARD virtual void* GetNativeWindow() const = 0;

        virtual void Shutdown() = 0;

        virtual void ModifyCursorVisibility(bool InCursorVisible) = 0;
        virtual void ModifyCursorClipping(bool InCursorClipped) = 0;
        virtual void CenterCursor() = 0;

        /**
         * @brief Factory method to create the platform-native window implementation.
         */
        static std::unique_ptr<Window> Create(const WindowProperties& InProps = WindowProperties());
    };

} // namespace Sora
