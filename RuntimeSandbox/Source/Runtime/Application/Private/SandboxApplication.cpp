#include "Runtime/Application/Public/SandboxApplication.h"
#include <Core/Application/Public/EntryPoint.h>
#include <Core/Logging/Public/Log.h>
#include <Core/SECI/Subsystem/Public/SECISubsystem.h>

namespace Sora
{
    /**
     * @brief Client-side test subsystem replacing the old SandboxLayer.
     */
    class SandboxSubsystem : public SECI::SECISubsystem {
    public:
        SandboxSubsystem() = default;
        virtual ~SandboxSubsystem() override = default;

        virtual bool OnInitialize() override {
            SORA_CLIENT_INFO("SandboxSubsystem initialized!");
            return true;
        }

        virtual void OnUpdate(float deltaTime) override {
            // Frame logic, gameplay updates, or physics ticks
        }

        virtual void OnShutdown() override {
            SORA_CLIENT_INFO("SandboxSubsystem shut down.");
        }

        virtual void OnEvent(SECI::SECIEvent& InEvent) override {
            // Catch sandbox events here
        }

        virtual SECI::SECIEventCategory GetSubscribedEventCategories() const override {
            return SECI::EventCategoryNone;
        }

        virtual std::string GetName() const override { return "SandboxSubsystem"; }
        virtual SECI::SECISubsystemPriority GetPriority() const override { return SECI::SECISubsystemPriority::Normal; }
    };
}

SandboxApplication::SandboxApplication() {}

SandboxApplication::~SandboxApplication() {}

void SandboxApplication::OnRegisterSubsystems(Sora::SECI::SECISubsystemRegistry& InRegistry) {
    // Replaces PushLayer(new SandboxLayer())
    InRegistry.RegisterSubsystem<Sora::SandboxSubsystem>();
}

Sora::Application* Sora::CreateApplication() {
    return new SandboxApplication();
}