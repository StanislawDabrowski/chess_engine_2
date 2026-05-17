#pragma once
#include <cstdint>


namespace MoveOrdering
{
	static constexpr int16_t MoveType_score[14] = {
		30, //KnightMove;
		30, //BishopMove;
		50, //RookMove;
		90, //QueenMove;
		0,   //KingMove;
		10, //PawnSinglePush;
		10, //PawnDoublePush;
		200, //PawnCapture;
		15, //EnPassant
		40, //Castle
		500, //PromotionToQueen
		-300, //PromotionToKnight
		-300, //PromotionToRook
		-300, //PromotionToBishop
	};
}

