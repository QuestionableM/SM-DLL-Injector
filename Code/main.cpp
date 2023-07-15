#include <Windows.h>

#include <filesystem>
#include <cstddef>
#include <string>
#include <vector>

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

bool get_application_location(std::string& app_path)
{
	namespace fs = std::filesystem;

	char v_path_buffer[MAX_PATH];
	const DWORD v_buffer_size = GetModuleFileNameA(NULL, v_path_buffer, sizeof(v_path_buffer));
	if (v_buffer_size == 0)
	{
		MessageBoxA(NULL, "Couldn't locate the application directory", "FATAL ERROR", MB_ICONERROR);
		return false;
	}

	fs::path v_path = std::string(v_path_buffer, static_cast<std::size_t>(v_buffer_size));
	if (!v_path.has_parent_path())
	{
		MessageBoxA(NULL, "Couldn't locate the parent path!", "NO PARENT PATH", MB_ICONERROR);
		return false;
	}

	app_path = v_path.parent_path().string();
	return true;
}

bool find_modules_in_directory()
{
	std::string v_app_directory;
	if (!get_application_location(v_app_directory))
		return false;

	const std::string v_module_directory = v_app_directory + "/DLLModules";

	namespace fs = std::filesystem;
	std::error_code v_ec;
	fs::directory_iterator v_dir_iter(v_module_directory, fs::directory_options::skip_permission_denied, v_ec);

	if (v_ec) return false;

	for (const auto& v_cur_dir : v_dir_iter)
	{
		if (v_ec || !v_cur_dir.is_regular_file() || !v_cur_dir.path().has_extension())
			continue;

		g_modulesToAttach.push_back(ModuleData{
			.path = v_cur_dir.path().string(),
			.ptr = NULL
		});
	}

	return true;
}

void attach_process()
{
	g_dllModule = LoadLibraryA("vcruntime140_1_.dll");
	if (!g_dllModule)
	{
		MessageBoxA(NULL, "vcruntime140_1_.dll is missing!", "MISSING DLL", MB_ICONERROR);
		return;
	}

	if (!find_modules_in_directory())
		return;

	for (auto& v_module : g_modulesToAttach)
	{
		v_module.ptr = LoadLibraryA(v_module.path.c_str());
		if (!v_module.ptr)
		{
			MessageBoxA(NULL, v_module.path.c_str(), "MODULE ERROR", MB_ICONERROR);
			return;
		}
	}

	constexpr std::size_t v_function_count = sizeof(g_dllFuncNames) / sizeof(const char*);
	for (std::size_t a = 0; a < v_function_count; a++)
	{
		FARPROC v_func_ptr = GetProcAddress(g_dllModule, g_dllFuncNames[a]);
		if (!v_func_ptr)
		{
			MessageBoxA(NULL, g_dllFuncNames[a], "PROC NOT FOUND", MB_ICONERROR);
			return;
		}

		g_dllFuncPointers[a] = v_func_ptr;
	}
}

void detach_process()
{
	for (auto& v_module : g_modulesToAttach)
		if (v_module.ptr) FreeLibrary(v_module.ptr);

	if (g_dllModule)
		FreeLibrary(g_dllModule);
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		attach_process();
		break;
	case DLL_PROCESS_DETACH:
		detach_process();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}