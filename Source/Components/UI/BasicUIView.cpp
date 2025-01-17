#include "PCH.h"

#include "BasicUIView.h"
#include "imgui.h"
#include "UI/JSHelpers.h"
#include "Ultralight/View.h"
#include "Events/AudioEvents.h"
#include <Ultralight/String.h>
#include "Events/HavanaEvents.h"

BasicUIView::BasicUIView()
	: Component("BasicUIView")
{

}

BasicUIView::BasicUIView(const char* Name)
	: Component(Name)
{

}

void BasicUIView::Init()
{
	SourceFile = File(FilePath);
}

void BasicUIView::OnSerialize(json& outJson)
{
	outJson["FilePath"] = FilePath.LocalPath;
}

void BasicUIView::OnDeserialize(const json& inJson)
{
	if (inJson.contains("FilePath"))
	{
		FilePath = Path(inJson["FilePath"]);
	}
}

void BasicUIView::OnUpdateHistory(ultralight::View* caller)
{
}

void BasicUIView::OnDOMReady(ultralight::View* caller,
	uint64_t frame_id,
	bool is_main_frame,
	const ultralight::String& url)
{
	ultralight::Ref<ultralight::JSContext> context = caller->LockJSContext();
	ultralight::SetJSContext(context.get());
	ultralight::JSObject global = ultralight::JSGlobalObject();

	global["PlaySound"] = BindJSCallback(&BasicUIView::PlaySound);

	OnUILoad(global, caller);
}

void BasicUIView::OnUILoad(ultralight::JSObject& GlobalWindow, ultralight::View* Caller)
{
}

void BasicUIView::ExecuteScript(const std::string& Script)
{
	ViewRef->EvaluateScript(Script.c_str());
}

void BasicUIView::PlaySound(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
{
	if (args.empty() || !args[0].IsString())
	{
		BRUH("PlaySound(string) argument mismatch.");
		return;
	}
	ultralight::String str = args[0].ToString();
	std::string str2(str.utf8().data());
	PlayAudioEvent evt(str2);
	evt.Fire();
}

#if ME_EDITOR

void BasicUIView::OnEditorInspect()
{
	ImVec2 selectorSize(-1.f, 19.f);
	HavanaUtils::Label("Source");
	if (ImGui::Button(((!FilePath.LocalPath.empty()) ? FilePath.LocalPath.c_str() : "Select Asset"), selectorSize))
	{
		RequestAssetSelectionEvent evt([this](Path selectedAsset) {
			FilePath = selectedAsset;
			SourceFile = File(FilePath);
			IsInitialized = false;
			}, AssetType::UI);
		evt.Fire();
	}
}

#endif
