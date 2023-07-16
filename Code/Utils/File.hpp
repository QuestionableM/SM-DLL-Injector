#pragma once

#include <filesystem>

namespace File
{
	inline bool Exists(const std::wstring& path)
	{
		namespace fs = std::filesystem;

		std::error_code v_ec;
		bool v_exists = fs::exists(path, v_ec);

		return !v_ec && v_exists;
	}

	inline bool Rename(const std::wstring& old_path, const std::wstring& new_path)
	{
		namespace fs = std::filesystem;

		std::error_code v_ec;
		fs::rename(old_path, new_path, v_ec);

		return !v_ec;
	}

	inline bool CreateDir(const std::wstring& directory)
	{
		namespace fs = std::filesystem;

		std::error_code v_ec;
		const bool v_created = fs::create_directory(directory, v_ec);

		return !v_ec && v_created;
	}

	inline bool Remove(const std::wstring& path)
	{
		namespace fs = std::filesystem;

		std::error_code v_ec;
		const bool v_removed = fs::remove(path, v_ec);

		return !v_ec && v_removed;
	}
}