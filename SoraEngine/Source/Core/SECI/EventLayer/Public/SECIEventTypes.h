#pragma once

#include "Core/Core.h"
#include <cstdint>

namespace Sora::SECI
{

	/**
	 * @brief Identification enum for every individual SECI event
	*/
    enum class SECIEventType : uint16_t
	{
		None = 0,

		// Window
		WindowClose,
		WindowResize,
		WindowFocus,
		WindowLostFocus,
		WindowMoved,

		// Core Application
		AppTick,
		AppUpdate,
		AppRender,

		// Key Input
		KeyPressed,
		KeyReleased,
		KeyTyped,

		// Mouse Input
		MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled,

		// Controller Input Events
        GamepadConnected,
        GamepadDisconnected,
        GamepadButtonPressed,
        GamepadButtonReleased,
        GamepadAxisMoved,

		// Low-Level RHI / Graphics Events
        RHIDeviceLost,
        RHIDeviceRestored,
        RHISwapchainResized,
        RHIRenderTargetInvalidated,

        // Engine Control & Editor Events
        EditorViewportResize,
        EditorEntitySelected,
        EditorPlayModeStarted,
        EditorPlayModeStopped,
	};

	/**
     * @brief Bitmask categories allowing event handlers to filter unwanted events instantly.
     */
    enum SECIEventCategory : uint32_t
    {
        EventCategoryNone        = 0,
        EventCategoryApplication = (1 << 0), // 0x01
        EventCategoryInput       = (1 << 1), // 0x02
        EventCategoryKeyboard    = (1 << 2), // 0x04
        EventCategoryMouse       = (1 << 3), // 0x08
        EventCategoryMouseButton = (1 << 4), // 0x10
        EventCategoryGamepad     = (1 << 5), // 0x20
        EventCategoryWindow      = (1 << 6), // 0x40
        EventCategoryRHI         = (1 << 7), // 0x80
        EventCategoryEditor      = (1 << 8)  // 0x100
    };

    /**
     * @brief Priority levels governing execution order inside SECIEventBus.
     */
    enum class SECIEventPriority : uint8_t
    {
        Lowest   = 0,
        Low      = 1,
        Normal   = 2,
        High     = 3,
        Critical = 4  // Processed immediately (e.g., RHI device loss, Window Close)
    };

    // Bitwise helper macros for SECIEventCategory bitmask checks
    inline SECIEventCategory operator|(SECIEventCategory a, SECIEventCategory b)
    {
        return static_cast<SECIEventCategory>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline SECIEventCategory operator&(SECIEventCategory a, SECIEventCategory b)
    {
        return static_cast<SECIEventCategory>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

} // namespace Sora::SECI
