#pragma once
#include <string>
#include <iostream>
#include <Board.h>
#include "PieceType.h"
#include "MoveType.h"
#include "Color.h"


namespace Utils
{
	
	// KnightMove = 0,
	// BishopMove = 1,
	// RookMove = 2,
	// QueenMove = 3,
	// KingMove = 4,
	// PawnSinglePush = 5,
	// PawnDoublePush = 6,
	// PawnCapture = 7,
	// EnPassant = 8,
	// Castle = 9,
	// PromotionToQueen = 10,
	// PromotionToKnight = 11,
	// PromotionToRook = 12,
	// PromotionToBishop = 13,
	
	constexpr size_t MoveType_to_PieceType[14] = {Knight, Bishop, Rook, Queen, King, Pawn, Pawn, Pawn, Pawn, King, Pawn, Pawn, Pawn, Pawn};
	constexpr size_t MoveType_to_promotion_piece[14] = {None, None, None, None, None, None, None, None, None, None, Queen, Knight, Rook, Bishop};

	//masks
	constexpr Bitboard FILE_A = 0x0101010101010101ULL;
	constexpr Bitboard FILE_B = 0x0202020202020202ULL;
	constexpr Bitboard FILE_C = 0x0404040404040404ULL;
	constexpr Bitboard FILE_D = 0x0808080808080808ULL;
	constexpr Bitboard FILE_E = 0x1010101010101010ULL;
	constexpr Bitboard FILE_F = 0x2020202020202020ULL;
	constexpr Bitboard FILE_G = 0x4040404040404040ULL;
	constexpr Bitboard FILE_H = 0x8080808080808080ULL;

	constexpr Bitboard RANK_1 = 0x00000000000000FFULL;
	constexpr Bitboard RANK_2 = 0x000000000000FF00ULL;
	constexpr Bitboard RANK_3 = 0x0000000000FF0000ULL;
	constexpr Bitboard RANK_4 = 0x00000000FF000000ULL;
	constexpr Bitboard RANK_5 = 0x000000FF00000000ULL;
	constexpr Bitboard RANK_6 = 0x0000FF0000000000ULL;
	constexpr Bitboard RANK_7 = 0x00FF000000000000ULL;
	constexpr Bitboard RANK_8 = 0xFF00000000000000ULL;

	constexpr Bitboard FILE_A_NEGATION = ~FILE_A;
	constexpr Bitboard FILE_B_NEGATION = ~FILE_B;
	constexpr Bitboard FILE_C_NEGATION = ~FILE_C;
	constexpr Bitboard FILE_D_NEGATION = ~FILE_D;
	constexpr Bitboard FILE_E_NEGATION = ~FILE_E;
	constexpr Bitboard FILE_F_NEGATION = ~FILE_F;
	constexpr Bitboard FILE_G_NEGATION = ~FILE_G;
	constexpr Bitboard FILE_H_NEGATION = ~FILE_H;

	constexpr Bitboard RANK_1_NEGATION = ~RANK_1;
	constexpr Bitboard RANK_2_NEGATION = ~RANK_2;
	constexpr Bitboard RANK_3_NEGATION = ~RANK_3;
	constexpr Bitboard RANK_4_NEGATION = ~RANK_4;
	constexpr Bitboard RANK_5_NEGATION = ~RANK_5;
	constexpr Bitboard RANK_6_NEGATION = ~RANK_6;
	constexpr Bitboard RANK_7_NEGATION = ~RANK_7;
	constexpr Bitboard RANK_8_NEGATION = ~RANK_8;

	//masks for promotins with captures
	constexpr Bitboard RANK_2_WITHOUT_A_FILE = RANK_2 & FILE_A_NEGATION;//black left capture
	constexpr Bitboard RANK_7_WITHOUT_A_FILE = RANK_7 & FILE_H_NEGATION;//white left capture
	constexpr Bitboard RANK_2_WITHOUT_H_FILE = RANK_2 & FILE_H_NEGATION;//black right capture
	constexpr Bitboard RANK_7_WITHOUT_H_FILE = RANK_7 & FILE_A_NEGATION;//white right capture

	constexpr Bitboard WHITE_KINGSIDE_CASTLE_MASK = 0b11ULL << 5;
	constexpr Bitboard WHITE_QUEENSIDE_CASTLE_MASK = 0b111ULL << 1;
	constexpr Bitboard BLACK_KINGSIDE_CASTLE_MASK = 0b11ULL << 61;
	constexpr Bitboard BLACK_QUEENSIDE_CASTLE_MASK = 0b111ULL << 57;

	constexpr uint16_t WHITE_KINGSIDE_CASTLE_FROM_TO_MASK = 4 | (6 << 6);
	constexpr uint16_t WHITE_QUEENSIDE_CASTLE_FROM_TO_MASK = 4 | (2 << 6);
	constexpr uint16_t BLACK_KINGSIDE_CASTLE_FROM_TO_MASK = 60 | (62 << 6);
	constexpr uint16_t BLACK_QUEENSIDE_CASTLE_FROM_TO_MASK = 60 | (58 << 6);



	void update_collective_bitboards(Board* board);
	void initialize_board(Board* board);
	std::string PieceType_to_string(PieceType piece_type);
	char PieceType_to_char(PieceType piece_type, Color color);
	std::pair<PieceType, Color> char_to_PieceType_and_Color(char c);
	void put_piece(BoardState* board_state, PieceType piece, Color color, uint8_t position);
	std::string get_fen(Board* board);
	void display_board(Board* board, std::ostream& output=std::cout);
	void display_board_each_piece_and_side_separately(Board* board, std::ostream& output=std::cout);
	std::string move_to_string(Move move);
	PieceType get_piece_on_square(Board* board, uint8_t square);
	MoveType get_move_type_from_squares(Board* board, uint8_t from_square, uint8_t to_square);
	Move string_to_move(Board* board, std::string move);
	bool MoveType_is_not_promotion(MoveType move_type);
	Bitboard occupancy_from_index(uint32_t index, uint64_t mask);
}
