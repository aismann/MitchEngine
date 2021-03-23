#include "SceneHierarchyWidget.h"
#include <Engine/World.h>
#include <Engine/Engine.h>
#include <optick.h>
#include <EditorApp.h>
#include <Components/Transform.h>
#include <ECS/CoreDetail.h>
#include <Utils/CommonUtils.h>
#include <imgui.h>
#include <HavanaEvents.h>
#include <Events/EventManager.h>
#include <Components/Graphics/Model.h>

SceneHierarchyWidget::SceneHierarchyWidget()
	: HavanaWidget("Hierarchy")
{
	EventManager::GetInstance().RegisterReceiver(this, { InspectEvent::GetEventId(), ClearInspectEvent::GetEventId() });
}

void SceneHierarchyWidget::Init()
{
	App = static_cast<EditorApp*>(GetEngine().GetGame());
}

void SceneHierarchyWidget::Destroy()
{
	App = nullptr;
	Entities = nullptr;
}

bool SceneHierarchyWidget::OnEvent(const BaseEvent& evt)
{
	if (evt.GetEventId() == ClearInspectEvent::GetEventId())
	{
		ClearSelection();
	}
	else if (evt.GetEventId() == InspectEvent::GetEventId())
	{
		const InspectEvent& event = static_cast<const InspectEvent&>(evt);

		SelectedCore = event.SelectedCore;
		SelectedEntity = event.SelectedEntity;
		SelectedTransform = event.SelectedTransform;
	}
	return false;
}

void SceneHierarchyWidget::SetData(Transform* inRoot, std::vector<Entity>& inEntities)
{
	RootTransform = inRoot;
	Entities = &inEntities;
}

void SceneHierarchyWidget::Update()
{

}

void SceneHierarchyWidget::Render()
{
	if (!IsOpen)
	{
		return;
	}

	auto world = GetEngine().GetWorld().lock();

	OPTICK_CATEGORY("Havana::UpdateWorld", Optick::Category::GameLogic);
	ImGui::Begin(Name.c_str(), 0, ImGuiWindowFlags_MenuBar);
	if (!world)
	{
		ImGui::End();
		return;
	}
	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("Create"))
	{
		if (ImGui::BeginMenu("Entity"))
		{
			if (ImGui::IsItemClicked())
			{
				InspectEvent evt;
				evt.SelectedEntity = GetEngine().GetWorld().lock()->CreateEntity();
				evt.Fire();
			}
			if (ImGui::MenuItem("Entity Transform"))
			{
				InspectEvent evt;
				evt.SelectedEntity = GetEngine().GetWorld().lock()->CreateEntity();
				evt.SelectedEntity->AddComponent<Transform>("New Entity");
				evt.Fire();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Core"))
		{
			DrawAddCoreList();
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();
	if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
	{
		OPTICK_CATEGORY("Entity List", Optick::Category::GameLogic);
		if (ImGui::IsWindowFocused())
		{
			if (SelectedTransform && GetEngine().GetEditorInput().IsKeyDown(KeyCode::Delete))
			{
				CommonUtils::RecusiveDelete(SelectedTransform->Parent, SelectedTransform);
				ClearInspectEvent evt;
				evt.Fire();
			}
		}
		UpdateWorldRecursive(RootTransform);
	}
	if (Entities->size() > 0)
	{
		if (ImGui::CollapsingHeader("Utility", ImGuiTreeNodeFlags_DefaultOpen))
		{
			OPTICK_CATEGORY("Utility Entities", Optick::Category::Debug);
			int i = 0;
			for (const Entity& ent : *Entities)
			{
				for (BaseComponent* comp : ent.GetAllComponents())
				{
					ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (SelectedEntity.Get() == &ent ? ImGuiTreeNodeFlags_Selected : 0);
					{
						node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
						ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, comp->GetName().c_str());
						if (ImGui::IsItemClicked())
						{
							InspectEvent evt;
							evt.SelectedEntity = EntityHandle(ent.GetId(), world->GetSharedPtr());
							evt.Fire();
						}
					}
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Entity Cores", ImGuiTreeNodeFlags_DefaultOpen))
	{
		OPTICK_CATEGORY("Entity Cores", Optick::Category::Debug);
		int i = 0;
		for (auto& comp : world->GetAllCoresArray())
		{
			ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (SelectedCore == comp.second.get() ? ImGuiTreeNodeFlags_Selected : 0);
			{
				node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
				ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, comp.second->GetName().c_str());
				if (ImGui::IsItemClicked())
				{
					InspectEvent evt;
					evt.SelectedCore = comp.second.get();
					evt.Fire();
				}
			}
		}
	}
	ImGui::End();
}

void SceneHierarchyWidget::DrawAddCoreList()
{
	ImGui::Text("Cores");
	ImGui::Separator();

	CoreRegistry& reg = GetCoreRegistry();

	for (auto& thing : reg)
	{
		if (ImGui::Selectable(thing.first.c_str()))
		{
			GetEngine().GetWorld().lock()->AddCoreByName(thing.first);
		}
	}
}

void SceneHierarchyWidget::UpdateWorldRecursive(Transform* root)
{
	OPTICK_CATEGORY("UpdateWorld::UpdateWorldRecursive", Optick::Category::GameLogic);
	if (!root)
		return;
	if (ImGui::BeginDragDropTarget())
	{
		HandleAssetDragAndDrop(root);

		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_CHILD_TRANSFORM"))
		{
			DragParentDescriptor.Parent->SetParent(*root);
		}
		ImGui::EndDragDropTarget();
	}

	int i = 0;
	for (SharedPtr<Transform> child : root->GetChildren())
	{
		OPTICK_CATEGORY("UpdateWorld::UpdateWorldRecursive::Child", Optick::Category::GameLogic);
		if (!child)
		{
			continue;
		}
		Transform* var = child.get();
		bool open = false;
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (SelectedTransform == var ? ImGuiTreeNodeFlags_Selected : 0);
		if (var->GetChildren().empty())
		{
			node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
			open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, var->GetName().c_str());
			if (ImGui::IsItemClicked())
			{
				InspectEvent evt;
				evt.SelectedTransform = var;
				evt.SelectedEntity = var->Parent;
				evt.Fire();
			}

			DrawEntityRightClickMenu(var);

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				//files.FullPath = dir.FullPath;
				DragParentDescriptor.Parent = var;
				ImGui::SetDragDropPayload("DND_CHILD_TRANSFORM", &DragParentDescriptor, sizeof(ParentDescriptor));
				ImGui::Text(var->GetName().c_str());
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				HandleAssetDragAndDrop(var);

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_CHILD_TRANSFORM"))
				{
					DragParentDescriptor.Parent->SetParent(*child);
				}
				ImGui::EndDragDropTarget();
			}

			//if (open)
			//{
			//	// your tree code stuff
			//	ImGui::TreePop();
			//}
		}
		else
		{
			OPTICK_CATEGORY("UpdateWorld::UpdateWorldRecursive::Leaf", Optick::Category::GameLogic);
			open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, var->GetName().c_str());
			if (ImGui::IsItemClicked())
			{
				InspectEvent evt;
				evt.SelectedTransform = var;
				evt.SelectedEntity = var->Parent;
				evt.Fire();
			}

			DrawEntityRightClickMenu(var);

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				DragParentDescriptor.Parent = var;
				//files.FullPath = dir.FullPath;
				ImGui::SetDragDropPayload("DND_CHILD_TRANSFORM", &DragParentDescriptor, sizeof(ParentDescriptor));
				ImGui::Text(var->GetName().c_str());
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				HandleAssetDragAndDrop(var);

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_CHILD_TRANSFORM"))
				{
					DragParentDescriptor.Parent->SetParent(*child);
				}
				ImGui::EndDragDropTarget();
			}

			if (open)
			{
				UpdateWorldRecursive(var);
				ImGui::TreePop();
				//ImGui::TreePop();
			}
		}

		i++;
	}
}
void SceneHierarchyWidget::DrawEntityRightClickMenu(Transform* transform)
{
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Add Child"))
		{
			EntityHandle ent = GetEngine().GetWorld().lock()->CreateEntity();
			ent->AddComponent<Transform>().SetParent(*transform);
			GetEngine().GetWorld().lock()->Simulate();
		}

		if (ImGui::MenuItem("Delete", "Del"))
		{
			CommonUtils::RecusiveDelete(transform->Parent, transform);
			GetEngine().GetWorld().lock()->Simulate();
			ClearInspectEvent evt;
			evt.Fire();
		}
		if (ImGui::BeginMenu("Add Component"))
		{
			CommonUtils::DrawAddComponentList(transform->Parent);
			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}
}

void SceneHierarchyWidget::ClearSelection()
{
	SelectedTransform = nullptr;
	SelectedEntity = EntityHandle();
	SelectedCore = nullptr;
}

void SceneHierarchyWidget::HandleAssetDragAndDrop(Transform* root)
{
	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ASSET_BROWSER"))
	{
		IM_ASSERT(payload->DataSize == sizeof(AssetBrowser::AssetDescriptor));
		AssetBrowser::AssetDescriptor payload_n = *(AssetBrowser::AssetDescriptor*)payload->Data;

		if (payload_n.Type == AssetBrowser::AssetType::Model)
		{
			auto ent = GetEngine().GetWorld().lock()->CreateEntity();
			ent->AddComponent<Transform>(payload_n.Name.substr(0, payload_n.Name.find_last_of('.'))).SetParent(*root);
			ent->AddComponent<Model>((payload_n.FullPath.FullPath));
		}
		if (payload_n.Type == AssetBrowser::AssetType::Prefab)
		{
			EntityHandle ent = GetEngine().GetWorld().lock()->CreateFromPrefab(payload_n.FullPath.FullPath, root);
		}
	}
}
