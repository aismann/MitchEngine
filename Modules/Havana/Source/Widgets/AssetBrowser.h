#pragma once
#include <chrono>
#include <string>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <thread>
#include <stack>
#include <map>
#include "Path.h"
#include "Pointers.h"
#include "Graphics/Texture.h"
#include "File.h"
#include "imgui.h"
#include <Types/AssetType.h>
#include <Events/EventReceiver.h>

class Transform;

#if ME_EDITOR
enum MyItemColumnID
{
	MyItemColumnID_ID,
	MyItemColumnID_Name,
	MyItemColumnID_Action,
	MyItemColumnID_Quantity,
	MyItemColumnID_Description
};

enum class FileStatus : unsigned int
{
	Created = 0,
	Modified,
	Deleted
};

class AssetBrowser
	: public EventReceiver
{
public:

	struct AssetDescriptor
	{
		int         ID;
		int         Quantity;
		std::string Name;
		File MetaFile;
		Path FullPath;
		AssetType Type;
		// We have a problem which is affecting _only this demo_ and should not affect your code:
		// As we don't rely on std:: or other third-party library to compile dear imgui, we only have reliable access to qsort(),
		// however qsort doesn't allow passing user data to comparing function.
		// As a workaround, we are storing the sort specs in a static/global for the comparing function to access.
		// In your own use case you would probably pass the sort specs to your sorting/comparing functions directly and not use a global.
		// We could technically call ImGui::TableGetSortSpecs() in CompareWithSortSpecs(), but considering that this function is called
		// very often by the sorting algorithm it would be a little wasteful.
		static const ImGuiTableSortSpecs* s_current_sort_specs;

		// Compare function to be used by qsort()
		static int CompareWithSortSpecs(const void* lhs, const void* rhs)
		{
			const AssetDescriptor* a = (const AssetDescriptor*)lhs;
			const AssetDescriptor* b = (const AssetDescriptor*)rhs;
			for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
			{
				// Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
				// We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
				const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
				int delta = 0;
				switch (sort_spec->ColumnUserID)
				{
				case MyItemColumnID_ID:             delta = ((int)a->Type - (int)b->Type);                break;
				case MyItemColumnID_Name:           delta = (strcmp(a->Name.c_str(), b->Name.c_str()));     break;
				case MyItemColumnID_Quantity:       delta = (a->Quantity - b->Quantity);    break;
				case MyItemColumnID_Description:    delta = (strcmp(a->Name.c_str(), b->Name.c_str()));     break;
				default: IM_ASSERT(0); break;
				}
				if (delta > 0)
					return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
				if (delta < 0)
					return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
			}

			// qsort() is instable so always return a way to differenciate items.
			// Your own compare function may want to avoid fallback on implicit sort specs e.g. a Name compare if it wasn't already part of the sort specs.
			return ((int)a->Type - (int)b->Type);
		}
	};
	struct Directory
	{
		std::map<std::string, Directory> Directories;
		std::vector<AssetBrowser::AssetDescriptor> Files;
		Path FullPath;
	};
	AssetBrowser() = delete;
	AssetBrowser(const std::string& pathToWatch, std::chrono::duration<int, std::milli> delay);

	void ReloadDirectories();

	~AssetBrowser();
	void Start(const std::function<void(std::string, FileStatus)>& action);
	std::chrono::duration<int, std::milli> Delay;
	std::string PathToWatch;

	void ThreadStart(const std::function<void(std::string, FileStatus)>& action);

	void Draw();

	void DrawAssetTable();

	void RequestOverlay(const std::function<void(Path)> cb = nullptr, AssetType forcedType = AssetType::Unknown);
	bool OnEvent(const BaseEvent& evt);
	void Recursive(Directory& dir);
	void ProccessDirectory(const std::filesystem::directory_entry& file, Directory& dirRef);
	void BuildAssets();
	void ClearAssets();
private:
	bool ProccessDirectoryRecursive(std::string& dir, Directory& dirRef, const std::filesystem::directory_entry& file);
	void BuildAssetsRecursive(Directory& dir);

	std::vector<SharedPtr<Resource>> m_compiledAssets;
	void SavePrefab(json& d, Transform* CurrentTransform, bool IsRoot);
	std::unordered_map<std::string, std::filesystem::file_time_type> Paths;
	bool IsRunning = true;
	bool Contains(const std::string& key);
	std::thread fileBrowser;
	Directory AssetDirectory;
	Directory EngineAssetDirectory;
	std::unordered_map<std::string, SharedPtr<Moonlight::Texture>> Icons;
	AssetDescriptor* SelectedAsset = nullptr;

	bool IsBrowserOpen = false;
	std::vector<AssetBrowser::AssetDescriptor> MasterAssetsList;
	std::function<void(Path)> AssetSelectedCallback;
	std::vector<AssetDescriptor*> FilteredAssetList;
	AssetType ForcedAssetFilter = AssetType::Unknown;
	bool items_need_filtered = true;
};

#endif