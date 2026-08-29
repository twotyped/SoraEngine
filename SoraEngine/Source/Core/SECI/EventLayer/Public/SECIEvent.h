#pragma once

#include "Core/Core.h"
#include "SECIEventTypes.h"

#include <string>
#include <functional>

namespace Sora::SECI
{

	/**
     * @brief Abstract base class for all SECI events across Engine, RHI, Window, and Editor.
     */
    class SORA_API SECIEvent
    {
    public:
        virtual ~SECIEvent() = default;

        SORA_NO_DISCARD virtual SECIEventType GetEventType() const = 0;
        SORA_NO_DISCARD virtual const char* GetName() const = 0;
        SORA_NO_DISCARD virtual uint32_t GetCategoryFlags() const = 0;
        SORA_NO_DISCARD virtual SECIEventPriority GetPriority() const;
        SORA_NO_DISCARD virtual std::string ToString() const;
           
        /**
         * @brief Fast bitmask check to see if this event belongs to a specific category.
         */
        SORA_NO_DISCARD inline bool IsInCategory(SECIEventCategory InCategory) const;

    public:
		/**
         * @brief Set to true by a subscriber to stop the event from propagating further down the layer stack.
         */
        bool Handled = false;

    protected:
        /**
         * @brief Allows derived classes to override default priority during construction.
         */
        explicit SECIEvent(SECIEventPriority InPriority = SECIEventPriority::Normal);

    private:
        /**
         * @brief The priority level of the event, governing its place in the execution order within the SECIEventBus
         */
        SECIEventPriority Priority = SECIEventPriority::Normal;
    };

    /**
     * @brief Convenience macros to eliminate boilerplate in derived event classes.
     */
    #define SECI_EVENT_CLASS_TYPE(type) \
        static SECIEventType GetStaticType() { return SECIEventType::type; } \
        virtual SECIEventType GetEventType() const override { return GetStaticType(); } \
        virtual const char* GetName() const override { return #type; }

    #define SECI_EVENT_CLASS_CATEGORY(category_flags) \
        virtual uint32_t GetCategoryFlags() const override { return category_flags; }

} // namespace Sora::SECI
