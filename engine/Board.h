#pragma once
#include <cstdint>
#include "BoardRecord.h"
#include <string>

class Board
{
public:
	static uint64_t zobrist_pieces_hashes[2][6][64];
	static uint64_t zobrist_castling_hashes[16];
	static uint64_t zobrist_en_passant_hashes[64];
	static uint64_t zobrist_side_to_move_hash;

	static constexpr uint16_t MAXIMUM_GAME_LENGTH = 2048;
	BoardState positions_stack[MAXIMUM_GAME_LENGTH];
	size_t current_position_idx;
	size_t side_to_move; // 0 - white, 1 - black

	uint8_t castling_mask[64];
	uint32_t initial_fullmove_count;

	Board();
	static void initialize_hashes();
	void initialize_castling_mask();
	void load_fen(std::string fen);
	void make_move(Move move);
	void unmake_move();
	void calculate_hash();
};
