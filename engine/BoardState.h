#pragma once
#include "Defs.h"
#include <cstdint>

struct BoardState
{
	static constexpr int maximum_number_of_legal_moves = 255;
	Move legal_moves[maximum_number_of_legal_moves];
	Bitboard pieces[2][6]; // [color][piece_type]
	Bitboard all_pipeces_types[2];//all pieces of a specified color
	Bitboard all_pieces;
	uint8_t legal_move_next_idx;
	uint8_t en_passant_square; // 0 if no en passant square
	uint8_t halfmove_clock; // for fifty-move rule
	uint8_t castling_rights; // bit 0 - white kingside, bit 1 - white queenside, bit 2 - black kingside, bit 3 - black queenside
};
