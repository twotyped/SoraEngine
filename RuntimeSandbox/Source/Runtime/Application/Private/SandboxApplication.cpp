#include "Runtime/Application/Public/SandboxApplication.h"
#include <Core/Application/Public/EntryPoint.h>
#include <Core/Logging/Public/Log.h>
#include <Core/SECI/Subsystem/Public/SECISubsystem.h>

#include <Runtime/RHI/Public/RHIDevice.h>
#include <Runtime/RHI/Public/RHISwapchain.h>
#include <Runtime/RHI/Public/RHICommandList.h>
#include <Runtime/RHI/Public/Pipeline/RHIPipeline.h>
#include <Runtime/RHI/Public/Pipeline/RHIShaderModule.h>

#include <Runtime/RHI/Public/Vulkan/VulkanDevice.h>

#include "Runtime/Shaders/Public/ShaderHelpers.h"

#include <filesystem>
#include <iostream>

namespace Sora
{
    /**
     * @brief Client-side test subsystem replacing the old SandboxLayer.
     */
    class SandboxSubsystem : public SECI::SECISubsystem {
    public:
        SandboxSubsystem() = default;
		explicit SandboxSubsystem(std::shared_ptr<RHI::RHIDevice> InDevice)
            : Device(InDevice) {}

        virtual ~SandboxSubsystem() override = default;

        virtual bool OnInitialize() override {
            SORA_CLIENT_INFO("SandboxSubsystem initializing pipeline...");

            if (!Device) {
                SORA_CLIENT_ERROR("SandboxSubsystem failed: RHIDevice is null!");
                return false;
            }

            std::cout << "Current Working Directory: " 
			  << std::filesystem::current_path() << std::endl;

			// Check if the file actually exists from the program's perspective
			if (!std::filesystem::exists("Shaders/TriangleVS.spv")) {
				std::cerr << "File NOT FOUND at: " 
						  << std::filesystem::absolute("Shaders/TriangleVS.spv") << std::endl;
			}

            auto VSCodes = Shader::LoadSPIRV("Shaders/TriangleVS.spv");
            auto FSCodes = Shader::LoadSPIRV("Shaders/TriangleFS.spv");

            if (VSCodes.empty() || FSCodes.empty()) {
                SORA_CLIENT_ERROR("Failed to load SPIR-V shader files!");
                return false;
            }

            RHI::RHIShaderModuleDesc VSDesc{};
            VSDesc.Stage = RHI::RHIShaderStage::Vertex;
            VSDesc.Bytecode = VSCodes;
            VSDesc.EntryPoint = "main";
            auto VS = Device->CreateShaderModule(VSDesc);

            RHI::RHIShaderModuleDesc FSDesc{};
            FSDesc.Stage = RHI::RHIShaderStage::Fragment;
            FSDesc.Bytecode = FSCodes;
            FSDesc.EntryPoint = "main";
            auto FS = Device->CreateShaderModule(FSDesc);

            RHI::RHIGraphicsPipelineDesc PipelineDesc{};
            PipelineDesc.VertexShader = VS;
            PipelineDesc.FragmentShader = FS;
            PipelineDesc.Topology = RHI::RHIPrimitiveTopology::TriangleList;
            PipelineDesc.VertexLayout.Bindings = {}; 
            PipelineDesc.VertexLayout.Attributes = {};

            PipelineDesc.ColorAttachmentFormats = { RHI::RHIFormat::B8G8R8A8_UNORM };

            Pipeline = Device->CreateGraphicsPipeline(PipelineDesc);

            SORA_CLIENT_INFO("SandboxSubsystem initialized!");
            return true;
        }

        virtual void OnUpdate(float deltaTime) override {
			if (!Device || !Pipeline) return;

			auto Swapchain = Device->GetSwapchain();
			
			if (!Swapchain) {
				Swapchain = Device->CreateSwapchain(nullptr, 1280, 720); 
				if (!Swapchain) return;
			}

			if (!Swapchain->AcquireNextImage()) {
				Swapchain->Resize(Swapchain->GetDesc().Width, Swapchain->GetDesc().Height);
				return;
			}

			Device->BeginFrame();

            auto BackBuffer = Swapchain->GetCurrentBackBuffer();
            auto CmdList = Device->GetMainCommandList();

            // 2. Transition Swapchain image to ColorAttachment layout
            CmdList->PipelineBarrier({
                { BackBuffer, RHI::RHITextureLayout::Undefined, RHI::RHITextureLayout::ColorAttachment }
            });

            // 3. Dynamic Rendering Scope (Clear to dark gray)
            RHI::RHIRenderingInfo RenderInfo{};
            RenderInfo.RenderArea = { 0, 0, Swapchain->GetDesc().Width, Swapchain->GetDesc().Height };
            RenderInfo.ColorAttachments = {
                { BackBuffer, true, { 0.05f, 0.05f, 0.05f, 1.0f } }
            };

            CmdList->BeginRendering(RenderInfo);

            // 4. Bind Pipeline & Dynamic Viewport/Scissor States
            CmdList->BindPipeline(Pipeline);
            CmdList->SetViewport({ 0.0f, 0.0f, (float)Swapchain->GetDesc().Width, (float)Swapchain->GetDesc().Height, 0.0f, 1.0f });
            CmdList->SetScissor({ 0, 0, Swapchain->GetDesc().Width, Swapchain->GetDesc().Height });

            // 5. Procedural Draw (3 vertices using SV_VertexID in Slang)
            CmdList->Draw(3);

            CmdList->EndRendering();

            // 6. Transition Swapchain image to Present layout
            CmdList->PipelineBarrier({
                { BackBuffer, RHI::RHITextureLayout::ColorAttachment, RHI::RHITextureLayout::Present }
            });

            // 7. Submit Work & Present
            Device->EndFrame();

            auto VkDev = std::dynamic_pointer_cast<RHI::VulkanDevice>(Device);
            Swapchain->Present();
        }

        virtual void OnShutdown() override {
            Pipeline.reset();
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

    private:
        std::shared_ptr<RHI::RHIDevice> Device{ nullptr };
        std::shared_ptr<RHI::RHIGraphicsPipeline> Pipeline{ nullptr };
    };
}

SandboxApplication::SandboxApplication() {}

SandboxApplication::~SandboxApplication() {}

void SandboxApplication::OnRegisterSubsystems(Sora::SECI::SECISubsystemRegistry& InRegistry, std::shared_ptr<Sora::RHI::RHIDevice> InDevice) {
    InRegistry.RegisterSubsystem<Sora::SandboxSubsystem>(InDevice);
}

Sora::Application* Sora::CreateApplication() {
    return new SandboxApplication();
}