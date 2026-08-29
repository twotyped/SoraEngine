#include "Core/SECI/EventLayer/Public/Events/KeyEvent.h"
#include <sstream>

namespace Sora::SECI
{

    KeyPressedEvent::KeyPressedEvent(KeyCode InCode, bool InIsRepeat)
        : KeyEvent(InCode)
        , IsRepeating(InIsRepeat) {}

    std::string KeyPressedEvent::ToString() const {
        std::stringstream ss;
        ss << "KeyPressedEvent: " << Code << " (Repeat: " << IsRepeating << ")";
        return ss.str();
    }

    KeyReleasedEvent::KeyReleasedEvent(KeyCode InCode)
        : KeyEvent(InCode) {}

    std::string KeyReleasedEvent::ToString() const {
        std::stringstream ss;
        ss << "KeyReleasedEvent: " << Code;
        return ss.str();
    }

} // namespace Sora::SECI
