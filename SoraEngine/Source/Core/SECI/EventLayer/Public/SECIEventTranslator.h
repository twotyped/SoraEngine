#pragma once

#include "SECIEvent.h"

namespace Sora::SECI
{

    /**
     * @brief Template interface for converting OS/Backend native events into engine SECIEvents.
     *
     * @tparam NativeEventStruct Third-party struct (e.g., GLFWwindow + parameters, Win32 MSG)
     */
    template<typename NativeEventStruct>
    class SECIEventTranslator
    {
    public:
        virtual ~SECIEventTranslator() = default;

        /**
         * @brief Converts native data and posts directly to SECIEventBus.
         */
        virtual bool TranslateAndDispatch(const NativeEventStruct& InNativeMessage) = 0;
    };

} // namespace Sora::SECI_
