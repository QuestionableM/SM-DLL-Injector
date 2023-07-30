#include <iostream>
#include <fstream>
#include <vector>
#include <string>

int file_to_cpp(const std::string& source_file, const std::string& output_file, const std::string& variable_name)
{
	std::ifstream v_input_file(source_file, std::ios::binary);
	if (!v_input_file.is_open())
	{
		std::cout << "[FileTpCpp] Couldn't open the specified source file: " << source_file << std::endl;
		return -2;
	}

	v_input_file.seekg(0, std::ios::end);
	std::vector<unsigned char> v_source_bytes(static_cast<std::size_t>(v_input_file.tellg()));
	v_input_file.seekg(0, std::ios::beg);

	v_input_file.read(reinterpret_cast<char*>(v_source_bytes.data()), v_source_bytes.size());
	v_input_file.close();

	std::ofstream v_output_file(output_file);
	if (!v_output_file.is_open())
	{
		std::cout << "[FileToCpp] Couldn't open the output file: " << output_file << std::endl;
		return -3;
	}

	v_output_file << "#pragma once\n\n";
	v_output_file << "const char " << variable_name << "[" << (v_source_bytes.size() + 1) << "] = {\n\t\"";

	std::size_t v_counter = 0;
	for (std::size_t a = 0; a < v_source_bytes.size(); a++)
	{
		if (v_counter >= 50)
		{
			v_counter = 0;
			v_output_file << "\"\n\t\"";
		}

		v_output_file << "\\x" << std::hex << (int)v_source_bytes[a];
		v_counter++;
	}

	v_output_file << "\"\n};";
	v_output_file.close();

	std::cout << "[FileToCpp] Successfully written to: " << output_file << std::endl;

	return 0;
}

int main(int argc, char** argv)
{
	if (argc != 4)
	{
		std::cout << "[FileToCpp] Expected 4 arguments (" << argc << " provided)\n";
		return -1;
	}

	const std::string v_variable_name = argv[1];
	const std::string v_source_file = argv[2];
	const std::string v_destination_file = argv[3];

	return file_to_cpp(v_source_file, v_destination_file, v_variable_name);
}