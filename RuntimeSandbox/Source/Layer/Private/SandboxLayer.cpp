#include "Layer/Public/SandboxLayer.h"
#include "Core/Logging/Public/Log.h"
#include "Core/SECI/EventLayer/Public/SECIEventHandler.h"

SandboxLayer::SandboxLayer()
    : SECILayer("SandboxLayer") {}

void SandboxLayer::OnAttach() {
    SORA_CLIENT_INFO("SandboxLayer Attached!");
}

void SandboxLayer::OnDetach() {
    SORA_CLIENT_INFO("SandboxLayer Detached!");
}

void SandboxLayer::OnUpdate() {
    // Layer tick logic (e.g., game object updates)
}

void SandboxLayer::OnEvent(Sora::SECI::SECIEvent& InEvent) {
    Sora::SECI::SECIEventHandler dispatcher(InEvent);

    dispatcher.Dispatch<Sora::SECI::KeyPressedEvent>([this](Sora::SECI::KeyPressedEvent& e) {
        return OnKeyPressed(e);
    });
}

bool SandboxLayer::OnKeyPressed(Sora::SECI::KeyPressedEvent& InEvent) {
    if (InEvent.GetKeyCode() == Sora::Key::Space)
    {
        SORA_CLIENT_TRACE("Spacebar pressed inside SandboxLayer!");
        return true; // Mark event as handled so lower layers don't process it
    }

    return false;
}
