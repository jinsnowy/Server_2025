#pragma once


namespace System {
	class Path {
	public:
		static std::string GetDirectoryName(const std::string& path);
		static std::string GetFileName(const std::string& path);
		static std::string GetExtension(const std::string& path);
		static std::string GetFileNameWithoutExtension(const std::string& path);
		static std::string GetRootPath(const std::string& path);
		static std::string GetFullPath(const std::string& path);
		static std::string Join(const std::string& path1, const std::string& path2);
		static void CreateDirectory(const std::string& path);

		static std::string GetWokringDirectory();
		static std::string GetExecutablePath();
	};
}