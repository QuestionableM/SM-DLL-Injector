#pragma once

#include "win_include.hpp"

#include <string>

namespace WinError
{
	std::string GetLastErrorStringA()
	{
		LPSTR v_msgBuffer;
		const std::size_t v_msgSize = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
			reinterpret_cast<LPSTR>(&v_msgBuffer),
			0,
			NULL
		);

		std::string v_outputMsg;
		if (v_msgBuffer == 0)
			v_outputMsg.assign("UNKNOWN_ERROR");
		else
			v_outputMsg.assign(v_msgBuffer, v_msgSize);

		LocalFree(v_msgBuffer);
		return v_outputMsg;
	}
}