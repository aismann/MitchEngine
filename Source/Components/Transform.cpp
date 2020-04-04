#include "PCH.h"
#include "Transform.h"
#include <algorithm>
#include "Math/Vector3.h"
#include "misc/cpp/imgui_stdlib.h"
#include "Engine/Engine.h"
#include "Mathf.h"
#include <SimpleMath.h>

Transform::Transform()
	: Component("Transform")
	, WorldTransform(DirectX::XMMatrixIdentity())
	, Position(0.f, 0.f, 0.f)
	, Rotation(0.f, 0.f, 0.f)
	, Scale(1.0f, 1.0f, 1.0f)
{
}


Transform::Transform(const std::string& TransformName)
	: Component("Transform")
	, WorldTransform(DirectX::XMMatrixIdentity())
	, Name(std::move(TransformName))
	, Position(0.f, 0.f, 0.f)
	, Rotation(0.f, 0.f, 0.f)
	, Scale(1.0f, 1.0f, 1.0f)
{
}

Transform::~Transform()
{
	if (ParentTransform)
	{
		ParentTransform->RemoveChild(this);
		ParentTransform = nullptr;
	}
}

void Transform::SetPosition(Vector3 NewPosition)
{
	WorldTransform.GetInternalMatrix().Translation((Position - NewPosition).GetInternalVec());
	Position = NewPosition;
	SetDirty(true);
	//UpdateRecursively(this);
}


void Transform::SetScale(Vector3 NewScale)
{
	if (NewScale == Vector3())
	{
		return;
	}
	Scale = NewScale;
	SetDirty(true);
}

void Transform::SetScale(float NewScale)
{
	if (NewScale == Scale.X() && NewScale == Scale.Y() && NewScale == Scale.Z())
	{
		return;
	}
	Scale = Vector3(NewScale);
	SetDirty(true);
}

void Transform::Translate(Vector3 NewPosition)
{
	if (NewPosition == Vector3())
	{
		return;
	}
	SetWorldPosition(Position + NewPosition);
}

Vector3 Transform::Front()
{
	float mat1 = -WorldTransform.GetInternalMatrix()(2, 0);
	float mat2 = -WorldTransform.GetInternalMatrix()(2, 1);
	float mat3 = -WorldTransform.GetInternalMatrix()(2, 2);
	return Vector3(mat1, mat2, mat3);
}

Vector3& Transform::GetPosition()
{
	return Position;
}

void Transform::UpdateRecursively(Transform* CurrentTransform)
{
	OPTICK_EVENT("SceneGraph::UpdateRecursively");
	for (Transform* Child : CurrentTransform->Children)
	{
		if (Child->m_isDirty)
		{
			OPTICK_EVENT("SceneGraph::Update::IsDirty");
			//Quaternion quat = Quaternion(Child->Rotation);
			DirectX::SimpleMath::Matrix id = DirectX::XMMatrixIdentity();
			DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateFromQuaternion(DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(Child->Rotation[1], Child->Rotation[0], Child->Rotation[2]));// , Child->Rotation.Y(), Child->Rotation.Z());
			DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(Child->GetScale().GetInternalVec());
			DirectX::SimpleMath::Matrix pos = XMMatrixTranslationFromVector(Child->GetPosition().GetInternalVec());
			Child->SetWorldTransform(Matrix4((scale * rot * pos) * CurrentTransform->WorldTransform.GetInternalMatrix()));
		}
		UpdateRecursively(Child);
	}
}

void Transform::UpdateWorldTransform()
{
	DirectX::SimpleMath::Matrix id = DirectX::XMMatrixIdentity();
	DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateFromQuaternion(DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(Rotation[1], Rotation[0], Rotation[2]));// , Child->Rotation.Y(), Child->Rotation.Z());
	DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(GetScale().GetInternalVec());
	DirectX::SimpleMath::Matrix pos = XMMatrixTranslationFromVector(GetPosition().GetInternalVec());
	SetWorldTransform(Matrix4((scale * rot * pos)));

	UpdateRecursively(this);
}

Vector3 Transform::GetWorldPosition()
{
	return WorldTransform.GetPosition();
}

void Transform::SetWorldPosition(const Vector3& NewPosition)
{
	DirectX::SimpleMath::Matrix& mat = WorldTransform.GetInternalMatrix();

	mat._41 = NewPosition[0];
	mat._42 = NewPosition[1];
	mat._43 = NewPosition[2];

	Position += Vector3(mat._41 - Position[0], mat._42 - Position[1], mat._43 - Position[2]);

	SetDirty(true);
}

void Transform::Reset()
{
	SetWorldPosition(Vector3());
	SetRotation(Vector3());
	SetScale(1.f);
}

void Transform::SetWorldTransform(Matrix4& NewWorldTransform, bool InIsDirty)
{
	WorldTransform = std::move(NewWorldTransform);
	{
		DirectX::SimpleMath::Quaternion quat;
		DirectX::SimpleMath::Vector3 pos;
		DirectX::SimpleMath::Vector3 scale;
		NewWorldTransform.GetInternalMatrix().Decompose(scale, quat, pos);

		//InternalRotation = Quaternion(ParentTransform->GetWorldRotation().GetInternalVec() * Quaternion(quat).GetInternalVec());
		//Vector3 rotation = Quaternion::ToEulerAngles(Quaternion(quat));
		//Rotation = Vector3(Mathf::Degrees(Rotation.X()), Mathf::Degrees(Rotation.Y()), Mathf::Degrees(Rotation.Z()));
		//Position = Vector3(pos);
	}

	// update local transform
	WorldTransform = std::move(NewWorldTransform);
	SetDirty(InIsDirty);
}

const bool Transform::IsDirty() const
{
	return m_isDirty;
}

Transform* Transform::GetParent()
{
	return ParentTransform;
}

Matrix4 Transform::GetLocalToWorldMatrix()
{
	DirectX::SimpleMath::Matrix id = DirectX::XMMatrixIdentity();
	DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateFromYawPitchRoll(Rotation.Y(), Rotation.X(), Rotation.Z());// , Child->Rotation.Y(), Child->Rotation.Z());
	DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(Scale.GetInternalVec());
	DirectX::SimpleMath::Matrix pos = XMMatrixTranslationFromVector(Position.GetInternalVec());
	if (ParentTransform)
	{
		return Matrix4((scale * rot * pos) * ParentTransform->WorldTransform.GetInternalMatrix());
	}
	else
	{
		return Matrix4(scale * rot * pos);
	}
}

Matrix4 Transform::GetWorldToLocalMatrix()
{
	DirectX::SimpleMath::Matrix mat;
	GetLocalToWorldMatrix().GetInternalMatrix().Invert(mat);
	return Matrix4(mat);
}

void Transform::Init()
{
}

#if ME_EDITOR
void Transform::OnEditorInspect()
{
	ImGui::InputText("Name", &Name);

	Vector3 OldPosition = Position;
	HavanaUtils::EditableVector3("Position", Position);
	Vector3 OldRotation = Rotation;
	HavanaUtils::EditableVector3("Rotation", Rotation);
	if (OldRotation != Rotation || OldPosition != Position)
	{
		SetRotation(Rotation);
	}
	HavanaUtils::EditableVector3("Scale", Scale);
	Vector3 WorldPos = GetWorldPosition();
	HavanaUtils::EditableVector3("World Position", WorldPos);
	if (WorldPos != GetWorldPosition())
	{
		SetWorldPosition(WorldPos);
	}

	if (ImGui::Button("Reset Transform"))
	{
		Reset();
	}
}
#endif

void Transform::SetDirty(bool Dirty)
{
	if (Dirty && (Dirty != m_isDirty))
	{
		for (Transform* Child : Children)
		{
			Child->SetDirty(Dirty);
		}
	}
	m_isDirty = Dirty;
}

Vector3 Transform::GetScale()
{
	return Scale;
}

void Transform::LookAt(const Vector3& InDirection)
{
	Vector3 worldPos = GetWorldPosition();
	WorldTransform = Matrix4(DirectX::SimpleMath::Matrix::CreateLookAt(worldPos.GetInternalVec(), (GetWorldPosition() + InDirection).GetInternalVec(), Vector3::Up.GetInternalVec()).Transpose());

	WorldTransform.GetInternalMatrix()._41 = worldPos[0];
	WorldTransform.GetInternalMatrix()._42 = worldPos[1];
	WorldTransform.GetInternalMatrix()._43 = worldPos[2];

	DirectX::SimpleMath::Quaternion quat;
	WorldTransform.GetInternalMatrix().Decompose(DirectX::SimpleMath::Vector3(), quat, DirectX::SimpleMath::Vector3());

	Quaternion quat2(quat);

	Rotation = Quaternion::ToEulerAngles(quat2);
	Rotation = Vector3(Mathf::Degrees(Rotation.X()), Mathf::Degrees(Rotation.Y()), Mathf::Degrees(Rotation.Z()));
	InternalRotation = quat2;
	SetDirty(true);
}

void Transform::SetRotation(Vector3 euler)
{
	DirectX::SimpleMath::Quaternion quat2 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(Mathf::Radians(euler.Y()), Mathf::Radians(euler.X()), Mathf::Radians(euler.Z()));
	Quaternion quat(quat2);
	InternalRotation = quat;
	Rotation = euler;
	SetDirty(true);
}

void Transform::SetWorldRotation(Quaternion InRotation)
{
	InternalRotation = Quaternion(ParentTransform->GetWorldRotation().GetInternalVec() * InternalRotation.GetInternalVec());
	Rotation = Quaternion::ToEulerAngles(InternalRotation);
	SetDirty(true);
}

Vector3 Transform::GetRotation()
{
	return Rotation;
}

Quaternion Transform::GetWorldRotation()
{
	DirectX::SimpleMath::Quaternion quat;

	WorldTransform.GetInternalMatrix().Decompose(DirectX::SimpleMath::Vector3(), quat, DirectX::SimpleMath::Vector3());
	Quaternion quat2(quat);

	return quat2;
}

Vector3 Transform::GetWorldRotationEuler()
{
	return Quaternion::ToEulerAngles(GetWorldRotation());
}

//
//void Transform::SetRotation(glm::quat quat)
//{
//	//glm::rotate(Rotation, quat);
//	Rotation = glm::eulerAngles(quat);
//	SetDirty(true);
//}

void Transform::SetParent(Transform& NewParent)
{
	if (ParentTransform)
	{
		ParentTransform->RemoveChild(this);
	}
	ParentTransform = &NewParent;
	ParentTransform->Children.push_back(this);
	SetDirty(true);
}

void Transform::RemoveChild(Transform* TargetTransform)
{
	if (Children.size() > 0)
	{
		Children.erase(std::remove(Children.begin(), Children.end(), TargetTransform), Children.end());
		TargetTransform->ParentTransform = nullptr;
	}
}

Transform* Transform::GetChildByName(const std::string& Name)
{
	for (auto child : Children)
	{
		if (child->Name == Name)
		{
			return child;
		}
	}
	return nullptr;
}

Matrix4 Transform::GetMatrix()
{
	return WorldTransform;
}

void Transform::Serialize(json& outJson)
{
	Component::Serialize(outJson);

	outJson["Position"] = { Position.X(),Position.Y(),Position.Z() };
	outJson["Rotation"] = { Rotation.X(), Rotation.Y(), Rotation.Z() };
	outJson["Scale"] = { Scale.X(), Scale.Y(), Scale.Z() };
}

void Transform::Deserialize(const json& inJson)
{
	SetPosition(Vector3((float)inJson["Position"][0], (float)inJson["Position"][1], (float)inJson["Position"][2]));
	SetRotation(Vector3((float)inJson["Rotation"][0], (float)inJson["Rotation"][1], (float)inJson["Rotation"][2]));
	if (inJson.find("Scale") != inJson.end())
	{
		SetScale(Vector3((float)inJson["Scale"][0], (float)inJson["Scale"][1], (float)inJson["Scale"][2]));
	}

	DirectX::SimpleMath::Matrix id = DirectX::XMMatrixIdentity();
	DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateFromQuaternion(DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(Rotation[1], Rotation[0], Rotation[2]));// , Child->Rotation.Y(), Child->Rotation.Z());
	DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(GetScale().GetInternalVec());
	DirectX::SimpleMath::Matrix pos = XMMatrixTranslationFromVector(GetPosition().GetInternalVec());
	if (ParentTransform)
	{
		SetWorldTransform(Matrix4((scale * rot * pos) * ParentTransform->WorldTransform.GetInternalMatrix()), false);
	}
	else
	{
		SetWorldTransform(Matrix4(scale * rot * pos), false);
	}
}

void Transform::SetName(const std::string& name)
{
	Name = name;
}
