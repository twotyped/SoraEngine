#pragma once

#include <Core/Application/Public/Application.h>

#ifndef SORA_DEBUG
    #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

class SandboxApplication : public Sora::Application {
public:
    SandboxApplication();
    virtual ~SandboxApplication() override;

protected:
    virtual void OnRegisterSubsystems(Sora::SECI::SECISubsystemRegistry& InRegistry) override;
};
