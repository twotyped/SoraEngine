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

        RAWINPUTDEVICE rid;
        rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
        rid.usUsage = HID_USAGE_GENERIC_MOUSE;
        rid.dwFlags = 0; // Use 0 for normal focus-only input (RIDEV_INPUTSINK receives mouse moves even when tabbed out)
        rid.hwndTarget = WindowHandle;

        if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
            SORA_ENGINE_ERROR("Failed to register Raw Input device!");
        }

        SetWindowLongPtr(WindowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        ShowWindow(WindowHandle, SW_SHOW);
        UpdateWindow(WindowHandle);

        bool targetVisibility = WindowData.CursorVisible;
		bool targetClipping = WindowData.CursorClipped;

		WindowData.CursorVisible = !targetVisibility;
		WindowData.CursorClipped = !targetClipping;

		ModifyCursorVisibility(targetVisibility);
		ModifyCursorClipping(targetClipping);

        SORA_ENGINE_INFO("Created Win32 Window: \'{0}\' ({1}x{2})", WindowData.Title, WindowData.Width, WindowData.Height);
    }

    void WindowsWindow::Shutdown() {
        if (WindowHandle) {
            DestroyWindow(WindowHandle);
            WindowHandle = nullptr;
        }
    }

    void WindowsWindow::OnUpdate() {
        MSG msg = {};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		if (GetFocus() == WindowHandle && WindowData.CursorClipped) {
			CenterCursor();
		}
    }

    void WindowsWindow::SetVSync(bool InEnabled) {
        WindowData.VSync = InEnabled;
    }

    void WindowsWindow::ModifyCursorVisibility(bool InCursorVisible) {
		if (WindowData.CursorVisible == InCursorVisible) return;

		if (InCursorVisible) {
			while (ShowCursor(TRUE) < 0);
		} else {
			while (ShowCursor(FALSE) >= 0);
		}

		WindowData.CursorVisible = InCursorVisible;
    }

    void WindowsWindow::ModifyCursorClipping(bool InCursorClipped) {
		if (WindowData.CursorClipped == InCursorClipped) return;

		if (InCursorClipped) {
			RECT clientRect;
			GetClientRect(WindowHandle, &clientRect);

			POINT topLeft = { clientRect.left, clientRect.top };
			POINT bottomRight = { clientRect.right, clientRect.bottom };

			ClientToScreen(WindowHandle, &topLeft);
			ClientToScreen(WindowHandle, &bottomRight);

			RECT screenRect = { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
			ClipCursor(&screenRect);

			CenterCursor(); 
		} else {
			ClipCursor(NULL);
		}

		WindowData.CursorClipped = InCursorClipped;
    }

    void WindowsWindow::CenterCursor() {
        if (!WindowData.CursorClipped || !WindowHandle) return;

        RECT clientRect;
		GetClientRect(WindowHandle, &clientRect);

		POINT center = { 
			(clientRect.right - clientRect.left) / 2, 
			(clientRect.bottom - clientRect.top) / 2 
		};

		ClientToScreen(WindowHandle, &center);
		SetCursorPos(center.x, center.y);
    }

    LRESULT CALLBACK WindowsWindow::WindowProc(HWND InHwnd, UINT InMsg, WPARAM InWParam, LPARAM InLParam) {
		WindowsWindow* window = reinterpret_cast<WindowsWindow*>(GetWindowLongPtr(InHwnd, GWLP_USERDATA));

        switch (InMsg) {
            case WM_SIZE: {
                if (window) {
                    window->WindowData.Width = LOWORD(InLParam);
                    window->WindowData.Height = HIWORD(InLParam);

                    if (window->WindowData.CursorClipped) {
						window->ModifyCursorClipping(true);
					}

                    if (window->WindowData.EventCallback) {
                        SECI::WindowResizeEvent Event(window->WindowData.Width, window->WindowData.Height);
                        window->WindowData.EventCallback(Event);
                    }
                }
                return 0;
            }
            
            case WM_CLOSE: {
                if (window && window->WindowData.EventCallback) {
                    SECI::WindowCloseEvent Event;
                    window->WindowData.EventCallback(Event);
                }
                DestroyWindow(InHwnd);
                return 0;
            }

			case WM_SETFOCUS: {
				if (window) {
					if (window->WindowData.EventCallback) {
						SECI::WindowFocusEvent Event;
						window->WindowData.EventCallback(Event);
					}

					// Re-apply Win32 clip & show state directly
					if (window->WindowData.CursorClipped) {
						RECT clientRect;
						GetClientRect(InHwnd, &clientRect);
						POINT topLeft = { clientRect.left, clientRect.top };
						POINT bottomRight = { clientRect.right, clientRect.bottom };
						ClientToScreen(InHwnd, &topLeft);
						ClientToScreen(InHwnd, &bottomRight);

						RECT screenRect = { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
						ClipCursor(&screenRect);
						window->CenterCursor();
					}

					if (!window->WindowData.CursorVisible) {
						while (ShowCursor(FALSE) >= 0);
					}
				}
				return 0;
			}

			case WM_KILLFOCUS: {
				if (window) {
					if (window->WindowData.EventCallback) {
						SECI::WindowLostFocusEvent Event;
						window->WindowData.EventCallback(Event);
					}

					// Release hardware cursor to desktop on alt-tab
					ClipCursor(NULL);
					while (ShowCursor(TRUE) < 0);
				}
				return 0;
			}

            case WM_DESTROY: {
                SetWindowLongPtr(InHwnd, GWLP_USERDATA, 0);
                PostQuitMessage(0);
                return 0;
            }

            case WM_KEYDOWN: {
                if (window && window->WindowData.EventCallback) {
                    bool IsRepeat = (InLParam & 0x40000000) != 0;
                    SECI::KeyPressedEvent Event(static_cast<KeyCode>(InWParam), IsRepeat);
                    window->WindowData.EventCallback(Event);
                }
                return 0;
            }

            case WM_KEYUP:
			case WM_SYSKEYUP: {
				if (window && window->WindowData.EventCallback) {
					SECI::KeyReleasedEvent Event(static_cast<KeyCode>(InWParam));
					window->WindowData.EventCallback(Event);
				}
				return 0;
			}

   //         case WM_MOUSEMOVE: {
			//	if (window && window->WindowData.EventCallback) {
			//		float x = static_cast<float>(LOWORD(InLParam));
			//		float y = static_cast<float>(HIWORD(InLParam));
			//		SECI::MouseMovedEvent event(x, y);
			//		window->WindowData.EventCallback(event);
			//	}
			//	return 0;
			//}

			case WM_INPUT: {
				UINT dwSize = sizeof(RAWINPUT);
				static BYTE lpb[sizeof(RAWINPUT)];

				if (GetRawInputData((HRAWINPUT)InLParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) == (UINT)-1) {
					break;
				}

				RAWINPUT* raw = (RAWINPUT*)lpb;

				if (raw->header.dwType == RIM_TYPEMOUSE) {
					// Ensure the input is relative movement, not absolute positioning
					if ((raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0) {
						int xRel = raw->data.mouse.lLastX;
						int yRel = raw->data.mouse.lLastY;

						if ((xRel != 0 || yRel != 0) && window && window->WindowData.EventCallback) {
							SECI::MouseMovedEvent event(static_cast<float>(xRel), static_cast<float>(yRel));
							window->WindowData.EventCallback(event);
						}
					}
				}
				break;
			}

            case WM_LBUTTONDOWN: {
				if (window && window->WindowData.EventCallback) {
					SECI::MouseButtonPressedEvent event(Mouse::ButtonLeft);
					window->WindowData.EventCallback(event);
				}
				return 0;
			}

            case WM_LBUTTONUP: {
				if (window && window->WindowData.EventCallback) {
					SECI::MouseButtonReleasedEvent event(Mouse::ButtonLeft);
					window->WindowData.EventCallback(event);
				}
				return 0;
			}

			case WM_RBUTTONDOWN: {
				if (window && window->WindowData.EventCallback) {
					SECI::MouseButtonPressedEvent event(Mouse::ButtonRight);
					window->WindowData.EventCallback(event);
				}
				return 0;
			}

			case WM_RBUTTONUP: {
				if (window && window->WindowData.EventCallback) {
					SECI::MouseButtonReleasedEvent event(Mouse::ButtonRight);
					window->WindowData.EventCallback(event);
				}
				return 0;
			}

			case WM_MOUSEWHEEL: {
				if (window && window->WindowData.EventCallback) {
					float yOffset = static_cast<float>(GET_WHEEL_DELTA_WPARAM(InWParam)) / WHEEL_DELTA;
					SECI::MouseScrolledEvent event(0.0f, yOffset);
					window->WindowData.EventCallback(event);
				}
				return 0;
			}
        }

        return DefWindowProcW(InHwnd, InMsg, InWParam, InLParam);
    }

} // namespace Sora