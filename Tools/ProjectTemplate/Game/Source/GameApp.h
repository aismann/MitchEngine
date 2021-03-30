#pragma once
#include <Game.h>

class GameApp
	: public Game
{
public:
	GameApp(int argc, char** argv);
	~GameApp();

	virtual void OnInitialize() override;

	virtual void OnStart() override;
	virtual void OnUpdate(float DeltaTime) override;
	virtual void OnEnd() override;

	virtual void PostRender() override;
};