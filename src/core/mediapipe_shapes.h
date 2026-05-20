#ifndef MEDIAPIPE_SHAPES_H
#define MEDIAPIPE_SHAPES_H

#include <cstdint>
#include <vector>

#include "tensor_utils.h"

inline const std::vector<int64_t> &mediapipeInputShape()
{
	static const std::vector<int64_t> shape{1, 144, 256, 3};
	return shape;
}

inline const std::vector<int64_t> &mediapipeOutputShape()
{
	static const std::vector<int64_t> shape{1, 144, 256, 2};
	return shape;
}

inline bool mediapipeTensorSizesValid(size_t &inputElements, size_t &outputElements)
{
	return tensorElementCount(mediapipeInputShape(), inputElements) &&
	       tensorElementCount(mediapipeOutputShape(), outputElements);
}

inline void mediapipeNetworkSize(uint32_t &width, uint32_t &height)
{
	const auto &dims = mediapipeInputShape();
	height = static_cast<uint32_t>(dims[1]);
	width = static_cast<uint32_t>(dims[2]);
}

#endif
