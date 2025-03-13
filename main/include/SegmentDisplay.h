#ifndef MCP230XX_H
#define MCP230XX_H

#include <cstdint>

#include <vector>

// Enum-like structure defining the 7-segment display segments

namespace SegmentDisplay
{
	// Heights: Top, Upper, Middle, Lower, Bottom
	// Sides: Left, Center, Right

	enum class Default7Segment
	{
		None = 0,
		Top = 1 << 0,
		UpperRight = 1 << 1,
		LowerRight = 1 << 2,
		Bottom = 1 << 3,
		LowerLeft = 1 << 4,
		UpperLeft = 1 << 5,
		Middle = 1 << 6,
	};

	enum class Default16Segment
	{
		None = 0,
		TopLeft = 1 << 0,
		TopRight = 1 << 1,
		UpperRight = 1 << 2,
		LowerRight = 1 << 3,
		BottomRight = 1 << 4,
		BottomLeft = 1 << 5,
		LowerLeft = 1 << 6,
		UpperLeft = 1 << 7,

		MiddleLeft = 1 << 8,
		MiddleRight = 1 << 9,
		UpperCenter = 1 << 10,
		LowerCenter = 1 << 11,

		DiagUpperRight = 1 << 12,
		DiagLowerRight = 1 << 13,
		DiagLowerLeft = 1 << 14,
		DiagUpperLeft = 1 << 15,
	};

	// Function template to map a character to a 7-segment bit pattern
	template <typename ET = Default7Segment, typename T = uint8_t>
	constexpr T char_to_7seg(char c)
	{
		c = std::toupper((unsigned char)c);

		switch (c)
		{
		case '0':
			return custom_char_seg<ET, T>({ET::Top, ET::UpperRight, ET::LowerRight, ET::Bottom, ET::LowerLeft, ET::UpperLeft});
		case '1':
			return custom_char_seg<ET, T>({ET::UpperRight, ET::LowerRight});
		case '2':
			return custom_char_seg<ET, T>({ET::Top, ET::UpperRight, ET::Middle, ET::LowerLeft, ET::Bottom});
		case '3':
			return custom_char_seg<ET, T>({ET::Top, ET::UpperRight, ET::Middle, ET::LowerRight, ET::Bottom});
		case '4':
			return custom_char_seg<ET, T>({ET::UpperLeft, ET::Middle, ET::UpperRight, ET::LowerRight});
		case '5':
			return custom_char_seg<ET, T>({ET::Top, ET::UpperLeft, ET::Middle, ET::LowerRight, ET::Bottom});
		case '6':
			return custom_char_seg<ET, T>({ET::Top, ET::UpperLeft, ET::Middle, ET::LowerLeft, ET::LowerRight, ET::Bottom});
		case '7':
			return custom_char_seg<ET, T>({ET::Top, ET::UpperRight, ET::LowerRight});
		case '8':
			return custom_char_seg<ET, T>({ET::Top, ET::UpperRight, ET::LowerRight, ET::Bottom, ET::LowerLeft, ET::UpperLeft, ET::Middle});
		case '9':
			return custom_char_seg<ET, T>({ET::Top, ET::UpperRight, ET::LowerRight, ET::Bottom, ET::UpperLeft, ET::Middle});

		case 'A':
			return custom_char_seg<ET, T>({ET::Top, ET::UpperRight, ET::LowerRight, ET::LowerLeft, ET::UpperLeft, ET::Middle});
		case 'B':
			return custom_char_seg<ET, T>({ET::LowerRight, ET::Bottom, ET::LowerLeft, ET::UpperLeft, ET::Middle});
		case 'C':
			return custom_char_seg<ET, T>({ET::Top, ET::Bottom, ET::LowerLeft, ET::UpperLeft});
		case 'D':
			return custom_char_seg<ET, T>({ET::UpperRight, ET::LowerRight, ET::Bottom, ET::LowerLeft, ET::Middle});
		case 'E':
			return custom_char_seg<ET, T>({ET::Top, ET::Bottom, ET::LowerLeft, ET::UpperLeft, ET::Middle});
		case 'F':
			return custom_char_seg<ET, T>({ET::Top, ET::LowerLeft, ET::UpperLeft, ET::Middle});

		case ' ':
			return custom_char_seg<ET, T>({});

		default:
			return 0; // Unknown character, return custom_char_seg<ET, T>({ 0 (no segments on
		}
	}

	// Function template to map a character to a 7-segment bit pattern
	template <typename ET = Default16Segment, typename T = uint16_t>
	constexpr T char_to_16seg(char c)
	{
		c = std::toupper((unsigned char)c);

		switch (c)
		{
		case '0':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperRight, ET::LowerRight, ET::BottomLeft, ET::BottomRight, ET::LowerLeft, ET::UpperLeft});
		case '1':
			return custom_char_seg<ET, T>({ET::UpperRight, ET::LowerRight});
		case '2':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperRight, ET::MiddleLeft, ET::MiddleRight, ET::LowerLeft, ET::BottomLeft, ET::BottomRight});
		case '3':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperRight, ET::MiddleLeft, ET::MiddleRight, ET::LowerRight, ET::BottomLeft, ET::BottomRight});
		case '4':
			return custom_char_seg<ET, T>({ET::UpperLeft, ET::MiddleLeft, ET::MiddleRight, ET::UpperRight, ET::LowerRight});
		case '5':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperLeft, ET::MiddleLeft, ET::MiddleRight, ET::LowerRight, ET::BottomLeft, ET::BottomRight});
		case '6':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperLeft, ET::MiddleLeft, ET::MiddleRight, ET::LowerLeft, ET::LowerRight, ET::BottomLeft, ET::BottomRight});
		case '7':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperRight, ET::LowerRight});
		case '8':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperRight, ET::LowerRight, ET::BottomLeft, ET::BottomRight, ET::LowerLeft, ET::UpperLeft, ET::MiddleLeft, ET::MiddleRight});
		case '9':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperRight, ET::LowerRight, ET::BottomLeft, ET::BottomRight, ET::UpperLeft, ET::MiddleLeft, ET::MiddleRight});

		case 'A':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperRight, ET::LowerRight, ET::LowerLeft, ET::UpperLeft, ET::MiddleLeft, ET::MiddleRight});
		case 'B':
			return custom_char_seg<ET, T>({ET::LowerRight, ET::BottomLeft, ET::BottomRight, ET::LowerLeft, ET::UpperLeft, ET::MiddleLeft, ET::MiddleRight});
		case 'C':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::BottomLeft, ET::BottomRight, ET::LowerLeft, ET::UpperLeft});
		case 'D':
			return custom_char_seg<ET, T>({ET::UpperRight, ET::LowerRight, ET::BottomLeft, ET::BottomRight, ET::LowerLeft, ET::MiddleLeft, ET::MiddleRight});
		case 'E':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::BottomLeft, ET::BottomRight, ET::LowerLeft, ET::UpperLeft, ET::MiddleLeft, ET::MiddleRight});
		case 'F':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::LowerLeft, ET::UpperLeft, ET::MiddleLeft, ET::MiddleRight});

		case 'G':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperLeft, ET::MiddleRight, ET::LowerLeft, ET::LowerRight, ET::BottomLeft, ET::BottomRight});
		case 'H':
			return custom_char_seg<ET, T>({ET::UpperRight, ET::LowerRight, ET::LowerLeft, ET::UpperLeft, ET::MiddleLeft, ET::MiddleRight});
		case 'I':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperCenter, ET::LowerCenter, ET::BottomLeft, ET::BottomRight});
		case 'J':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperRight, ET::LowerRight, ET::BottomLeft, ET::BottomRight});
		case 'K':
			return custom_char_seg<ET, T>({ET::LowerLeft, ET::UpperLeft, ET::MiddleLeft, ET::DiagUpperRight, ET::DiagLowerRight});

		case 'L':
			return custom_char_seg<ET, T>({ET::BottomLeft, ET::BottomRight, ET::LowerLeft, ET::UpperLeft});
		case 'M':
			return custom_char_seg<ET, T>({ET::UpperRight, ET::LowerRight, ET::LowerLeft, ET::UpperLeft, ET::DiagUpperRight, ET::DiagUpperLeft});
		case 'N':
			return custom_char_seg<ET, T>({ET::UpperRight, ET::LowerRight, ET::LowerLeft, ET::UpperLeft, ET::DiagLowerRight, ET::DiagUpperLeft});
		case 'O':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperRight, ET::LowerRight, ET::BottomLeft, ET::BottomRight, ET::LowerLeft, ET::UpperLeft});
		case 'P':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperRight, ET::LowerLeft, ET::UpperLeft, ET::MiddleLeft, ET::MiddleRight});

		case 'Q':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperRight, ET::LowerRight, ET::BottomLeft, ET::BottomRight, ET::LowerLeft, ET::UpperLeft, ET::DiagLowerRight});
		case 'R':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperRight, ET::LowerLeft, ET::UpperLeft, ET::MiddleLeft, ET::MiddleRight, ET::DiagLowerRight});
		case 'S':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::DiagUpperLeft, ET::MiddleLeft, ET::MiddleRight, ET::LowerRight, ET::BottomLeft, ET::BottomRight});
		case 'T':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::UpperCenter, ET::LowerCenter});
		case 'U':
			return custom_char_seg<ET, T>({ | ET::UpperRight, ET::LowerRight, ET::BottomLeft, ET::BottomRight, ET::LowerLeft, ET::UpperLeft});
		//
		case 'V':
			return custom_char_seg<ET, T>({ET::LowerLeft, ET::UpperLeft, ET::DiagLowerLeft, ET::DiagUpperRight});
		case 'W':
			return custom_char_seg<ET, T>({ET::UpperRight, ET::LowerRight, ET::LowerLeft, ET::UpperLeft, ET::DiagLowerRight, ET::DiagLowerLeft});
		case 'X':
			return custom_char_seg<ET, T>({ET::DiagUpperRight, ET::DiagLowerRight, ET::DiagLowerLeft, ET::DiagUpperLeft});
		case 'Y':
			return custom_char_seg<ET, T>({ET::DiagUpperRight, ET::DiagUpperLeft, ET::LowerCenter});
		case 'Z':
			return custom_char_seg<ET, T>({ET::TopLeft, ET::TopRight, ET::DiagUpperRight, ET::DiagLowerLeft, ET::BottomLeft, ET::BottomRight});

		case ' ':
			return custom_char_seg<ET, T>({});

		default:
			return 0; // Unknown character, return 0 (no segments on)
		}
	}

	// Function template to create custom char
	template <typename ET = Default16Segment, typename T = uint16_t>
	constexpr T custom_char_seg(const std::vector<ET> &segs)
	{
		static_assert(std::is_integral<T>::value, "Return type must be an integer");
		// static_assert(std::is_unsigned<T>::value, "Return type must be unsigned");

		T aggr = 0;

		for (ET seg : segs)
			aggr |= static_cast<T>(seg);

		return aggr;
	}

}

#endif
