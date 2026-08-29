#include "Core/SECI/EventLayer/Public/Events/ApplicationEvent.h"

namespace Sora::SECI {

    AppTickEvent::AppTickEvent() : SECIEvent(SECIEventPriority::Normal) {}
    AppUpdateEvent::AppUpdateEvent() : SECIEvent(SECIEventPriority::Normal) {}
    AppRenderEvent::AppRenderEvent() : SECIEvent(SECIEventPriority::Normal) {}

} // namespace Sora::SECI