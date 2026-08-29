#pragma once

#include "Core/Core.h"
#include "Core/SECI/EventLayer/Public/SECIEvent.h"
#include <string>

namespace Sora::SECI
{

	/**
     * @brief Base execution block controlled by SECI to hook submodules into engine update/event loops.
     */
    class SORA_API SECILayer {
    public:
        explicit SECILayer(const std::string& InDebugName = "SECILayer");
        virtual ~SECILayer();

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate() {}
        virtual void OnEvent(SECIEvent& InEvent) {}

        SORA_NO_DISCARD inline const std::string& GetName() const { return DebugName; }

    protected:
        std::string DebugName;
    };
	
} // namespace Sora::SECI
