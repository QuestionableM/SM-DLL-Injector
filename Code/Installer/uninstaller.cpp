#include "Common/game_finder.hpp"

#include "Utils/Console.hpp"
#include "Utils/File.hpp"

bool run_main_uninstaller(const std::wstring& game_directory)
{
	const std::wstring v_original_dll = game_directory + L"/vcruntime140_1_.dll";
	const std::wstring v_proxy_dll = game_directory + L"/vcruntime140_1.dll";

	if (File::Exists(v_original_dll))
	{
		if (File::Exists(v_proxy_dll))
		{
			if (!File::Remove(v_proxy_dll))
			{
				DebugErrorL("Couldn't remove the proxy DLL!");
				return false;
			}
		}

		if (!File::Rename(v_original_dll, v_proxy_dll))
		{
			DebugErrorL("Couldn't rename the original DLL!");
			return false;
		}
	}
	else
	{
		DebugOutL("The DLL-Injector has already been uninstalled!");
	}

	return true;
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	CreateDebugConsole(L"DLL-Injector-Uninstaller");
	SetFocus(GetConsoleWindow());

	if (!GameFinder::IsGameRunning())
	{
		std::wstring v_game_path;
		if (GameFinder::FindGame(v_game_path))
		{
			DebugOutL("Found the game path: ", v_game_path);
			if (run_main_uninstaller(v_game_path))
				DebugOutL(0b0101_fg, "Successfully uninstalled the DLL-Injector!");
			else
				DebugOutL(0b1001_fg, "Something went wrong while uninstalling the DLL-Injector!");
		}
	}
	else
	{
		DebugErrorL("Please close the game before running the program!");
	}

	DebugOutL("Press any key to close the uninstaller...");
	ConsoleWait();

	return 0;
}