#pragma once
#include "ECS/Core.h"
#include "Events/EventReceiver.h"

class Transform;

class SceneGraph
	: public Core<SceneGraph>
	, public EventReceiver
{
public:
	SceneGraph();
	~SceneGraph();

	// Separate init from construction code.
	virtual void Init() final;

	// Each core must update each loop
	virtual void Update(float dt) final;

	virtual void OnEntityAdded(Entity& NewEntity) final;
	EntityHandle RootTransformEntity;
	Transform* GetRootTransform();

	void OnEntityRemoved(Entity& InEntity) override;

#if ME_EDITOR
	virtual void OnEditorInspect() final;
#endif

	void OnEntityDestroyed(Entity& InEntity) override;

private:
	bool OnEvent(const BaseEvent& evt) override;

	Transform* RootTransform = nullptr;

};