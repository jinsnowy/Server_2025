#pragma once


template<typename T>
static inline std::shared_ptr<T> SharedFrom(T* ptr) {
	return std::static_pointer_cast<T>(ptr->shared_from_this());
}

template<typename T>
static inline std::shared_ptr<T> SharedFrom(T& inst) {
	return std::static_pointer_cast<T>(inst.shared_from_this());
}