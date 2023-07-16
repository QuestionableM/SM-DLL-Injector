#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>

namespace String
{
	inline std::string ToUtf8(const std::wstring& wstr)
	{
		const int v_count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);

		std::string v_str(v_count, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &v_str[0], v_count, NULL, NULL);

		return v_str;
	}

	inline std::wstring ToWide(const std::string_view& v_str)
	{
		const int v_str_sz = static_cast<int>(v_str.size());
		const int v_count = MultiByteToWideChar(CP_UTF8, 0, v_str.data(), v_str_sz, NULL, 0);

		std::wstring v_wstr(v_count, 0);
		MultiByteToWideChar(CP_UTF8, 0, v_str.data(), v_str_sz, &v_wstr[0], v_count);

		return v_wstr;
	}

	inline std::wstring ToWide(const char* str)
	{
		const int v_str_sz = static_cast<int>(strlen(str));
		const int v_count = MultiByteToWideChar(CP_UTF8, 0, str, v_str_sz, NULL, 0);

		std::wstring v_wstr(v_count, 0);
		MultiByteToWideChar(CP_UTF8, 0, str, v_str_sz, &v_wstr[0], v_count);

		return v_wstr;
	}

	inline std::wstring ToWide(const std::string& str)
	{
		const int v_str_sz = static_cast<int>(str.size());
		const int v_count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), v_str_sz, NULL, 0);

		std::wstring v_wstr(v_count, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), v_str_sz, &v_wstr[0], v_count);

		return v_wstr;
	}

	bool ReadRegistryKeyW(const std::wstring& main_key, const std::wstring& sub_key, std::wstring& r_output)
	{
		wchar_t v_data_buf[MAX_PATH] = {};
		DWORD v_buf_sz = sizeof(v_data_buf);

		const LSTATUS v_status = RegGetValueW(
			HKEY_CURRENT_USER,
			main_key.c_str(),
			sub_key.c_str(),
			RRF_RT_REG_SZ,
			NULL,
			reinterpret_cast<PVOID>(v_data_buf),
			&v_buf_sz
		);

		if (v_status == ERROR_SUCCESS)
		{
			r_output = std::wstring(v_data_buf);
			return true;
		}

		return false;
	}
}