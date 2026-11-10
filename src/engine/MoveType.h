#pragma once
#include <cstdint>

enum MoveType : uint8_t
{
	KnightMove = 0,
	BishopMove = 1,
	RookMove = 2,
	QueenMove = 3,
	KingMove = 4,
	PawnSinglePush = 5,
	PawnDoublePush = 6,
	PawnCapture = 7,
	EnPassant = 8,
	Castle = 9,
	PromotionToQueen = 10,
	PromotionToKnight = 11,
	PromotionToRook = 12,
	PromotionToBishop = 13,
};
