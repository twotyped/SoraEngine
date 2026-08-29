#include "Runtime/Application/Public/SandboxApplication.h"
#include "Layer/Public/SandboxLayer.h"
#include <Core/Application/Public/EntryPoint.h>

SandboxApplication::SandboxApplication() {
    PushLayer(new SandboxLayer());
}

SandboxApplication::~SandboxApplication() {}

Sora::Application* Sora::CreateApplication() {
    return new SandboxApplication();
}
