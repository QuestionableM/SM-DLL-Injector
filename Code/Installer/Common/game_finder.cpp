#include "game_finder.hpp"

#include "Utils/Console.hpp"
#include "Utils/String.hpp"
#include "Utils/File.hpp"

#include <valve_vdf\vdf_parser.hpp>

#include <Psapi.h>

bool GameFinder::IsGameRunning()
{
	DWORD process_list[1024], cbNeeded;
	if (!EnumProcesses(process_list, sizeof(process_list), &cbNeeded))
		return false;

	const DWORD self_proc_id = GetCurrentProcessId();
	for (unsigned int i = 0; i < (cbNeeded / sizeof(DWORD)); i++)
	{
		if (process_list[i] == 0 || process_list[i] == self_proc_id) continue;

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_list[i]);
		if (hProcess == nullptr)
			continue;

		TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
		HMODULE hMod;
		DWORD cbNeeded;

		if (!EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
		{
			CloseHandle(hProcess);
			continue;
		}

		GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
		const std::string v_proc_str = String::ToUtf8(szProcessName);

		CloseHandle(hProcess);

		if (v_proc_str == "ScrapMechanic.exe")
			return true;
	}

	return false;
}

bool GameFinder::FindSteamPath(std::wstring& steam_path)
{
	if (String::ReadRegistryKeyW(L"SOFTWARE\\Valve\\Steam", L"SteamPath", steam_path))
		return true;

	if (String::ReadRegistryKeyW(L"SOFTWARE\\WOW6432Node\\Valve\\Steam", L"SteamPath", steam_path))
		return true;

	return false;
}

bool GameFinder::FindGamePath(std::wstring& game_path)
{
	std::wstring v_steam_path;
	if (!GameFinder::FindSteamPath(v_steam_path))
	{
		DebugErrorL("Couldn't find the steam path!");
		return false;
	}

	DebugOutL("Found a steam path: ", v_steam_path);

	const std::wstring v_vdf_path = v_steam_path + L"\\steamapps\\libraryfolders.vdf";
	if (!File::Exists(v_vdf_path))
	{
		DebugErrorL("Couldn't locate the steam libraryfolders file!");
		return false;
	}

	std::ifstream v_vdf_file_reader(v_vdf_path);
	if (!v_vdf_file_reader.is_open())
	{
		DebugErrorL("Couldn't open the steam libraryfolders file!");
		return false;
	}

	tyti::vdf::basic_object<char> v_vdf_root = tyti::vdf::read(v_vdf_file_reader);
	v_vdf_file_reader.close();

	if (v_vdf_root.name != "libraryfolders")
	{
		DebugErrorL("Invalid vdf root!");
		return false;
	}

	for (const auto& v_lib_folder : v_vdf_root.childs)
	{
		const auto v_attrib_iter = v_lib_folder.second->attribs.find("path");
		if (v_attrib_iter == v_lib_folder.second->attribs.end())
			continue;

		const auto v_childs_iter = v_lib_folder.second->childs.find("apps");
		if (v_childs_iter == v_lib_folder.second->childs.end())
			continue;

		const auto v_app_attrib_iter = v_childs_iter->second->attribs.find("387990");
		if (v_app_attrib_iter == v_childs_iter->second->attribs.end())
			continue;

		const std::wstring v_library_path_wstr = String::ToWide(v_attrib_iter->second);
		const std::wstring v_library_sm_path = v_library_path_wstr + L"\\steamapps\\common\\Scrap Mechanic\\Release";

		if (File::Exists(v_library_sm_path))
		{
			game_path = v_library_sm_path;
			return true;
		}
	}

	return false;
}

void remove_crlf_end(std::wstring& v_string)
{
	while (!v_string.empty() && (v_string[v_string.size() - 1] == '\n' || v_string[v_string.size() - 1] == '\r'))
		v_string.pop_back();
}

void remove_quotes(std::wstring& v_string)
{
	if (v_string.size() < 2)
		return;

	if (v_string[0] == '"' && v_string[v_string.size() - 1] == '"')
	{
		v_string.erase(v_string.begin());
		v_string.pop_back();
	}
}

bool GameFinder::CheckPathValid(std::wstring& game_path)
{
	if (!File::Exists(game_path))
	{
		DebugErrorL("The specified path doesn't exist!");
		return false;
	}

	if (File::Exists(game_path + L"\\ScrapMechanic.exe"))
		return true;

	if (File::Exists(game_path + L"\\Release\\ScrapMechanic.exe"))
	{
		game_path.append(L"\\Release");
		return true;
	}

	DebugErrorL("The specified path does not contain game content!");
	return false;
}

bool GameFinder::FindGame(std::wstring& game_path)
{
	if (GameFinder::FindGamePath(game_path))
		return true;

	DebugOutL("Couldn't find Scrap Mechanic on your machine. Please specify the path manually");

	while (true)
	{
		DebugOut("> ");

		if (!ConsoleRead(game_path))
		{
			DebugErrorL("Failed to read console input!");
			return false;
		}

		remove_crlf_end(game_path);
		remove_quotes(game_path);

		if (GameFinder::CheckPathValid(game_path))
			return true;
	}

	return false;
}