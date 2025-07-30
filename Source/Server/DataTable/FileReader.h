#pragma once

namespace Server::DataTable
{
	class FileReader {
	public:
		FileReader(const std::string& file_path)
			: file_path_(file_path) {
			file_stream_.open(file_path_);
			if (!file_stream_.is_open()) {
				throw std::runtime_error("Failed to open file: " + file_path_);
			}
		}
		~FileReader() {
			if (file_stream_.is_open()) {
				file_stream_.close();
			}
		}

		std::ifstream& stream() {
			return file_stream_;
		}

	private:
		std::ifstream file_stream_;
		std::string file_content_;
		std::string file_path_;
	};
}


