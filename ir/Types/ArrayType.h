///
/// @file ArrayType.h
/// @brief 数组类型类，支持一维数组和多维数组
///
/// @author Jiu-Zz
/// @version 1.0
/// @date 2026-05-11
///
#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "StorageSet.h"
#include "Type.h"

class ArrayType final : public Type {
	struct ArrayTypeHasher final {
		size_t operator()(const ArrayType & type) const noexcept
		{
			size_t seed = std::hash<const Type *>{}(type.getElementType());
			for (auto dim: type.getDimensions()) {
				seed ^= std::hash<int64_t>{}(dim) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}
	};

	struct ArrayTypeEqual final {
		bool operator()(const ArrayType & lhs, const ArrayType & rhs) const noexcept
		{
			return lhs.getElementType() == rhs.getElementType() && lhs.getDimensions() == rhs.getDimensions();
		}
	};

public:
	ArrayType(const Type * elementType, std::vector<int64_t> dims)
		: Type(Type::ArrayTyID), elementType(elementType), dims(std::move(dims))
	{}

	[[nodiscard]] const Type * getElementType() const
	{
		return elementType;
	}

	[[nodiscard]] const std::vector<int64_t> & getDimensions() const
	{
		return dims;
	}

	[[nodiscard]] bool isPointerLike() const
	{
		return !dims.empty() && dims.front() == 0;
	}

	[[nodiscard]] int32_t getSize() const override
	{
		if (dims.empty()) {
			return elementType ? elementType->getSize() : 0;
		}

		if (isPointerLike()) {
			return 4;
		}

		int64_t total = elementType ? elementType->getSize() : 0;
		for (auto dim: dims) {
			total *= (dim > 0 ? dim : 1);
		}
		return static_cast<int32_t>(total);
	}

	[[nodiscard]] std::string toString() const override
	{
		std::string inner = elementType ? elementType->toString() : "i32";
		for (auto it = dims.rbegin(); it != dims.rend(); ++it) {
			inner = "[" + std::to_string(*it) + " x " + inner + "]";
		}
		return inner;
	}

	[[nodiscard]] std::string getLegacyElementTypeString() const
	{
		return elementType ? elementType->toString() : "i32";
	}

	[[nodiscard]] std::string getLegacyDimensionsString() const
	{
		std::string suffix;
		for (auto dim: dims) {
			suffix += "[" + std::to_string(dim) + "]";
		}
		return suffix;
	}

	static const ArrayType * get(const Type * elementType, std::vector<int64_t> dims)
	{
		static StorageSet<ArrayType, ArrayTypeHasher, ArrayTypeEqual> storageSet;
		return storageSet.get(elementType, std::move(dims));
	}

private:
	const Type * elementType = nullptr;
	std::vector<int64_t> dims;
};
