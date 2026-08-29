#pragma once

#include "Core/Core.h"
#include "Core/SECI/EventLayer/Public/SECIEvent.h"

namespace Sora::SECI
{

    class SORA_API AppTickEvent : public SECIEvent
    {
    public:
        AppTickEvent();

        SECI_EVENT_CLASS_TYPE(AppTick)
        SECI_EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class SORA_API AppUpdateEvent : public SECIEvent
    {
    public:
        AppUpdateEvent();

        SECI_EVENT_CLASS_TYPE(AppUpdate)
        SECI_EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class SORA_API AppRenderEvent : public SECIEvent
    {
    public:
        AppRenderEvent();

        SECI_EVENT_CLASS_TYPE(AppRender)
        SECI_EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

} // namespace Sora::SECI
