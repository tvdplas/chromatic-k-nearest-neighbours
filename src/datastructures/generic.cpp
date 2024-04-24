#pragma once

namespace DS {
	template<size_t SIZE, class T> inline int array_size(T(&arr)[SIZE]) {
		return (int)SIZE;
	}
}