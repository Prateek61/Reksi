#pragma once

#include <iostream>
#include <memory>
#include <filesystem>
#include <fstream>

inline std::shared_ptr<std::string> FileStringLoader(const std::filesystem::path& path)
{
	// Check if file exists
	if (!std::filesystem::exists(path))
	{
		throw std::runtime_error("File does not exist");
	}

	// Open file
	std::ifstream stream(path, std::ios::in | std::ios::binary);
	if (!stream)
	{
		throw std::runtime_error("Failed to open file");
	}

	stream.seekg(0, std::ios::end);
	const int64_t size = stream.tellg();
	stream.seekg(0, std::ios::beg);

	auto ptr = std::make_shared<std::string>(size, '\0');
	stream.read(ptr->data(), size);
	return ptr;
}