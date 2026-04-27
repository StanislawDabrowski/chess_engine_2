#pragma once
#include <cstdint>

enum PieceType : uint8_t
{
	Pawn = 0,
	Knight = 1,
	Bishop = 2,
	Rook = 3,
	Queen = 4,
	King = 5,
	Empty = 6,
	None = 7
};
