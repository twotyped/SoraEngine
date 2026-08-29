#pragma once

#include "Core/Core.h"
#include "Core/SECI/EventLayer/Public/SECIEvent.h"
#include "Core/Input/Public/MouseCodes.h"

namespace Sora::SECI
{

    class SORA_API MouseMovedEvent : public SECIEvent {
    public:
        MouseMovedEvent(float InX, float InY);

        SORA_NO_DISCARD inline float GetX() const { return MouseX; }
        SORA_NO_DISCARD inline float GetY() const { return MouseY; }

        SORA_NO_DISCARD std::string ToString() const override;

        SECI_EVENT_CLASS_TYPE(MouseMoved)
		SECI_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        float MouseX;
        float MouseY;
    };

    class SORA_API MouseScrolledEvent : public SECIEvent {
    public:
        MouseScrolledEvent(float InXOffset, float InYOffset);

        SORA_NO_DISCARD inline float GetXOffset() const { return XOffset; }
        SORA_NO_DISCARD inline float GetYOffset() const { return YOffset; }

        SORA_NO_DISCARD std::string ToString() const override;

        SECI_EVENT_CLASS_TYPE(MouseScrolled)
		SECI_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        float XOffset;
        float YOffset;
    };

    class SORA_API MouseButtonEvent : public SECIEvent {
    public:
        SORA_NO_DISCARD inline MouseCode GetMouseButton() const { return Button; }

        SECI_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

    protected:
        explicit MouseButtonEvent(MouseCode InButton)
            : Button(InButton) {}

        MouseCode Button;
    };

    class SORA_API MouseButtonPressedEvent : public MouseButtonEvent {
    public:
        explicit MouseButtonPressedEvent(MouseCode InButton);

        SORA_NO_DISCARD std::string ToString() const override;

        SECI_EVENT_CLASS_TYPE(MouseButtonPressed)
    };

    class SORA_API MouseButtonReleasedEvent : public MouseButtonEvent {
    public:
        explicit MouseButtonReleasedEvent(MouseCode InButton);

        SORA_NO_DISCARD std::string ToString() const override;

        SECI_EVENT_CLASS_TYPE(MouseButtonReleased)
    };
} // namespace Sora::SECI