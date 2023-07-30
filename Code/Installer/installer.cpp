#include "PackedData/pd_dll_proxy.hpp"
#include "Common/game_finder.hpp"

#include "Utils/Console.hpp"
#include "Utils/File.hpp"

#include <fstream>

bool write_packed_data(const std::wstring& path, const char* binary_data, std::size_t binary_size)
{
	std::ofstream v_binary_output(path, std::ios::binary);
	if (!v_binary_output.is_open())
	{
		DebugErrorL("Couldn't write the data to: ", path);
		return false;
	}

	v_binary_output.write(binary_data, binary_size);
	v_binary_output.close();

	return true;
}

bool run_main_installer(const std::wstring& game_directory)
{
	const std::wstring v_moved_dll = game_directory + L"/vcruntime140_1_.dll";
	const std::wstring v_proxy_dll = game_directory + L"/vcruntime140_1.dll";

	if (!File::Exists(v_moved_dll))
	{
		DebugOutL("Moving the dll");
		if (!File::Rename(v_proxy_dll, v_moved_dll))
		{
			DebugErrorL("Couldn't move the dll!");
			return false;
		}
	}
	else
	{
		DebugOutL("DLL is already moved");
	}

	File::CreateDir(game_directory + L"/DLLModules");

	//Subtract the null character from the binary
	return write_packed_data(v_proxy_dll, g_proxyDllData, sizeof(g_proxyDllData) - 1);
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	CreateDebugConsole(L"DLL-Injector-Installer");
	SetFocus(GetConsoleWindow());

	if (!GameFinder::IsGameRunning())
	{
		std::wstring v_game_path;
		if (GameFinder::FindGame(v_game_path))
		{
			DebugOutL("Found the game path: ", v_game_path);
			if (run_main_installer(v_game_path))
				DebugOutL(0b0101_fg, "Successfully installed the DLL-Injector!");
			else
				DebugOutL(0b1001_fg, "Something went wrong while installing the DLL-Injector!");
		}
	}
	else
	{
		DebugErrorL("Please close the game before running the program!");
	}

	DebugOutL("Press any key to close the installer...");
	ConsoleWait();

	return 0;
}