// 2018 Mitchell Andrews
#pragma once
#include <iostream>
#include <string>

#include "IWindow.h"

#if ME_PLATFORM_WIN64

class D3D12Window final
	: public IWindow
{
public:
	enum class Style : DWORD
	{
		ME_WINDOWED = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		ME_AERO_BORDERLESS = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
		ME_BASIC_BORDERLESS = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX
	};

	D3D12Window(std::string title, int width = WINDOW_WIDTH, int height = WINDOW_HEIGHT);
	~D3D12Window();

	virtual bool ShouldClose() final;
	virtual void ParseMessageQueue() final;
	virtual void Swap() final;
	virtual void SetTitle(const std::string& title) final;

	Vector2 GetSize() const;
	virtual Vector2 GetPosition() final;

	void CanMoveWindow(bool CanMove);
	void SetBorderless(bool enabled);

	auto IsWindowCompositionEnabled() -> bool;

	Style SelectBorderlessStyle();

	void SetBorderlessShadow(bool enabled);
	auto SetShadow(HWND handle, bool enabled) -> void;
	auto AdjustMaximizedClientRect(HWND window, RECT& rect) -> void;
	bool borderless = true;
	bool borderless_shadow = true;
	bool borderless_drag = true;
	bool borderless_resize = true;

	virtual void Minimize() final;
	virtual void ExitMaximize() final;

	bool IsMaximized();

	virtual void Maximize() final;

	LRESULT HitTest(POINT cursor) const;

	virtual void Exit() final;

private:
	bool ExitRequested = false;
	bool canMoveWindow = false;

	bool IsMaximized(HWND hwnd);
	HWND Window;
};

#endif