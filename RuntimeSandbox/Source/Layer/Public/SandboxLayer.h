#pragma once

#include "Core/SECI/Layer/Public/SECILayer.h"
#include "Core/SECI/EventLayer/Public/SECIEvent.h"
#include "Core/SECI/EventLayer/Public/Events/KeyEvent.h"
#include "Core/SECI/EventLayer/Public/Events/MouseEvent.h"

class SandboxLayer : public Sora::SECI::SECILayer {
public:
    SandboxLayer();
    virtual ~SandboxLayer() override = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate() override;
    void OnEvent(Sora::SECI::SECIEvent& InEvent) override;

private:
    bool OnKeyPressed(Sora::SECI::KeyPressedEvent& InEvent);
    bool OnMouseButtonPressed(Sora::SECI::MouseButtonPressedEvent& InEvent);
};