#pragma once
#include "ECS/Core.h"

#include "Engine/Input.h"
#include "Path.h"
#include "Utils/StringUtils.h"
#include "Events/EventReceiver.h"

class AudioSource;
#ifdef FMOD_ENABLED
namespace FMOD { class System; }
#endif

class AudioCore
	: public Core<AudioCore>
	, public EventReceiver
{
public:
	AudioCore();

	virtual void Update(float dt) final;

	void InitComponent(AudioSource& audioSource);

	virtual void OnEntityAdded(Entity& NewEntity) final;
	virtual void OnEntityRemoved(Entity& NewEntity) final;

	virtual bool OnEvent(const BaseEvent& InEvent) final;

private:
	virtual void Init() override;
	bool IsInitialized = false;

	std::map<std::string, SharedPtr<AudioSource>> m_cachedSounds;

	virtual void OnStart() final;
	void OnStop() final;

#ifdef FMOD_ENABLED
	FMOD::System* system;
#endif
};

ME_REGISTER_CORE(AudioCore);