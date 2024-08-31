#pragma once

#include <Windows.h>

#include <string>
#include <memory>

class FileMappingIOImpl;
class FileMappingIO
{
public:
	FileMappingIO();
	~FileMappingIO();

	bool init(const std::string& name, size_t size, const std::string& signal, const std::string& mutex, int ioFlags);

	void uninit();

	size_t size() const;

	std::shared_ptr<char[]> read();

	size_t write(const char* source, size_t size);

private:
	bool open(const std::string& name, size_t size, const std::string& signal, const std::string& mutex);

	bool create(const std::string& name, size_t size, const std::string& signal, const std::string& mutex);

private:
	std::unique_ptr<FileMappingIOImpl> impl_{};
};

