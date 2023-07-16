#include "Console.hpp"

namespace Engine
{
	HANDLE Console::Handle = NULL;
	__ConsoleOutputHandler Console::Out = {};

	bool Console::CreateEngineConsole(const wchar_t* title)
	{
		if (Console::Handle == NULL)
		{
			if (AllocConsole())
			{
				SetConsoleOutputCP(CP_UTF8);
				SetConsoleTitleW(title);

				Console::Handle = GetStdHandle(STD_OUTPUT_HANDLE);

				return true;
			}
		}

		return false;
	}

	void Console::WaitForAnyKey()
	{
		HANDLE v_con_input = GetStdHandle(STD_INPUT_HANDLE);
		DWORD v_con_mode;
		DWORD v_buff_written;
		char v_buffer;

		GetConsoleMode(v_con_input, &v_con_mode);
		SetConsoleMode(v_con_input, 0);

		WaitForSingleObject(v_con_input, INFINITE);
		ReadConsoleA(v_con_input, &v_buffer, 1, &v_buff_written, NULL);

		SetConsoleMode(v_con_input, v_con_mode);
	}

	bool Console::ReadString(std::wstring& str)
	{
		wchar_t v_input_buffer[MAX_PATH];
		DWORD v_fill_size;

		BOOL v_read_result = ReadConsoleW(GetStdHandle(STD_INPUT_HANDLE), (LPVOID)v_input_buffer, sizeof(v_input_buffer) / sizeof(wchar_t), &v_fill_size, NULL);
		if (!v_read_result) return false;

		str = std::wstring(v_input_buffer, static_cast<std::size_t>(v_fill_size));
		return true;
	}

	bool Console::ReadString(std::string& str)
	{
		char v_input_buffer[MAX_PATH];
		DWORD v_fill_size;

		BOOL v_read_result = ReadConsoleA(GetStdHandle(STD_INPUT_HANDLE), (LPVOID)v_input_buffer, sizeof(v_input_buffer) / sizeof(char), &v_fill_size, NULL);
		if (!v_read_result) return false;

		str = std::string(v_input_buffer, static_cast<std::size_t>(v_fill_size));
		return true;
	}

	bool Console::AttachToConsole()
	{
		if (Console::Handle == NULL)
		{
			Console::Handle = GetStdHandle(STD_OUTPUT_HANDLE);
			return (Console::Handle != NULL);
		}

		return false;
	}

	void Console::DestroyConsole()
	{
		if (Console::Handle == NULL) return;

		FreeConsole();
		Console::Handle = NULL;
	}
}