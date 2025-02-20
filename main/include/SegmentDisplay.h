#ifndef MCP230XX_H
#define MCP230XX_H

#include <cstdint>

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
		static_assert(std::is_integral<T>::value, "Return type must be an integer");
		// static_assert(std::is_unsigned<T>::value, "Return type must be unsigned");

		inline constexpr T operator|(T aggr, ET seg)
		{
			// return aggr | static_cast<T>(1 << static_cast<std::underlying_type_t>(seg));
			return aggr | static_cast<T>(seg);
		}

		switch (c)
		{
		case '0':
			return 0 | ET::Top | ET::UpperRight | ET::LowerRight | ET::Bottom | ET::LowerLeft | ET::UpperLeft;
		case '1':
			return 0 | ET::UpperRight | ET::LowerRight;
		case '2':
			return 0 | ET::Top | ET::UpperRight | ET::Middle | ET::LowerLeft | ET::Bottom;
		case '3':
			return 0 | ET::Top | ET::UpperRight | ET::Middle | ET::LowerRight | ET::Bottom;
		case '4':
			return 0 | ET::UpperLeft | ET::Middle | ET::UpperRight | ET::LowerRight;
		case '5':
			return 0 | ET::Top | ET::UpperLeft | ET::Middle | ET::LowerRight | ET::Bottom;
		case '6':
			return 0 | ET::Top | ET::UpperLeft | ET::Middle | ET::LowerLeft | ET::LowerRight | ET::Bottom;
		case '7':
			return 0 | ET::Top | ET::UpperRight | ET::LowerRight;
		case '8':
			return 0 | ET::Top | ET::UpperRight | ET::LowerRight | ET::Bottom | ET::LowerLeft | ET::UpperLeft | ET::Middle;
		case '9':
			return 0 | ET::Top | ET::UpperRight | ET::LowerRight | ET::Bottom | ET::UpperLeft | ET::Middle;
		case 'A':
		case 'a':
			return 0 | ET::Top | ET::UpperRight | ET::LowerRight | ET::LowerLeft | ET::UpperLeft | ET::Middle;
		case 'B':
		case 'b':
			return 0 | ET::LowerRight | ET::Bottom | ET::LowerLeft | ET::UpperLeft | ET::Middle;
		case 'C':
		case 'c':
			return 0 | ET::Top | ET::Bottom | ET::LowerLeft | ET::UpperLeft;
		case 'D':
		case 'd':
			return 0 | ET::UpperRight | ET::LowerRight | ET::Bottom | ET::LowerLeft | ET::Middle;
		case 'E':
		case 'e':
			return 0 | ET::Top | ET::Bottom | ET::LowerLeft | ET::UpperLeft | ET::Middle;
		case 'F':
		case 'f':
			return 0 | ET::Top | ET::LowerLeft | ET::UpperLeft | ET::Middle;
		default:
			return 0; // Unknown character, return 0 |  0 (no segments on
		}
	}

}

#endif