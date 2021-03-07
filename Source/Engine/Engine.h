#pragma once
#include "World.h"
#include "Dementia.h"
#include "Clock.h"
#include "File.h"
#include "Events/EventReceiver.h"
#include "World/Scene.h"
#include "Camera/CameraData.h"
#include <string>
#include "Config.h"
#include "Input.h"
#include "Work/Burst.h"

class Game;
class IWindow;
class BGFXRenderer;

class Engine
	: public EventReceiver
{
public:
	const float FPS = 240.f;
	long long FrameRate;

	Engine();
	~Engine();

	void Init(Game* game);

	void InitGame();

	void StopGame();

	void LoadScene(const std::string& Level);
	
	void Run();
	virtual bool OnEvent(const BaseEvent& evt);

	BGFXRenderer& GetRenderer() const;

	std::weak_ptr<World> GetWorld() const;

	bool IsRunning() const;
	void Quit();
	const bool IsInitialized() const;

	IWindow* GetWindow();

	Game* GetGame() const;

	Config& GetConfig() const;
	Input& GetInput();
#if ME_PLATFORM_UWP || ME_PLATFORM_WIN64
	Burst& GetBurstWorker();
    Burst burst;
#endif

	class CameraCore* Cameras = nullptr;
	class SceneGraph* SceneNodes = nullptr;
	class RenderCore* ModelRenderer = nullptr;
	class AudioCore* AudioThread = nullptr;
	class UICore* UI = nullptr;
	Clock GameClock;
	Moonlight::CameraData EditorCamera;
	Scene* CurrentScene = nullptr;
	float DeltaTime = 0.f;
private:
	Input m_input;
	std::shared_ptr<World> GameWorld;
	bool Running = false;
	IWindow* GameWindow = nullptr;
	class Config* EngineConfig = nullptr;
	Game* m_game = nullptr;
	float AccumulatedTime = 0.0f;
	float FrameTime = 0.0f;
	bool m_isInitialized = false;
	ME_SINGLETON_DEFINITION(Engine)

	BGFXRenderer* NewRenderer = nullptr;
};

Engine& GetEngine();
