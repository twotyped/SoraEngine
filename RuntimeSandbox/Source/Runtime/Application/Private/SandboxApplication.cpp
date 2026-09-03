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
#include <Runtime/Slang/SlangSubsystem.h>
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
		explicit SandboxSubsystem(std::shared_ptr<RHI::RHIDevice> InDevice, std::shared_ptr<SlangSubsystem> InSlangSubsystem)
            : Device(InDevice), pSlangSubsystem(InSlangSubsystem) {}

        virtual ~SandboxSubsystem() override = default;

        virtual bool OnInitialize() override {
            SORA_CLIENT_INFO("SandboxSubsystem initializing pipeline...");

            if (!Device) {
                SORA_CLIENT_ERROR("SandboxSubsystem failed: RHIDevice is null!");
                return false;
            }

     //       std::cout << "Current Working Directory: " 
			  //<< std::filesystem::current_path() << std::endl;

            ShaderCompilationResult VertexResult = pSlangSubsystem->CompileShader("shaders://Triangle.slang", "MainVS");
            ShaderCompilationResult FragmentResult = pSlangSubsystem->CompileShader("shaders://Triangle.slang", "MainFS");

            RHI::RHIShaderModuleDesc VSDesc{};
            VSDesc.Stage = RHI::RHIShaderStage::Vertex;
            VSDesc.Bytecode = std::move(VertexResult.Bytecode);
            VSDesc.EntryPoint = "MainVS";
            auto VS = Device->CreateShaderModule(VSDesc);

            RHI::RHIShaderModuleDesc FSDesc{};
            FSDesc.Stage = RHI::RHIShaderStage::Fragment;
            FSDesc.Bytecode = std::move(FragmentResult.Bytecode);
            FSDesc.EntryPoint = "MainFS";
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
            auto CmdList = Device->CreateCommandList();

            if (!CmdList || !BackBuffer) {
				SORA_ENGINE_ERROR("CommandList or BackBuffer is null!");
				return;
			}

            CmdList->Begin();

            CmdList->PipelineBarrier({
                { BackBuffer, RHI::RHITextureLayout::Undefined, RHI::RHITextureLayout::ColorAttachment }
            });

            RHI::RHIRenderingInfo RenderInfo{};
            RenderInfo.RenderArea = { 0, 0, Swapchain->GetDesc().Width, Swapchain->GetDesc().Height };
            RenderInfo.ColorAttachments = {
                { BackBuffer, true, { 0.05f, 0.05f, 0.05f, 1.0f } }
            };

            RenderInfo.HasDepthAttachment = true;
            RenderInfo.DepthAttachment.Texture = Swapchain->GetDepthTexture();
            RenderInfo.DepthAttachment.ClearOnLoad = true;
            RenderInfo.DepthAttachment.ClearColor = { 1.0f, 0.0f, 0.0f, 0.0f }; 

            CmdList->BeginRendering(RenderInfo);

            CmdList->BindPipeline(Pipeline);
            CmdList->SetViewport({ 0.0f, 0.0f, (float)Swapchain->GetDesc().Width, (float)Swapchain->GetDesc().Height, 0.0f, 1.0f });
            CmdList->SetScissor({ 0, 0, Swapchain->GetDesc().Width, Swapchain->GetDesc().Height });

            CmdList->Draw(3);

            CmdList->EndRendering();

            CmdList->PipelineBarrier({
                { BackBuffer, RHI::RHITextureLayout::ColorAttachment, RHI::RHITextureLayout::Present }
            });

            CmdList->End();

            CmdList->Submit();

            Swapchain->Present();

            Device->EndFrame();
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
        std::shared_ptr<SlangSubsystem> pSlangSubsystem{ nullptr };
        std::shared_ptr<RHI::RHIGraphicsPipeline> Pipeline{ nullptr };
    };
}

SandboxApplication::SandboxApplication() {}

SandboxApplication::~SandboxApplication() {}

void SandboxApplication::OnRegisterSubsystems(Sora::SECI::SECISubsystemRegistry& InRegistry, std::shared_ptr<Sora::RHI::RHIDevice> InDevice, std::shared_ptr<Sora::SlangSubsystem> InSlangSubsystem) {
    InRegistry.RegisterSubsystem<Sora::SandboxSubsystem>(InDevice, InSlangSubsystem);
}

Sora::Application* Sora::CreateApplication() {
    return new SandboxApplication();
}