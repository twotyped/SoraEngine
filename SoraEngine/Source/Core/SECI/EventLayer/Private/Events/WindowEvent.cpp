#include "Core/SECI/EventLayer/Public/Events/WindowEvent.h"
#include <sstream>

namespace Sora::SECI
{

    WindowResizeEvent::WindowResizeEvent(uint32_t InWidth, uint32_t InHeight)
        : SECIEvent(SECIEventPriority::High)
        , Width(InWidth)
        , Height(InHeight) {}

    std::string WindowResizeEvent::ToString() const {
        std::stringstream ss;
        ss << "WindowResizeEvent: (" << Width << ", " << Height << ")";
        return ss.str();
    }

    WindowCloseEvent::WindowCloseEvent()
        : SECIEvent(SECIEventPriority::Critical) {}

    WindowFocusEvent::WindowFocusEvent()
        : SECIEvent(SECIEventPriority::Normal) {}

    WindowLostFocusEvent::WindowLostFocusEvent()
        : SECIEvent(SECIEventPriority::Normal) {}

} // namespace Sora::SECI