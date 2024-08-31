#include <iostream>

#include "FileMappingIO.h"

class Locker
{
public:
	Locker(HANDLE mutex);
	~Locker();

private:
	HANDLE mutex_;
};

Locker::Locker(HANDLE mutex) : mutex_(mutex)
{
	if (this->mutex_) WaitForSingleObject(this->mutex_, INFINITE);
}

Locker::~Locker()
{
	if (mutex_) ReleaseMutex(mutex_);
}

class FileMappingIOImpl
{
public:
	size_t size_{};
	HANDLE file_{};
	LPVOID cache_{};
	HANDLE signal_{}, mutex_{};
};

FileMappingIO::FileMappingIO() : impl_(std::make_unique<FileMappingIOImpl>())
{
}

FileMappingIO::~FileMappingIO()
{
	uninit();
}

bool FileMappingIO::init(const std::string& name, size_t size, const std::string& signal, const std::string& mutex, int ioFlags)
{
	return open(name, size, signal, mutex) || ((ioFlags & std::ios::out) && create(name, size, signal, mutex));
}

void FileMappingIO::uninit()
{
	if (impl_->mutex_ != NULL)
	{
		CloseHandle(impl_->mutex_);
		impl_->mutex_ = NULL;
	}

	if (impl_->signal_ != NULL)
	{
		CloseHandle(impl_->signal_);
		impl_->signal_ = NULL;
	}

	if (impl_->file_ != NULL)
	{
		if (impl_->cache_)
		{
			UnmapViewOfFile(impl_->cache_);
			impl_->cache_ = NULL;
		}

		CloseHandle(impl_->file_);
		impl_->file_ = NULL;
	}
}

size_t FileMappingIO::size() const
{
	return impl_->size_;
}

std::shared_ptr<char[]> FileMappingIO::read()
{
	switch (WaitForSingleObject(impl_->signal_, INFINITE))
	{
	case WAIT_OBJECT_0:
	{
		Locker locker{ impl_->mutex_ };
		auto buffer{ std::make_shared<char[]>(impl_->size_) };
		memcpy_s(buffer.get(), impl_->size_, impl_->cache_, impl_->size_);
		return buffer;
	}
	default: break;
	}
	return nullptr;
}

size_t FileMappingIO::write(const char* source, size_t size)
{
	Locker locker{ impl_->mutex_ };
	memset(impl_->cache_, 0, impl_->size_);
	if(!memcpy_s(impl_->cache_, impl_->size_, source, size))
	{
		SetEvent(impl_->signal_);
		return min(size, impl_->size_);
	}
	return 0;
}

bool FileMappingIO::open(const std::string& name, size_t size, const std::string& signal, const std::string& mutex)
{
	if (!(impl_->file_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name.c_str()))
		|| !(impl_->signal_ = OpenEventA(EVENT_ALL_ACCESS, FALSE, signal.c_str()))
		|| !(impl_->mutex_ = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mutex.c_str()))
		|| !(impl_->cache_ = MapViewOfFile(impl_->file_, FILE_MAP_ALL_ACCESS, 0, 0, impl_->size_ = size)))
	{
		uninit();
		return false;
	}
	return true;
}

bool FileMappingIO::create(const std::string& name, size_t size, const std::string& signal, const std::string& mutex)
{
	if (!(impl_->file_ = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name.c_str()))
		|| !(impl_->signal_ = CreateEventA(NULL, FALSE, FALSE, signal.c_str()))
		|| !(impl_->mutex_ = CreateMutexA(NULL, FALSE, mutex.c_str()))
		|| !(impl_->cache_ = MapViewOfFile(impl_->file_, FILE_MAP_ALL_ACCESS, 0, 0, impl_->size_ = size)))
	{
		uninit();
		return false;
	}
	return true;
}
