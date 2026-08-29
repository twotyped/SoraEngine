#include "Core/Application/Public/Application.h"
#include "Core/Logging/Public/Log.h"
#include "Core/SECI/EventLayer/Public/SECIEventHandler.h"

namespace Sora
{

	Application::Application() {
		AppWindow = Window::Create(WindowProperties("Sora: Sandbox", 1280, 720));
		
		AppWindow->SetEventCallback([this](SECI::SECIEvent& InEvent) {
			OnEvent(InEvent);
		});
	}

	Application::~Application() {}

	void Application::PushLayer(SECI::SECILayer* InLayer) {
		AppLayerStack.PushLayer(InLayer);
	}

	void Application::PushOverlay(SECI::SECILayer* InOverlay) {
		AppLayerStack.PushOverlay(InOverlay);
	}

	void Application::OnEvent(SECI::SECIEvent& InEvent) {
		SORA_ENGINE_TRACE("{0}", InEvent.ToString());

		SECI::SECIEventHandler dispatcher(InEvent);

        dispatcher.Dispatch<SECI::WindowCloseEvent>([this](SECI::WindowCloseEvent& e) {
            return OnWindowClose(e);
        });

        dispatcher.Dispatch<SECI::WindowResizeEvent>([this](SECI::WindowResizeEvent& e) {
            return OnWindowResize(e);
        });

		for (auto it = AppLayerStack.rbegin(); it != AppLayerStack.rend(); ++it)
        {
            if (InEvent.Handled)
            {
                break;
            }
            (*it)->OnEvent(InEvent);
        }
	}

	bool Application::OnWindowClose(SECI::WindowCloseEvent& InEvent) {
        IsRunning = false;
        return true;
    }

    bool Application::OnWindowResize(SECI::WindowResizeEvent& InEvent) {
        SORA_ENGINE_INFO("Resizing graphics viewport to {0}x{1}", InEvent.GetWidth(), InEvent.GetHeight());
        return true;
    }

	void Application::Run() {
		while (IsRunning) {
			for (SECI::SECILayer* Layer : AppLayerStack) {
				Layer->OnUpdate();
			}
			AppWindow->OnUpdate();
		}
	}

} // namespace Sora
