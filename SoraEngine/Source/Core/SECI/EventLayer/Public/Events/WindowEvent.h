#pragma once

#include "Core/Core.h"
#include "Core/SECI/EventLayer/Public/SECIEvent.h"

namespace Sora::SECI
{

	class SORA_API WindowResizeEvent : public SECIEvent {
	public:
		WindowResizeEvent(uint32_t InWidth, uint32_t InHeight);
		
		SORA_NO_DISCARD inline uint32_t GetWidth() const { return Width; }
		SORA_NO_DISCARD inline uint32_t GetHeight() const { return Height; }
		SORA_NO_DISCARD std::string ToString () const override;

		SECI_EVENT_CLASS_TYPE(WindowResize)
        SECI_EVENT_CLASS_CATEGORY(EventCategoryWindow | EventCategoryApplication)

	private:
		uint32_t Width = 0;
        uint32_t Height = 0;
	};

	class SORA_API WindowCloseEvent : public SECIEvent
    {
    public:
        WindowCloseEvent();

        SECI_EVENT_CLASS_TYPE(WindowClose)
        SECI_EVENT_CLASS_CATEGORY(EventCategoryWindow | EventCategoryApplication)
    };

    class SORA_API WindowFocusEvent : public SECIEvent
    {
    public:
        WindowFocusEvent();

        SECI_EVENT_CLASS_TYPE(WindowFocus)
        SECI_EVENT_CLASS_CATEGORY(EventCategoryWindow)
    };

    class SORA_API WindowLostFocusEvent : public SECIEvent
    {
    public:
        WindowLostFocusEvent();

        SECI_EVENT_CLASS_TYPE(WindowLostFocus)
        SECI_EVENT_CLASS_CATEGORY(EventCategoryWindow)
    };

} // namespace Sora::SECI
