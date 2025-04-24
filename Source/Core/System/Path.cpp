#include "stdafx.h"
#include "Path.h"

namespace System {
	namespace fs = std::filesystem;

	std::string Path::GetDirectoryName(const std::string& path) {
		return fs::path(path).parent_path().string();
	}

	std::string Path::GetFileName(const std::string& path) {
		return fs::path(path).filename().string();
	}

	std::string Path::GetExtension(const std::string& path) {
		return fs::path(path).extension().string();
	}

	std::string Path::GetFileNameWithoutExtension(const std::string& path) {
		return fs::path(path).replace_extension("").string();
	}

	std::string Path::GetRootPath(const std::string& path) {
		return fs::path(path).root_path().string();
	}

	std::string Path::GetFullPath(const std::string& path) {
		return fs::canonical(path).string();
	}

	std::string Path::Join(const std::string& path1, const std::string& path2) {
		return fs::path(path1).append(path2).string();
	}

	void Path::CreateDirectory(const std::string& path) {
		fs::create_directories(path);
	}

	std::string Path::GetWokringDirectory() {
		return GetDirectoryName(GetExecutablePath());
	}

	std::string Path::GetExecutablePath() {
#ifdef _WIN32
		char buffer[MAX_PATH] = {};
		::GetModuleFileNameA(NULL, buffer, MAX_PATH);
		return buffer;
#else
		return "";
#endif
	}
}