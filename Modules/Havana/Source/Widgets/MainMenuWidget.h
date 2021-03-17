#pragma once
#include <HavanaWidget.h>
#include <functional>
#include <unordered_map>
#include <string>
#include <Pointers.h>
#include <Commands/CommandManager.h>
#include <Math/Vector2.h>
#include <Path.h>
#include <JSON.h>

class EditorApp;

namespace Moonlight { class Texture; }

class MainMenuWidget
	: public HavanaWidget
{
public:
	MainMenuWidget();

	void Init() final;
	void Destroy() final;

	void Update() final;
	void Render() final;

	void SetData(std::vector<SharedPtr<HavanaWidget>>* widgetList, EditorApp* editorApp);

	void SetWindowTitle(const std::string& title);

	void DrawOpenFilePopup();

	void SetCallbacks(std::function<void()> StartGame, std::function<void()> PauseGame, std::function<void()> StopGame);

	bool MaximizeOnPlay = false;

private:
	// these should be events
	std::function<void()> StartGameFunc;
	std::function<void()> PauseGameFunc;
	std::function<void()> StopGameFunc;

	EditorApp* App = nullptr;
	std::vector<SharedPtr<HavanaWidget>>* WidgetList = nullptr;

	std::unordered_map<std::string, SharedPtr<Moonlight::Texture>> Icons;
	std::string WindowTitle;
	ImVec2 MainMenuSize;
	Vector2 TitleBarDragPosition;
	Vector2 TitleBarDragSize;
	bool OpenScene = false;
	CommandManager EditorCommands;
	bool ShowDemoWindow = false;
	Path CurrentDirectory;
	json AssetDirectory;

	void BrowseDirectory(const Path& path);
};