#ifndef TENSOR_UTILS_H
#define TENSOR_UTILS_H

#include <cstddef>
#include <cstdint>
#include <vector>

inline bool tensorElementCount(const std::vector<int64_t> &dims, size_t &outCount)
{
	constexpr size_t kMaxElements = 64 * 1024 * 1024;
	size_t count = 1;

	for (int64_t dim : dims) {
		if (dim <= 0) {
			return false;
		}
		if (static_cast<uint64_t>(dim) > kMaxElements / count) {
			return false;
		}
		count *= static_cast<size_t>(dim);
	}

	outCount = count;
	return true;
}

#endif
