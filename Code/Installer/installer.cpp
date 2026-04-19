#include "../Resources/installer_resources.h"
#include "Common/game_finder.hpp"

#include "Utils/Console.hpp"
#include "Utils/File.hpp"

#include <fstream>

static bool WritePackedData(const std::wstring& path, const void* pBinary, const std::size_t binarySz)
{
	std::ofstream v_binOutput(path, std::ios::binary);
	if (!v_binOutput.is_open())
	{
		DebugErrorL("Couldn't write the data to: ", path);
		return false;
	}

	v_binOutput.write(reinterpret_cast<const char*>(pBinary), binarySz);
	return true;
}

static bool RunMainInstaller(const std::wstring& gameDirectory)
{
	const std::wstring v_movedDll = gameDirectory + L"/vcruntime140_1_.dll";
	const std::wstring v_proxyDll = gameDirectory + L"/vcruntime140_1.dll";

	if (!File::Exists(v_movedDll))
	{
		DebugOutL("Moving the dll");
		if (!File::Rename(v_proxyDll, v_movedDll))
		{
			DebugErrorL("Couldn't move the dll!");
			return false;
		}
	}
	else
	{
		DebugOutL("DLL is already moved");
	}

	File::CreateDir(gameDirectory + L"/DLLModules");

	HRSRC v_hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
	if (v_hRes)
	{
		HGLOBAL v_hResData = LoadResource(NULL, v_hRes);
		if (v_hResData)
		{
			const void* v_pResData = LockResource(v_hResData);
			if (v_pResData)
			{
				const DWORD v_resDataSz = SizeofResource(NULL, v_hRes);
				return WritePackedData(v_proxyDll, v_pResData, v_resDataSz);
			}
		}
	}

	return false;
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
		std::wstring v_gamePath;
		if (GameFinder::FindGame(v_gamePath))
		{
			DebugOutL("Found the game path: ", v_gamePath);
			if (RunMainInstaller(v_gamePath))
			{
				DebugOutL(0b0101_fg, "Successfully installed the DLL-Injector!");
			}
			else
			{
				DebugErrorL("Something went wrong while installing the DLL-Injector!");
				DebugErrorL("Make sure you are not running this in the same folder as ScrapMechanic.exe!");

				DWORD v_errorCode = GetLastError();
				if (v_errorCode)
				{
					DebugErrorL("(CODE: ", v_errorCode, ") -> ", std::system_category().message(v_errorCode));
				}
			}
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