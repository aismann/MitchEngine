#pragma once
#include "ECS/Core.h"

#include "Engine/Input.h"
#include "Path.h"
#include "Utils/StringUtils.h"
#include "Events/EventReceiver.h"

class AudioSource;
//
//namespace DirectX { class SoundEffectInstance; }
//namespace DirectX { class SoundEffect; }
//namespace DirectX { class AudioEngine; }

class AudioCore
	: public Core<AudioCore>
	, public EventReceiver
{
public:
	AudioCore();

	virtual void Update(float dt) final;

	void InitComponent(AudioSource& audioSource);

	virtual void OnEntityAdded(Entity& NewEntity) final;

	virtual bool OnEvent(const BaseEvent& InEvent) final;

private:
	virtual void Init() override;

	//std::unique_ptr<DirectX::AudioEngine> mEngine;

	std::map<std::string, AudioSource> m_cachedSounds;

	virtual void OnStart() final;
	void OnStop() final;
};

ME_REGISTER_CORE(AudioCore);