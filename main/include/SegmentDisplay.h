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
			return static_cast<T>(0) | ET::Top | ET::UpperRight | ET::LowerRight | ET::Bottom | ET::LowerLeft | ET::UpperLeft;
		case '1':
			return static_cast<T>(0) | ET::UpperRight | ET::LowerRight;
		case '2':
			return static_cast<T>(0) | ET::Top | ET::UpperRight | ET::Middle | ET::LowerLeft | ET::Bottom;
		case '3':
			return static_cast<T>(0) | ET::Top | ET::UpperRight | ET::Middle | ET::LowerRight | ET::Bottom;
		case '4':
			return static_cast<T>(0) | ET::UpperLeft | ET::Middle | ET::UpperRight | ET::LowerRight;
		case '5':
			return static_cast<T>(0) | ET::Top | ET::UpperLeft | ET::Middle | ET::LowerRight | ET::Bottom;
		case '6':
			return static_cast<T>(0) | ET::Top | ET::UpperLeft | ET::Middle | ET::LowerLeft | ET::LowerRight | ET::Bottom;
		case '7':
			return static_cast<T>(0) | ET::Top | ET::UpperRight | ET::LowerRight;
		case '8':
			return static_cast<T>(0) | ET::Top | ET::UpperRight | ET::LowerRight | ET::Bottom | ET::LowerLeft | ET::UpperLeft | ET::Middle;
		case '9':
			return static_cast<T>(0) | ET::Top | ET::UpperRight | ET::LowerRight | ET::Bottom | ET::UpperLeft | ET::Middle;

		case 'A':
		case 'a':
			return static_cast<T>(0) | ET::Top | ET::UpperRight | ET::LowerRight | ET::LowerLeft | ET::UpperLeft | ET::Middle;
		case 'B':
		case 'b':
			return static_cast<T>(0) | ET::LowerRight | ET::Bottom | ET::LowerLeft | ET::UpperLeft | ET::Middle;
		case 'C':
		case 'c':
			return static_cast<T>(0) | ET::Top | ET::Bottom | ET::LowerLeft | ET::UpperLeft;
		case 'D':
		case 'd':
			return static_cast<T>(0) | ET::UpperRight | ET::LowerRight | ET::Bottom | ET::LowerLeft | ET::Middle;
		case 'E':
		case 'e':
			return static_cast<T>(0) | ET::Top | ET::Bottom | ET::LowerLeft | ET::UpperLeft | ET::Middle;
		case 'F':
		case 'f':
			return static_cast<T>(0) | ET::Top | ET::LowerLeft | ET::UpperLeft | ET::Middle;

		default:
			return 0; // Unknown character, return static_cast<T>(0) |  0 (no segments on
		}
	}

	// Function template to map a character to a 7-segment bit pattern
	template <typename ET = Default16Segment, typename T = uint16_t>
	constexpr T char_to_16seg(char c)
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
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperRight | ET::LowerRight | ET::BottomLeft | ET::BottomRight | ET::LowerLeft | ET::UpperLeft;
		case '1':
			return static_cast<T>(0) | ET::UpperRight | ET::LowerRight;
		case '2':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperRight | ET::MiddleLeft | ET::MiddleRight | ET::LowerLeft | ET::BottomLeft | ET::BottomRight;
		case '3':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperRight | ET::MiddleLeft | ET::MiddleRight | ET::LowerRight | ET::BottomLeft | ET::BottomRight;
		case '4':
			return static_cast<T>(0) | ET::UpperLeft | ET::MiddleLeft | ET::MiddleRight | ET::UpperRight | ET::LowerRight;
		case '5':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperLeft | ET::MiddleLeft | ET::MiddleRight | ET::LowerRight | ET::BottomLeft | ET::BottomRight;
		case '6':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperLeft | ET::MiddleLeft | ET::MiddleRight | ET::LowerLeft | ET::LowerRight | ET::BottomLeft | ET::BottomRight;
		case '7':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperRight | ET::LowerRight;
		case '8':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperRight | ET::LowerRight | ET::BottomLeft | ET::BottomRight | ET::LowerLeft | ET::UpperLeft | ET::MiddleLeft | ET::MiddleRight;
		case '9':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperRight | ET::LowerRight | ET::BottomLeft | ET::BottomRight | ET::UpperLeft | ET::MiddleLeft | ET::MiddleRight;

		case 'A':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperRight | ET::LowerRight | ET::LowerLeft | ET::UpperLeft | ET::MiddleLeft | ET::MiddleRight;
		case 'B':
			return static_cast<T>(0) | ET::LowerRight | ET::BottomLeft | ET::BottomRight | ET::LowerLeft | ET::UpperLeft | ET::MiddleLeft | ET::MiddleRight;
		case 'C':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::BottomLeft | ET::BottomRight | ET::LowerLeft | ET::UpperLeft;
		case 'D':
			return static_cast<T>(0) | ET::UpperRight | ET::LowerRight | ET::BottomLeft | ET::BottomRight | ET::LowerLeft | ET::MiddleLeft | ET::MiddleRight;
		case 'E':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::BottomLeft | ET::BottomRight | ET::LowerLeft | ET::UpperLeft | ET::MiddleLeft | ET::MiddleRight;
		case 'F':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::LowerLeft | ET::UpperLeft | ET::MiddleLeft | ET::MiddleRight;

		case 'G':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperLeft | ET::MiddleRight | ET::LowerLeft | ET::LowerRight | ET::BottomLeft | ET::BottomRight;
		case 'H':
			return static_cast<T>(0) | ET::UpperRight | ET::LowerRight | ET::LowerLeft | ET::UpperLeft | ET::MiddleLeft | ET::MiddleRight;
		case 'I':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperCenter | ET::LowerCenter | ET::BottomLeft | ET::BottomRight;
		case 'J':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperRight | ET::LowerRight | ET::BottomLeft | ET::BottomRight;
		case 'K':
			return static_cast<T>(0) | ET::LowerLeft | ET::UpperLeft | ET::MiddleLeft | ET::DiagUpperRight | ET::DiagLowerRight;

		case 'L':
			return static_cast<T>(0) | ET::BottomLeft | ET::BottomRight | ET::LowerLeft | ET::UpperLeft;
		case 'M':
			return static_cast<T>(0) | ET::UpperRight | ET::LowerRight | ET::LowerLeft | ET::UpperLeft | ET::DiagUpperRight | ET::DiagUpperLeft;
		case 'N':
			return static_cast<T>(0) | ET::UpperRight | ET::LowerRight | ET::LowerLeft | ET::UpperLeft | ET::DiagLowerRight | ET::DiagUpperLeft;
		case 'O':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperRight | ET::LowerRight | ET::BottomLeft | ET::BottomRight | ET::LowerLeft | ET::UpperLeft;
		case 'P':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperRight | ET::LowerLeft | ET::UpperLeft | ET::MiddleLeft | ET::MiddleRight;

		case 'Q':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperRight | ET::LowerRight | ET::BottomLeft | ET::BottomRight | ET::LowerLeft | ET::UpperLeft | ET::DiagLowerRight;
		case 'R':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperRight | ET::LowerLeft | ET::UpperLeft | ET::MiddleLeft | ET::MiddleRight | ET::DiagLowerRight;
		case 'S':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::DiagUpperLeft | ET::MiddleLeft | ET::MiddleRight | ET::LowerRight | ET::BottomLeft | ET::BottomRight;
		case 'T':
			return static_cast<T>(0) | ET::TopLeft | ET::TopRight | ET::UpperCenter | ET::LowerCenter;
		case 'U':
			return static_cast<T>(0) | | ET::UpperRight | ET::LowerRight | ET::BottomLeft | ET::BottomRight | ET::LowerLeft | ET::UpperLeft;
		//
		case 'V':
			return static_cast<T>(0) | ET::LowerLeft | ET::UpperLeft | ET::DiagLowerLeft | ET::DiagUpperRight;
		case 'W':
			return static_cast<T>(0) | ET::UpperRight | ET::LowerRight | ET::LowerLeft | ET::UpperLeft | ET::DiagLowerRight | ET::DiagLowerLeft;
		case 'X':
			return static_cast<T>(0) | ET::DiagUpperRight | ET::DiagLowerRight | ET::DiagLowerLeft | ET::DiagUpperLeft;
		case 'Y':
			return static_cast<T>(0) | ET::DiagUpperRight | ET::DiagUpperLeft | ET::LowerCenter;
		case 'Z':
			return static_cast<T>(0) ET::TopLeft | ET::TopRight | ET::DiagUpperRight | ET::DiagLowerLeft | ET::BottomLeft | ET::BottomRight;
		
		default:
			return 0; // Unknown character, return static_cast<T>(0) |  0 (no segments on
		}
	}

}

#endif
