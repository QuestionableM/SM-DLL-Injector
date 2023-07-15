#include <iostream>

#include "PackedData/pd_dll_proxy.hpp"

#include <fstream>

void write_packed_data(const std::string& path, const char* binary_data, std::size_t binary_size)
{
	std::ofstream v_binary_output(path, std::ios::binary);
	if (!v_binary_output.is_open())
		std::cout << "Couldn't write the data to: " << path << std::endl;

	v_binary_output.write(binary_data, binary_size);
	v_binary_output.close();
}

int main()
{
	write_packed_data("./binary_output.dll", g_proxyDllData, sizeof(g_proxyDllData));
	return 0;
}