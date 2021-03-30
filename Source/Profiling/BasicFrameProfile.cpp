#include "PCH.h"

#include "BasicFrameProfile.h"
#include <imgui.h>
#include <Engine/Engine.h>

FrameProfile::FrameProfile()
{
}

void FrameProfile::Start()
{
}

void FrameProfile::End(float frameDelta)
{
	MainDelta = frameDelta;
}

void FrameProfile::Set(const std::string& name, ProfileCategory category)
{
	Profiles[name].Category = category;
	Profiles[name].Timer.Reset();
}

void FrameProfile::Complete(const std::string& name)
{
	Profiles[name].Timer.Update();
}

void FrameProfile::Render(const Vector2& inPosition, const Vector2& inSize)
{
	const ImU32 flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar /*| ImGuiWindowFlags_NoInputs*/ | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus;
	ImGui::SetNextWindowSize(ImVec2(inSize.x, (float)CurrentProfilerSize));
	ImGui::SetNextWindowPos(ImVec2(inPosition.x, inPosition.y - ((CurrentProfilerSize == kMinProfilerSize) ? 0 : CurrentProfilerSize/2.f)));

	ImGui::PushStyleColor(ImGuiCol_Border, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));


	static int frameCount = 0;
	static float frametime = 0.0f;

	// increase the counter by one
	static int m_fpscount = 0;
	static int fps = 0;
	m_fpscount++;
	++frameCount;

	float totalFrameTime = 0.f;
	for (auto& thing : FrameProfile::GetInstance().ProfileDump)
	{
		totalFrameTime += thing.second.Timer.GetDeltaSeconds();
	}

	totalFrameTime = FrameProfile::GetInstance().MainDelta;

	ImGui::Begin("FrameProfileBar", NULL, flags);
	{
		ImDrawList* draw_list = ImGui::GetForegroundDrawList();

		const ImVec2 p = ImGui::GetCursorScreenPos();
		static ImVec4 col2 = ImVec4(1.0f, .5f, 1.0f, 1.0f);
		static ImVec4 col3 = ImVec4(.5f, 1.f, 1.0f, 1.0f);
		{
			ImU32 col322 = ImColor(col2);
			ImU32 col323 = ImColor(col3);
			float x = p.x, y = p.y;

			float sz = 50.f;
			draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + ImGui::GetWindowSize().x, y + sz), col322);
			float previousX = 0;
			float targetFPS = (1.f / GetEngine().FPS);
			float size = (totalFrameTime / targetFPS);

			if (size >= 100.f)
			{
				size /= 100.f;
			}

			// Full window size
			float windowSizeX = ImGui::GetMainViewport()->Size.x;

			// How big is the whole frame UI wise
			float profileSize = (windowSizeX * size);
			for (auto& thing : FrameProfile::GetInstance().ProfileDump)
			{
				Vector3 profileColor = FrameProfile::GetInstance().GetCategoryColor(thing.second.Category);
				ImVec4 col = ImVec4(profileColor.x, profileColor.y, profileColor.z, 1.0f);
				ImU32 col32 = ImColor(col);
				float maxSize = (profileSize <= windowSizeX) ? profileSize : windowSizeX;
				float profileSizeX = (thing.second.Timer.GetDeltaSeconds() / totalFrameTime) * maxSize;
				draw_list->AddRectFilled(ImVec2(previousX + x, y), ImVec2(previousX + x + profileSizeX, y + CurrentProfilerSize), col32);
				ImGui::SameLine();
				previousX = previousX + profileSizeX;
			}
			if (profileSize > windowSizeX)
			{
				ImVec4 col = ImVec4(1.f, 0.f, 0.f, 1.f);
				ImU32 col32 = ImColor(col);

				float profileSizeX = profileSize / windowSizeX;
				float xxxx = windowSizeX * profileSizeX;
				draw_list->AddRectFilled(ImVec2(x + windowSizeX - (profileSize - windowSizeX), y), ImVec2((windowSizeX + xxxx), y + 10.f), col32);
				ImGui::SameLine();
				previousX = previousX + profileSizeX;
			}
			else
			{
				ImVec4 col = ImVec4(0.f, 1.f, 0.f, 1.f);
				ImU32 col32 = ImColor(col);
				//float xxxx = windowSizeX * profileSize;
				draw_list->AddRectFilled(ImVec2(previousX + x, y), ImVec2(windowSizeX + x, y + CurrentProfilerSize), col32);
			}

			ImGui::PopStyleVar(3);
			ImGui::PopStyleColor(1);

			if (ImGui::IsWindowHovered())
			{
				CurrentProfilerSize = kMaxProfilerSize;
				ImGui::SetNextWindowPos(ImVec2(ImGui::GetMousePos().x, (ImGui::GetMousePos().y - PreviousTooltipHeight) - CurrentProfilerSize));
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);

				for (auto& thing : FrameProfile::GetInstance().ProfileDump)
				{
					ImGui::TextUnformatted(thing.first.c_str());
					ImGui::SameLine();
					ImGui::TextUnformatted(std::to_string(thing.second.Timer.GetDeltaSeconds() * 1000.f).c_str());
					ImGui::SameLine();
					ImGui::TextUnformatted(" ms");
				}
				ImGui::PopTextWrapPos();

				PreviousTooltipHeight = ImGui::GetWindowHeight();
				ImGui::EndTooltip();
			}
			else
			{
				CurrentProfilerSize = kMinProfilerSize;
			}
		}
	}
	ImGui::End();
}

void FrameProfile::Dump()
{
	ProfileDump = Profiles;
	//std::sort(ProfileDump.begin(), ProfileDump.end(), [](const Clock& First, const Clock& Second) {
	//	return First.GetPreviousTime() < Second.GetPreviousTime();
	//});
}

Vector3 FrameProfile::GetCategoryColor(ProfileCategory category)
{
	switch (category)
	{
	case ProfileCategory::Game:
		return Vector3(0, 175, 84) / 255.f;
		break;
	case ProfileCategory::UI:
		return Vector3(153, 57, 85) / 255.f;
		break;
	case ProfileCategory::Rendering:
		return Vector3(25, 25, 35) / 255.f;
		break;
	default:
		break;
	}
	return Vector3(.1f, .1f, 1.f);
}

