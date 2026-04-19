#include "Utils/win_include.hpp"
#include "Utils/WinError.hpp"

#include <filesystem>
#include <cstddef>
#include <string>
#include <vector>

#include <Psapi.h>

const char* g_dllFuncNames[] =
{
	"__CxxFrameHandler4",
	"__NLG_Dispatch2",
	"__NLG_Return2"
};

FARPROC g_dllFuncPointers[sizeof(g_dllFuncNames) / sizeof(const char*)];
HMODULE g_dllModule = NULL;

extern "C"
{
	#define GET_FUNC_DEF(func_name, func_id, ...) \
		reinterpret_cast<decltype(&func_name)>(g_dllFuncPointers[func_id])(__VA_ARGS__)

	__declspec(dllexport) __int64 __CxxFrameHandler4(__int64 a1, __int64 a2, int a3, __int64 a4)
	{ return GET_FUNC_DEF(__CxxFrameHandler4, 0, a1, a2, a3, a4); }

	__declspec(dllexport) void __NLG_Dispatch2()
	{ GET_FUNC_DEF(__NLG_Dispatch2, 1); }

	__declspec(dllexport) void __NLG_Return2()
	{ GET_FUNC_DEF(__NLG_Return2, 2); }
}

struct ModuleData
{
	std::string path;
	HMODULE ptr;
};

static std::vector<ModuleData> g_modulesToAttach = {};

static void OutputDetailedErrorMsgBox(
	const char* mainMsg,
	const char* caption)
{
	std::string v_finalMsg(mainMsg);
	v_finalMsg.append("\n\nLast Error: ");
	v_finalMsg.append(WinError::GetLastErrorStringA());

	MessageBoxA(NULL, v_finalMsg.c_str(), caption, MB_ICONERROR);
}

static bool GetApplicationLocation(std::string& outAppPath)
{
	namespace fs = std::filesystem;

	char v_pathBuffer[MAX_PATH];
	const DWORD v_pathBufferSz = GetModuleFileNameA(NULL, v_pathBuffer, sizeof(v_pathBuffer));

	if (v_pathBufferSz == 0)
	{
		OutputDetailedErrorMsgBox("Couldn't locate the application directory", "FATAL ERROR");
		return false;
	}

	fs::path v_path = std::string(v_pathBuffer, static_cast<std::size_t>(v_pathBufferSz));
	if (!v_path.has_parent_path())
	{
		MessageBoxA(NULL, "Couldn't locate the parent path!", "NO PARENT PATH", MB_ICONERROR);
		return false;
	}

	outAppPath = v_path.parent_path().string();
	return true;
}

static bool FindModuleInDirectory()
{
	namespace fs = std::filesystem;

	std::string v_appDirectory;
	if (!GetApplicationLocation(v_appDirectory))
		return false;

	const std::string v_moduleDirectory = v_appDirectory + "/DLLModules";

	std::error_code v_ec;
	fs::directory_iterator v_dirIter(v_moduleDirectory, fs::directory_options::skip_permission_denied, v_ec);

	if (!v_ec)
	{
		for (const auto& v_curDir : v_dirIter)
		{
			if (v_ec || !v_curDir.is_regular_file() || !v_curDir.path().has_extension())
				continue;

			if (v_curDir.path().extension() != ".dll")
				continue;

			g_modulesToAttach.push_back(ModuleData{
				.path = v_curDir.path().string(),
				.ptr = NULL
			});
		}
	}
	else
	{
		MessageBoxA(NULL, "Couldn't find the DLLModules directory", "NO MODULES DIRECTORY", MB_OK);
	}

	return true;
}

static bool IsInjectorDisabled()
{
	return std::string_view(GetCommandLineA()).find("-noinject") != std::string::npos;
}

static bool IsCorrectProcess()
{
	const HANDLE v_hCurProc = GetCurrentProcess();
	if (!v_hCurProc) return false;

	HMODULE v_procModule;
	DWORD v_procNeeded;

	if (!EnumProcessModules(v_hCurProc, &v_procModule, sizeof(v_procModule), &v_procNeeded))
		return false;

	char v_moduleName[MAX_PATH] = "<UNKNOWN>";
	const DWORD v_moduleNameSz = GetModuleBaseNameA(v_hCurProc, v_procModule, v_moduleName, sizeof(v_moduleName));

	if (v_moduleNameSz == 0)
		return false;

	return std::string_view(v_moduleName, v_moduleNameSz) == "ScrapMechanic.exe";
}

static void AttachProcess()
{
	g_dllModule = LoadLibraryA("vcruntime140_1_.dll");
	if (!g_dllModule)
	{
		OutputDetailedErrorMsgBox("vcruntime140_1_.dll is missing!", "MISSING DLL");
		return;
	}

	// Loading the dll procs is a number one priority
	constexpr std::size_t v_functionCount = sizeof(g_dllFuncNames) / sizeof(const char*);
	for (std::size_t a = 0; a < v_functionCount; a++)
	{
		FARPROC v_pProc = GetProcAddress(g_dllModule, g_dllFuncNames[a]);
		if (!v_pProc)
		{
			OutputDetailedErrorMsgBox(g_dllFuncNames[a], "PROC NOT FOUND");
			return;
		}

		g_dllFuncPointers[a] = v_pProc;
	}

	if (IsCorrectProcess())
	{
		if (IsInjectorDisabled())
			return;

		if (!FindModuleInDirectory())
			return;

		for (auto& v_module : g_modulesToAttach)
		{
			v_module.ptr = LoadLibraryA(v_module.path.c_str());
			if (!v_module.ptr)
			{
				OutputDetailedErrorMsgBox(v_module.path.c_str(), "MODULE ERROR");
				return;
			}
		}
	}
}

static void DetachProcess()
{
	for (auto& v_module : g_modulesToAttach)
		if (v_module.ptr) FreeLibrary(v_module.ptr);

	if (g_dllModule)
		FreeLibrary(g_dllModule);
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		AttachProcess();
		break;
	case DLL_PROCESS_DETACH:
		DetachProcess();
		break;
	default:
		break;
	}

	return TRUE;
}