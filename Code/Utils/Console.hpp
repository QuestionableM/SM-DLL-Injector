#pragma once

#include "win_include.hpp"
#include "ConColors.hpp"

#include <sstream>
#include <iomanip>
#include <string>
#include <mutex>

#define QE_CREATE_CON_OUTPUT_TYPE(qe_type, qe_arg, qe_func_text) \
template<>                                                       \
struct ConsoleOutputType<qe_type>                                \
{                                                                \
	inline static void Output(qe_arg) qe_func_text               \
}                                                                                                                

#define QE_CREATE_CON_NUMBER_TYPE(qe_type)                    \
template<>                                                    \
struct ConsoleOutputType<qe_type>                             \
{                                                             \
	inline static void Output(const qe_type& number)          \
	{                                                         \
		Console::Output<std::string>(std::to_string(number)); \
	}                                                         \
}

#if defined(QE_CONSOLE_QUOTED_PREFIX)
#define QE_QUOTED_PREFIX "[" QE_CONSOLE_QUOTED_PREFIX "] "
#else
#define QE_QUOTED_PREFIX ""
#endif

namespace Engine
{
	class __ConsoleOutputHandler;

	template<class T>
	struct ConsoleOutputType;

	class Console
	{
		template<class>
		friend struct ConsoleOutputType;

		friend __ConsoleOutputHandler;
		
	public:
		inline static void Endl()
		{
			Console::Output<EngineConColor>(0b1110_fg);
			Console::Output<const char*>("\n");
		}

		static bool CreateEngineConsole(const wchar_t* title);

		static void WaitForAnyKey();

		static bool ReadString(std::wstring& str);
		static bool ReadString(std::string& str);

		static bool AttachToConsole();
		static void DestroyConsole();

		static __ConsoleOutputHandler Out;

	private:
		static HANDLE Handle;

		template<class T>
		inline static void Output(const T arg)
		{
			ConsoleOutputType<T>::Output(arg);
		}

		Console()  = default;
		~Console() = default;
	};

	//---------------CONSOLE TYPE DEFINITIONS--------------

	template<class T>
	struct ConsoleOutputType<T*>
	{
		inline static void Output(T* ptr)
		{
			const std::uintptr_t m_PointerValue = reinterpret_cast<std::uintptr_t>(ptr);

			std::stringstream m_stream;
			m_stream << std::setfill('0') << std::setw(sizeof(std::uintptr_t) * 2) << std::hex << m_PointerValue;

			Console::Output(m_stream.str());
		}
	};

	template<>
	struct ConsoleOutputType<const wchar_t*>
	{
		inline static void Output(const wchar_t* arg)
		{
			WriteConsoleW(Console::Handle, arg, static_cast<DWORD>(wcslen(arg)), NULL, NULL);
		}
	};

	template<>
	struct ConsoleOutputType<const char*>
	{
		inline static void Output(const char* arg)
		{
			WriteConsoleA(Console::Handle, arg, static_cast<DWORD>(strlen(arg)), NULL, NULL);
		}
	};

	template<>
	struct ConsoleOutputType<std::wstring>
	{
		inline static void Output(const std::wstring& msg)
		{
			WriteConsoleW(Console::Handle, msg.data(), static_cast<DWORD>(msg.size()), NULL, NULL);
		}
	};
	
	template<>
	struct ConsoleOutputType<std::string>
	{
		inline static void Output(const std::string& msg)
		{
			WriteConsoleA(Console::Handle, msg.data(), static_cast<DWORD>(msg.size()), NULL, NULL);
		}
	};

	QE_CREATE_CON_NUMBER_TYPE(unsigned long long);
	QE_CREATE_CON_NUMBER_TYPE(long long);
	QE_CREATE_CON_NUMBER_TYPE(unsigned long);
	QE_CREATE_CON_NUMBER_TYPE(long);
	QE_CREATE_CON_NUMBER_TYPE(unsigned int);
	QE_CREATE_CON_NUMBER_TYPE(int);
	QE_CREATE_CON_NUMBER_TYPE(unsigned short);
	QE_CREATE_CON_NUMBER_TYPE(short);
	QE_CREATE_CON_NUMBER_TYPE(unsigned char);
	QE_CREATE_CON_NUMBER_TYPE(char);

	QE_CREATE_CON_NUMBER_TYPE(float);
	QE_CREATE_CON_NUMBER_TYPE(double);

	QE_CREATE_CON_OUTPUT_TYPE(void (*)(), void (*func_ptr)(), { func_ptr(); });
	QE_CREATE_CON_OUTPUT_TYPE(EngineConColor, const EngineConColor& color, { SetConsoleTextAttribute(Console::Handle, static_cast<WORD>(color)); });
	QE_CREATE_CON_OUTPUT_TYPE(bool, const bool& bool_val, { ConsoleOutputType<std::string>::Output(bool_val ? "true" : "false"); });

	//-------------CONSOLE OUTPUT HANDLER------------------

	class __ConsoleOutputHandler
	{
		friend class Console;

	public:
		template<typename ...ArgList>
		inline void operator()(const ArgList& ...arg_list)
		{
			this->variadic_func(arg_list...);
		}

	private:
		template<typename CurArg>
		inline void variadic_func(const CurArg& cur_arg)
		{
			Console::Output(cur_arg);
		}

		template<typename CurArg, typename ...ArgList>
		inline void variadic_func(const CurArg& cur_arg, const ArgList& ...arg_list)
		{
			this->variadic_func(cur_arg);
			this->variadic_func(arg_list...);
		}

		//Remove copy constructors
		__ConsoleOutputHandler(const __ConsoleOutputHandler&) = delete;
		__ConsoleOutputHandler(__ConsoleOutputHandler&&) = delete;

		__ConsoleOutputHandler()  = default;
		~__ConsoleOutputHandler() = default;
	};
}

#define CreateDebugConsole(title) Engine::Console::CreateEngineConsole(title)
#define AttachDebugConsole() Engine::Console::AttachToConsole()

#define ConsoleWait() Engine::Console::WaitForAnyKey()
#define ConsoleRead(str_arg) Engine::Console::ReadString(str_arg)

#define DebugOut(...)  Engine::Console::Out(__VA_ARGS__)
#define DebugOutL(...) Engine::Console::Out(QE_QUOTED_PREFIX, __VA_ARGS__, Engine::Console::Endl)

#define DebugWarningL(...) Engine::Console::Out(0b1101_fg, QE_QUOTED_PREFIX "WARNING: ", __VA_ARGS__, Engine::Console::Endl)
#define DebugErrorL(...)   Engine::Console::Out(0b1001_fg, QE_QUOTED_PREFIX "ERROR: "  , __VA_ARGS__, Engine::Console::Endl)