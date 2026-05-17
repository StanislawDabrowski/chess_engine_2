#pragma once
#include <cstdint>


namespace MoveOrdering
{
	static constexpr int16_t MoveType_score_no_check[14] = {
		30,   //KnightMove
		30,   //BishopMove
		50,   //RookMove
		90,   //QueenMove
		0,    //KingMove
		10,   //PawnSinglePush
		10,   //PawnDoublePush
		200,  //PawnCapture
		15,   //EnPassant
		40,   //Castle
		500,  //PromotionToQueen
		-300, //PromotionToKnight
		-300, //PromotionToRook
		-300, //PromotionToBishop
	};
	static constexpr int16_t MoveType_score_in_check[14] = {
		20,   //KnightMove
		20,   //BishopMove
		15,   //RookMove
		10,   //QueenMove
		40,   //KingMove
		30,   //PawnSinglePush
		30,   //PawnDoublePush
		150,  //PawnCapture
		70,   //EnPassant - if en passant is legal while we are in check that measn the checing piece is the panw we are taking (ther is no possible position where the en passant will interpose a check)
		0,    //Castle - never occurs in check
		500,  //PromotionToQueen
		-300, //PromotionToKnight
		-300, //PromotionToRook
		-300, //PromotionToBishop 
	};
	static constexpr int16_t MoveType_score_qsearch_no_check[14] = {//MoveType_score_no_check but for qsearch, when we are not in check, in qsearch all moves are captures, promotions or checks, unless we are in check, so the scores are different
		50,   //KnightMove
		50,   //BishopMove
		30,   //RookMove
		15,   //QueenMove
		0,    //KingMove - never occurs in qsearch
		80,   //PawnSinglePush - always a check
		80,   //PawnDoublePush - always a check
		150,  //PawnCapture
		100,  //EnPassant - always a check - because of the very unique nature of en passant, en passant check exposes king significantly, hence in that specific case it's a very promising move
		0,    //Castle - never occurs in qsearch
		500,  //PromotionToQueen
		0,    //PromotionToKnight - never occurs in qsearch
		0,    //PromotionToRook - never occurs in qsearch
		0,    //PromotionToBishop - never occurs in qsearch
	};
	

}

