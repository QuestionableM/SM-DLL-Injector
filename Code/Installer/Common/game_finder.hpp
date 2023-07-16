#pragma once

#include <string>

namespace GameFinder
{
	bool IsGameRunning();

	bool FindSteamPath(std::wstring& steam_path);
	bool FindGamePath(std::wstring& game_path);
	bool CheckPathValid(std::wstring& game_path);
	bool FindGame(std::wstring& game_path);
}