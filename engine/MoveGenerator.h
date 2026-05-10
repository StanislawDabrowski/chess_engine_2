#pragma once
#include <cstdint>
#include <cstddef>
#include "Defs.h"
#include "Color.h"


class Board;

class MoveGenerator
{
private:
	Board* const board;

	constexpr static size_t max_pseudo_legal_moves_count = 512;
	Move pseudo_legal_moves[max_pseudo_legal_moves_count];//save psudo legal moves limit
	size_t pseudo_legal_moves_length;

	void generate_relevant_blockers_and_square_blockers();	
	void generate_square_bolckers();
	template<uint8_t castle_type>
	bool can_castle();//0 - white kingside, 1 - white queenside, 2 - black kingside, 3 - black queenside
	void add_legal_move(const Move move);
public:


	Bitboard bishop_relevant_blockers[64];
	Bitboard rook_relevant_blockers[64];
	Bitboard bishop_attack_tables[64][512] = {0};//max 512 possible attacks for a bishop on any square
	Bitboard rook_attack_tables[64][4096] = {0};//max 4096 possible attacks for a rook on any square
	Bitboard knight_attack_tables[64];
	Bitboard king_attack_tables[64];
	Bitboard pawn_attack_tables[2][64];

	//blockers for a specific square (used esspecialy for legal move generations for checking what blocks a check). e.g. for bishop for [3][30] (which is [d1][g4]) we get a bitboard with squares e2 and f3. blockers are generated unde an assumoption all squares between 2 squares are empty i.e. a requested square is attacked by a specific piece from a speicifc square.
	Bitboard bishop_square_blockers[64][64];//[square_of_interest][bishop_square]
	Bitboard rook_square_blockers[64][64];//[square_of_interest][rook_square]

	//masks of pieces which are the actual blockers for each square
	Bitboard bishop_blockers[64][512];
	Bitboard rook_blockers[64][4096];

	static uint64_t bishop_magic_numbers[64];
	static uint64_t rook_magic_numbers[64];


	//knights
	Bitboard white_kingside_castle_knight_attack_mask;
	Bitboard white_queenside_castle_knight_attack_mask;
	Bitboard black_kingside_castle_knight_attack_mask;
	Bitboard black_queenside_castle_knight_attack_mask;

	//pawns
	Bitboard white_kingside_castle_pawn_attack_mask;
	Bitboard white_queenside_castle_pawn_attack_mask;
	Bitboard black_kingside_castle_pawn_attack_mask;
	Bitboard black_queenside_castle_pawn_attack_mask;

	//king
	Bitboard white_kingside_castle_king_attack_mask;
	Bitboard white_queenside_castle_king_attack_mask;
	Bitboard black_kingside_castle_king_attack_mask;
	Bitboard black_queenside_castle_king_attack_mask;


	static constexpr size_t max_legal_moves_count = 256;//218 is a theoritiacl limit
	Move legal_moves[MoveGenerator::max_legal_moves_count];

	uint8_t checks;//each bit represents whether specific pices gives a check to the king of the side having a move. order is the same as in PieceType enum (e.g. 3rd bit (check & (1<<2)) coresponds to bishop because Bishop==2)



	MoveGenerator(Board* board);
	void initialize_attack_tables();
	Bitboard generate_rook_attack_table(uint8_t square, uint16_t occupancy_index);
	Bitboard generate_bishop_attack_table(uint8_t square, uint16_t occupancy_index);
	template<Color color>
	void generate_pseudo_legal_moves();
	template<Color color>
	void filter_pseudo_legal_moves();
	template<Color color>
	bool in_check() const;
};

