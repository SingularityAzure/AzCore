/*
	File: BucketArray.hpp
	Author: Philip Haynes
*/

#ifndef AZCORE_BUCKETARRAY_HPP
#define AZCORE_BUCKETARRAY_HPP

#include "../basictypes.hpp"
#include "Array.hpp"
#include <stdexcept> // std::out_of_range
#include <initializer_list>
#include <type_traits> // std::is_trivially_copyable
#include <cstring>     // memcpy

namespace AzCore {

// NOTE: This is so BigInt can fit in a single cache line.
#pragma pack(1)

/*  struct: BucketArray
	Author: Philip Haynes
	An array with a fixed-sized memory pool that behaves similarly to Array.    */
template <typename T, i32 count>
struct BucketArray {
	T data[count];
	i32 size;

	BucketArray() : size(0) {}
	BucketArray(i32 newSize) : size(newSize) {
		AzAssert(size <= count, "BucketArray initialized with a size bigger than count");
	}
	BucketArray(i32 newSize, const T &value) : size(newSize) {
		AzAssert(size <= count, "BucketArray initialized with a size bigger than count");
		for (i32 i = 0; i < size; i++) {
			data[i] = value;
		}
	}
	BucketArray(u32 newSize) : BucketArray((i32)newSize) {}
	BucketArray(u32 newSize, const T &value) : BucketArray((i32)newSize, value) {}
	BucketArray(const std::initializer_list<T> &init) : size(init.size()) {
		AzAssert(size <= count, "BucketArray initialized with a size bigger than count");
		i32 i = 0;
		for (const T &val : init) {
			data[i++] = val;
		}
	}
	BucketArray(const BucketArray<T, count> &other) : size(other.size) {
		if constexpr (std::is_trivially_copyable<T>::value) {
			memcpy((void *)data, (void *)other.data, sizeof(T) * size);
		} else {
			for (i32 i = 0; i < size; i++) {
				data[i] = other.data[i];
			}
		}
	}
	BucketArray(const T *string) : size(StringLength(string)) {
		AzAssert(size <= count, "BucketArray initialized with a size bigger than count");
		if constexpr (std::is_trivially_copyable<T>::value) {
			memcpy((void *)data, (void *)string, sizeof(T) * size);
		} else {
			for (i32 i = 0; i < size; i++) {
				data[i] = string[i];
			}
		}
	}

	BucketArray<T, count> &operator=(const BucketArray<T, count> &other) {
		size = other.size;
		if constexpr (std::is_trivially_copyable<T>::value) {
			memcpy((void *)data, (void *)other.data, sizeof(T) * size);
		} else {
			for (i32 i = 0; i < size; i++) {
				data[i] = other.data[i];
			}
		}
		return *this;
	}

	BucketArray<T, count> &operator=(const std::initializer_list<T> &init) {
		size = init.size();
		AzAssert(size <= count, "BucketArray assigned with a size bigger than count");
		if (size == 0) {
			return *this;
		}
		i32 i = 0;
		for (const T &val : init) {
			data[i++] = val;
		}
		return *this;
	}

	BucketArray<T, count> &operator=(const T *string) {
		size = StringLength(string);
		AzAssert(size <= count, "BucketArray assigned with a size bigger than count");
		if constexpr (std::is_trivially_copyable<T>::value) {
			memcpy((void *)data, (void *)string, sizeof(T) * size);
		} else {
			for (i32 i = 0; i < size; i++) {
				data[i] = string[i];
			}
		}
		return *this;
	}

	bool operator==(const BucketArray<T, count> &other) const {
		if (size != other.size) {
			return false;
		}
		for (i32 i = 0; i < size; i++) {
			if (data[i] != other.data[i]) {
				return false;
			}
		}
		return true;
	}

	bool operator==(const T *string) const {
		i32 i = 0;
		for (; string[i] != StringTerminators<T>::value && i < size; i++) {
			if (string[i] != data[i]) {
				return false;
			}
		}
		return i == size;
	}

	bool Contains(const T &val, i32 *dstIndex=nullptr) const {
		for (i32 i = 0; i < size; i++) {
			if (val == data[i]) {
				if (dstIndex) *dstIndex = i;
				return true;
			}
		}
		return false;
	}

	inline const T &operator[](i32 index) const {
		AzAssert(index < size && index >= 0, "BucketArray index is out of bounds");
		return data[index];
	}

	inline T &operator[](i32 index) {
		AzAssert(index < size && index >= 0, "BucketArray index is out of bounds");
		return data[index];
	}

	BucketArray<T, count> operator+(const T &other) const {
		BucketArray<T, count> result(*this);
		result.Append(other);
		return result;
	}

	BucketArray<T, count> operator+(const T *string) const {
		BucketArray<T, count> result(*this);
		result.Append(string);
		return result;
	}

	BucketArray<T, count> operator+(const BucketArray<T, count> &other) const {
		BucketArray<T, count> result(*this);
		result.Append(other);
		return result;
	}

	inline T &operator+=(const T &value) {
		return Append(value);
	}

	inline T &operator+=(const T &&value) {
		return Append(std::move(value));
	}

	inline BucketArray<T, count> &operator+=(const BucketArray<T, count> &other) {
		return Append(other);
	}

	inline BucketArray<T, count> &operator+=(const T *string) {
		return Append(string);
	}

	inline void Resize(i32 newSize, const T &value) {
		AzAssert(newSize <= count, "BucketArray Resized bigger than count");
		for (i32 i = size; i < newSize; i++) {
			data[i] = value;
		}
		size = newSize;
	}

	inline void Resize(i32 newSize) {
		AzAssert(newSize <= count, "BucketArray Resized bigger than count");
		size = newSize;
	}

	inline T &Append(const T &value) {
		AzAssert(size < count, "BucketArray Single-Append would overfill");
		return data[size++] = value;
	}

	inline T &Append(T &&value) {
		AzAssert(size < count, "BucketArray Single-Append would overfill");
		return data[size++] = std::move(value);
	}

	BucketArray<T, count> &Append(const T *string) {
		i32 newSize = size + StringLength(string);
		AzAssert(newSize <= count, "BucketArray C-string Append would overfill");
		for (i32 i = size; i < newSize; i++) {
			data[i] = string[i - size];
		}
		size = newSize;
		return *this;
	}

	BucketArray<T, count> &Append(const BucketArray<T, count> &other) {
		i32 copyStart = size;
		size += other.size;
		AzAssert(size <= count, "BucketArray Append would overfill");
		if constexpr (std::is_trivially_copyable<T>::value) {
			memcpy((void *)(data + copyStart), (void *)other.data, sizeof(T) * other.size);
		} else {
			for (i32 i = copyStart; i < size; i++) {
				data[i] = other.data[i - copyStart];
			}
		}
		return *this;
	}

	T &Insert(i32 index, const T &value) {
		T val(value);
		return Insert(index, std::move(val));
	}

	T &Insert(i32 index, T &&value) {
		AzAssert(index <= size && index >= 0, "BucketArray::Insert index is out of bounds");
		AzAssert(size < count, "BucketArray::Insert would overfill");
		for (i32 i = size++; i > index; i--) {
			data[i] = std::move(data[i - 1]);
		}
		return data[index] = value;
	}

	void Erase(i32 index) {
		AzAssert(index < size && index >= 0, "BucketArray::Erase index is out of bounds");
		size--;
		if constexpr (std::is_trivially_copyable<T>::value) {
			if (size > index) {
				memmove((void *)(data + index), (void *)(data + index + 1), sizeof(T) * (size - index));
			}
		} else {
			for (i32 i = index; i < size - 1; i++) {
				data[i] = std::move(data[i + 1]);
			}
		}
	}

	inline void Clear() {
		size = 0;
	}

	BucketArray<T, count> &Reverse() {
		for (i32 i = 0; i < size / 2; i++) {
			T buf = std::move(data[i]);
			data[i] = std::move(data[size - i - 1]);
			data[size - i - 1] = std::move(buf);
		}
		return *this;
	}

	T* begin() {
		return data;
	}
	T* end() {
		return data + size;
	}
	const T* begin() const {
		return data;
	}
	const T* end() const {
		return data + size;
	}

	inline T &Back() {
		return data[size - 1];
	}
};

#pragma pack()

} // namespace AzCore

#endif // AZCORE_BUCKETARRAY_HPP
