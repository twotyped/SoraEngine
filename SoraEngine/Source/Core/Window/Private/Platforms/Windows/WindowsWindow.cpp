#include "Core/Window/Public/Platforms/Windows/WindowsWindow.h"
#include "Core/Logging/Public/Log.h"
#include "Core/SECI/EventLayer/Public/Events/WindowEvent.h"
#include "Core/SECI/EventLayer/Public/Events/KeyEvent.h"
#include "Core/SECI/EventLayer/Public/Events/MouseEvent.h"

namespace Sora
{

	static bool WindowClassRegistered = false;
	static const wchar_t* SORA_WINDOW_CLASS_NAME = L"SoraEngineWindowClass";

	std::unique_ptr<Window> Window::Create(const WindowProperties& InProperties) {
        return std::make_unique<WindowsWindow>(InProperties);
    }

	WindowsWindow::WindowsWindow(const WindowProperties& InProperties) {
        Initialize(InProperties);
    }

    WindowsWindow::~WindowsWindow() {
        Shutdown();
    }

    void WindowsWindow::Initialize(const WindowProperties& InProperties) {
        WindowData.Title = InProperties.Title;
        WindowData.Width = InProperties.Width;
        WindowData.Height = InProperties.Height;

        InstanceHandle = GetModuleHandle(nullptr);

        if (!WindowClassRegistered) {
            WNDCLASSEXW WindowClass = {};

            WindowClass.cbSize = sizeof(WNDCLASSEXW);
            WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            WindowClass.lpfnWndProc = WindowsWindow::WindowProc;
            WindowClass.hInstance = InstanceHandle;
            WindowClass.hCursor = (HCURSOR)LoadImage(
                NULL,
                IDC_ARROW,
                IMAGE_CURSOR,
                0, 0,
                LR_SHARED
            );
            WindowClass.lpszClassName = SORA_WINDOW_CLASS_NAME;

            if (!RegisterClassExW(&WindowClass)) {
                SORA_ENGINE_ERROR("Failed to register Win32 Window Class!");
                return;
            }

            WindowClassRegistered = true;
        }

        RECT WindowRectangle = { 0, 0, static_cast<LONG>(WindowData.Width), static_cast<LONG>(WindowData.Height) };
        AdjustWindowRect(&WindowRectangle, WS_OVERLAPPEDWINDOW, FALSE);

        std::wstring wideTitle(WindowData.Title.begin(), WindowData.Title.end());

        WindowHandle = CreateWindowExW(
            0,
            SORA_WINDOW_CLASS_NAME,
            wideTitle.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            WindowRectangle.right - WindowRectangle.left,
            WindowRectangle.bottom - WindowRectangle.top,
            nullptr,
            nullptr,
            InstanceHandle,
            this
        );

        if (!WindowHandle) {
            SORA_ENGINE_ERROR("Failed to create Win32 HWND!");
            return;
        }

        SetWindowLongPtr(WindowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&WindowData));
        ShowWindow(WindowHandle, SW_SHOW);
        UpdateWindow(WindowHandle);

        SORA_ENGINE_INFO("Created Win32 Window: \'{0}\' ({1}x{2})", WindowData.Title, WindowData.Width, WindowData.Height);
    }

    void WindowsWindow::Shutdown()
    {
        if (WindowHandle) {
            DestroyWindow(WindowHandle);
            WindowHandle = nullptr;
        }
    }

    void WindowsWindow::OnUpdate()
    {
        MSG msg = {};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void WindowsWindow::SetVSync(bool InEnabled)
    {
        WindowData.VSync = InEnabled;
    }

    LRESULT CALLBACK WindowsWindow::WindowProc(HWND InHwnd, UINT InMsg, WPARAM InWParam, LPARAM InLParam)
    {
        WindowDataInternal* data = reinterpret_cast<WindowDataInternal*>(GetWindowLongPtr(InHwnd, GWLP_USERDATA));

        switch (InMsg)
        {
            case WM_SIZE:
            {
                if (data) {
                    data->Width = LOWORD(InLParam);
                    data->Height = HIWORD(InLParam);

                    if (data->EventCallback) {
                        SECI::WindowResizeEvent Event(data->Width, data->Height);
                        data->EventCallback(Event);
                    }
                }
                return 0;
            }
            
            case WM_CLOSE:
            {
                if (data && data->EventCallback) {
                    SECI::WindowCloseEvent Event;
                    data->EventCallback(Event);
                }
                return 0;
            }

            case WM_KILLFOCUS:
			{
				if (data && data->EventCallback) {
					SECI::WindowLostFocusEvent Event;
					data->EventCallback(Event);
				}
				return 0;
			}

            case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            }

            case WM_KEYDOWN:
            {
                if (data && data->EventCallback) {
                    bool IsRepeat = (InLParam & 0x40000000) != 0;
                    SECI::KeyPressedEvent Event(static_cast<KeyCode>(InWParam), IsRepeat);
                    data->EventCallback(Event);
                }
                return 0;
            }

            case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				if (data && data->EventCallback) {
					SECI::KeyReleasedEvent Event(static_cast<KeyCode>(InWParam));
					data->EventCallback(Event);
				}
				return 0;
			}

            case WM_MOUSEMOVE:
			{
				if (data && data->EventCallback) {
					float x = static_cast<float>(LOWORD(InLParam));
					float y = static_cast<float>(HIWORD(InLParam));
					SECI::MouseMovedEvent event(x, y);
					data->EventCallback(event);
				}
				return 0;
			}

            case WM_LBUTTONDOWN:
			{
				if (data && data->EventCallback) {
					SECI::MouseButtonPressedEvent event(Mouse::ButtonLeft);
					data->EventCallback(event);
				}
				return 0;
			}

            case WM_LBUTTONUP:
			{
				if (data && data->EventCallback) {
					SECI::MouseButtonReleasedEvent event(Mouse::ButtonLeft);
					data->EventCallback(event);
				}
				return 0;
			}

			case WM_RBUTTONDOWN:
			{
				if (data && data->EventCallback) {
					SECI::MouseButtonPressedEvent event(Mouse::ButtonRight);
					data->EventCallback(event);
				}
				return 0;
			}

			case WM_RBUTTONUP:
			{
				if (data && data->EventCallback) {
					SECI::MouseButtonReleasedEvent event(Mouse::ButtonRight);
					data->EventCallback(event);
				}
				return 0;
			}

			case WM_MOUSEWHEEL:
			{
				if (data && data->EventCallback) {
					float yOffset = static_cast<float>(GET_WHEEL_DELTA_WPARAM(InWParam)) / WHEEL_DELTA;
					SECI::MouseScrolledEvent event(0.0f, yOffset);
					data->EventCallback(event);
				}
				return 0;
			}
        }

        return DefWindowProcW(InHwnd, InMsg, InWParam, InLParam);
    }

} // namespace Sora