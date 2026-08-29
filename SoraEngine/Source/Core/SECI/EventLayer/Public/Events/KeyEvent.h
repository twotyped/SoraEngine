#pragma once

#include "Core/Core.h"
#include "Core/SECI/EventLayer/Public/SECIEvent.h"
#include "Core/Input/Public/KeyCodes.h"

namespace Sora::SECI
{

	class SORA_API KeyEvent : public SECIEvent {
	public:
		SORA_NO_DISCARD inline KeyCode GetKeyCode() const { return Code; }

		SECI_EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput);

	protected:
		explicit KeyEvent(KeyCode InCode)
            : Code(InCode) {}

        KeyCode Code;
	};

	class SORA_API KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(KeyCode InCode, bool InIsRepeat = false);

		SORA_NO_DISCARD inline bool IsRepeat() const { return IsRepeating; }
		SORA_NO_DISCARD std::string ToString() const override;

		SECI_EVENT_CLASS_TYPE(KeyPressed);

	private:
		bool IsRepeating = false;
	};

	class SORA_API KeyReleasedEvent : public KeyEvent {
	public:
		KeyReleasedEvent(KeyCode InCode);

		SORA_NO_DISCARD std::string ToString() const override;

		SECI_EVENT_CLASS_TYPE(KeyReleased);
	};

} // namespace Sora::SECI
