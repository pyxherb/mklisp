#ifndef _MKLISP_VALUE_H_
#define _MKLISP_VALUE_H_

#include <cstdint>
#include "basedefs.h"

namespace mklisp {
	enum class ValueType : uint8_t {
		Undefined = 0,
		Nil,
		Int,
		UInt,
		Long,
		ULong,
		Short,
		UShort,
		Byte,
		UByte,
		Char,
		Object,
		QuotedObject
	};

	class Object;

	struct Value {
		union {
			int32_t asInt;
			uint32_t asUInt;
			int64_t asLong;
			uint64_t asULong;
			int16_t asShort;
			uint16_t asUShort;
			int8_t asByte;
			uint8_t asUByte;
			char32_t asChar;
			Object *asObject;
		} exData;
		ValueType valueType;

		MKLISP_FORCEINLINE Value() : valueType(ValueType::Undefined) {}
		MKLISP_FORCEINLINE Value(ValueType valueType) : valueType(valueType) {}

		MKLISP_FORCEINLINE Value(int32_t data) : valueType(ValueType::Int) {
			exData.asInt = data;
		}
		MKLISP_FORCEINLINE Value(uint32_t data) : valueType(ValueType::UInt) {
			exData.asUInt = data;
		}
		MKLISP_FORCEINLINE Value(int64_t data) : valueType(ValueType::Long) {
			exData.asLong = data;
		}
		MKLISP_FORCEINLINE Value(uint64_t data) : valueType(ValueType::ULong) {
			exData.asULong = data;
		}
		MKLISP_FORCEINLINE Value(int16_t data) : valueType(ValueType::Short) {
			exData.asShort = data;
		}
		MKLISP_FORCEINLINE Value(uint16_t data) : valueType(ValueType::UShort) {
			exData.asUShort = data;
		}
		MKLISP_FORCEINLINE Value(int8_t data) : valueType(ValueType::Byte) {
			exData.asByte = data;
		}
		MKLISP_FORCEINLINE Value(uint8_t data) : valueType(ValueType::UByte) {
			exData.asUByte = data;
		}
		MKLISP_FORCEINLINE Value(char32_t data) : valueType(ValueType::Char) {
			exData.asChar = data;
		}
		MKLISP_FORCEINLINE Value(Object *data, bool quoted = false) : valueType(quoted ? ValueType::QuotedObject : ValueType::Object) {
			exData.asObject = data;
		}
	};
}

#endif
