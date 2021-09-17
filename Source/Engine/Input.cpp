#include "PCH.h"

#if ME_PLATFORM_UWP || ME_PLATFORM_WIN64
#include <WinUser.h>
#endif

#include "Engine/Input.h"
#include "CLog.h"
#include <string>
#include <iostream>
#include "SDL.h"

#pragma region Class

Input::Input()
{
	KeyboardState = SDL_GetKeyboardState(nullptr);
	MouseState = SDL_GetMouseState(nullptr, nullptr);// use these params

	std::vector<TypeId> events;
	events.push_back(MouseScrollEvent::GetEventId());
	EventManager::GetInstance().RegisterReceiver(this, events);

	PreviousKeyboardState = static_cast<const uint8_t*>(malloc(sizeof(uint8_t) * SDL_NUM_SCANCODES));
	KeyboardState = PreviousKeyboardState;
}

bool Input::OnEvent(const BaseEvent& evt)
{
	if (CaptureInput)
	{
		if (evt.GetEventId() == MouseScrollEvent::GetEventId())
		{
			const MouseScrollEvent& event = static_cast<const MouseScrollEvent&>(evt);
			MouseScroll = MouseScroll + event.Scroll;
		}
	}
	return false;
}

void Input::Pause()
{
	CaptureInput = false;
}

void Input::Resume()
{
	CaptureInput = true;
	SetMouseCapture(WantsToCaptureMouse);
}

void Input::Stop()
{
	//Mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	CaptureInput = false;
}

void Input::Update()
{
	if (CaptureInput)
	{
		KeyboardState = SDL_GetKeyboardState(nullptr);
		int mouseX = 0;
		int mouseY = 0;
		MouseState = SDL_GetMouseState(&mouseX, &mouseY);
		MousePosition = Vector2(mouseX, mouseY);
	}
}

void Input::PostUpdate()
{
	PreviousKeyboardState = static_cast<const uint8_t*>(memcpy((void*)PreviousKeyboardState, (void*)KeyboardState, sizeof(uint8_t) * SDL_NUM_SCANCODES));
	PreviousMouseState = MouseState;
}

#pragma endregion

#pragma region KeyboardInput

bool Input::IsKeyDown(KeyCode key)
{
	return KeyboardState[(uint32_t)key];
}

bool Input::WasKeyPressed(KeyCode key)
{
	return !PreviousKeyboardState[(uint32_t)key] && KeyboardState[(uint32_t)key];
}

bool Input::WasKeyReleased(KeyCode key)
{
	return PreviousKeyboardState[(uint32_t)key] && !KeyboardState[(uint32_t)key];
}

#pragma endregion

#pragma region MouseInput

Vector2 Input::GetMousePosition() const
{
	return MousePosition;
}

void Input::SetMousePosition(const Vector2& InPosition)
{
	if (CaptureInput)
	{
#if ME_EDITOR && ME_PLATFORM_WIN64
		Vector2 pos = Offset + InPosition;
		SetCursorPos(static_cast<int>(pos.x), static_cast<int>(pos.y));
#endif
		Update();
	}
}

Vector2 Input::GetMouseOffset()
{
	return Offset;
}

Vector2 Input::GetMouseScrollOffset()
{
	return MouseScroll;
}

void Input::SetMouseCapture(bool Capture)
{
	if (CaptureInput)
	{
		if (Capture)
		{
			//SDL_SCANCODE_RETURN;
			//Mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
		}
		else
		{
			//Mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
		}
		WantsToCaptureMouse = Capture;
	}
}

void Input::SetMouseOffset(const Vector2& InOffset)
{
	Offset = InOffset;
}

bool Input::IsMouseButtonDown(MouseButton mouseButton)
{
	return MouseState & SDL_BUTTON((uint32_t)mouseButton);
}

#pragma endregion
