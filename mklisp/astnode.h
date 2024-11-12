#ifndef _MKLISP_ASTNODE_H_
#define _MKLISP_ASTNODE_H_

#include <cstddef>
#include <cstdint>

namespace mklisp {
	struct SourcePosition {
		size_t line, column;

		inline SourcePosition() : line(SIZE_MAX), column(SIZE_MAX) {}
		inline SourcePosition(size_t line, size_t column) : line(line), column(column) {}

		inline bool operator<(const SourcePosition &loc) const {
			if (line < loc.line)
				return true;
			if (line > loc.line)
				return false;
			return column < loc.column;
		}

		inline bool operator>(const SourcePosition &loc) const {
			if (line > loc.line)
				return true;
			if (line < loc.line)
				return false;
			return column > loc.column;
		}

		inline bool operator==(const SourcePosition &loc) const {
			return (line == loc.line) && (column == loc.column);
		}

		inline bool operator>=(const SourcePosition &loc) const {
			return ((*this) == loc) || ((*this) > loc);
		}

		inline bool operator<=(const SourcePosition &loc) const {
			return ((*this) == loc) || ((*this) < loc);
		}
	};

	struct SourceLocation {
		SourcePosition beginPosition, endPosition;
	};

	struct SourceDocument;

	struct TokenRange {
		SourceDocument *document = nullptr;
		size_t beginIndex = SIZE_MAX, endIndex = SIZE_MAX;

		inline TokenRange() = default;
		inline TokenRange(SourceDocument *document, size_t index) : document(document), beginIndex(index), endIndex(index) {}
		inline TokenRange(SourceDocument *document, size_t beginIndex, size_t endIndex) : document(document), beginIndex(beginIndex), endIndex(endIndex) {}

		operator bool() {
			return beginIndex != SIZE_MAX;
		}
	};
}

#endif
