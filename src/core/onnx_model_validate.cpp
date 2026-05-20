#include "onnx_model_validate.h"

#include <fstream>
#include <filesystem>

bool onnxModelFileLooksValid(const std::string &path)
{
	namespace fs = std::filesystem;

	std::error_code ec;
	if (!fs::exists(path, ec) || !fs::is_regular_file(path, ec)) {
		return false;
	}

	const auto size = fs::file_size(path, ec);
	if (ec || size < 100 * 1024) {
		return false;
	}

	std::ifstream file(path, std::ios::binary);
	if (!file) {
		return false;
	}

	char magic[4] = {};
	file.read(magic, sizeof(magic));
	return file.gcount() == 4 && static_cast<unsigned char>(magic[0]) == 0x08;
}
