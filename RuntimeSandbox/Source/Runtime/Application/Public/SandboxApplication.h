#pragma once

#include <Core/Application/Public/Application.h>
#include <Runtime/RHI/Public/RHIDevice.h>

#ifndef SORA_DEBUG
    #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

class SandboxApplication : public Sora::Application {
public:
    SandboxApplication();
    virtual ~SandboxApplication() override;

protected:
    virtual void OnRegisterSubsystems(Sora::SECI::SECISubsystemRegistry& InRegistry, std::shared_ptr<Sora::RHI::RHIDevice> InDevice, std::shared_ptr<Sora::SlangSubsystem> InSlangSubsystem, std::shared_ptr<Sora::RenderingSubsystem> InRenderingSubsystem) override;
};
