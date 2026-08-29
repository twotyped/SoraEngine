#include "Core/SECI/EventLayer/Public/Events/MouseEvent.h"
#include <sstream>

namespace Sora::SECI
{

    MouseMovedEvent::MouseMovedEvent(float InX, float InY)
        : MouseX(InX), MouseY(InY) {}

    std::string MouseMovedEvent::ToString() const {
        std::stringstream ss;
        ss << "MouseMovedEvent: (" << MouseX << ", " << MouseY << ")";
        return ss.str();
    }

    MouseScrolledEvent::MouseScrolledEvent(float InXOffset, float InYOffset)
        : XOffset(InXOffset), YOffset(InYOffset) {}

    std::string MouseScrolledEvent::ToString() const {
        std::stringstream ss;
        ss << "MouseScrolledEvent: (" << XOffset << ", " << YOffset << ")";
        return ss.str();
    }

    MouseButtonPressedEvent::MouseButtonPressedEvent(MouseCode InButton)
        : MouseButtonEvent(InButton) {}

    std::string MouseButtonPressedEvent::ToString() const {
        std::stringstream ss;
        ss << "MouseButtonPressedEvent: " << Button;
        return ss.str();
    }

    MouseButtonReleasedEvent::MouseButtonReleasedEvent(MouseCode InButton)
        : MouseButtonEvent(InButton) {}

    std::string MouseButtonReleasedEvent::ToString() const {
        std::stringstream ss;
        ss << "MouseButtonReleasedEvent: " << Button;
        return ss.str();
    }

} // namespace Sora::SECI