#pragma once
#include <cstdint>
#include <cstddef>
#include "Defs.h"
#include "Color.h"


class Board;

class MoveGenerator
{
private:
	Board* board;

	constexpr static size_t max_pseudo_legal_moves_count = 512;
	Move pseudo_legal_moves[max_pseudo_legal_moves_count];//save psudo legal moves limit
	size_t pseudo_legal_moves_length;

	static void generate_relevant_blockers_and_square_blockers();	
	static void initialize_attack_tables();
	static void generate_square_bolckers();
	template<uint8_t castle_type>
	bool can_castle();//0 - white kingside, 1 - white queenside, 2 - black kingside, 3 - black queenside
	void add_legal_move(const Move move);
public:


	static Bitboard bishop_relevant_blockers[64];
	static Bitboard rook_relevant_blockers[64];
	static Bitboard bishop_attack_tables[64][512];//max 512 possible attacks for a bishop on any square
	static Bitboard rook_attack_tables[64][4096];//max 4096 possible attacks for a rook on any square
	static Bitboard knight_attack_tables[64];
	static Bitboard king_attack_tables[64];
	static Bitboard pawn_attack_tables[2][64];

	//blockers for a specific square (used esspecialy for legal move generations for checking what blocks a check). e.g. for bishop for [3][30] (which is [d1][g4]) we get a bitboard with squares e2 and f3. blockers are generated unde an assumoption all squares between 2 squares are empty i.e. a requested square is attacked by a specific piece from a speicifc square.
	static Bitboard bishop_square_blockers[64][64];//[square_of_interest][bishop_square]
	static Bitboard rook_square_blockers[64][64];//[square_of_interest][rook_square]

	//masks of pieces which are the actual blockers for each square
	static Bitboard bishop_blockers[64][512];
	static Bitboard rook_blockers[64][4096];

	static uint64_t bishop_magic_numbers[64];
	static uint64_t rook_magic_numbers[64];


	//knights
	static Bitboard white_kingside_castle_knight_attack_mask;
	static Bitboard white_queenside_castle_knight_attack_mask;
	static Bitboard black_kingside_castle_knight_attack_mask;
	static Bitboard black_queenside_castle_knight_attack_mask;

	//pawns
	static Bitboard white_kingside_castle_pawn_attack_mask;
	static Bitboard white_queenside_castle_pawn_attack_mask;
	static Bitboard black_kingside_castle_pawn_attack_mask;
	static Bitboard black_queenside_castle_pawn_attack_mask;

	//king
	static Bitboard white_kingside_castle_king_attack_mask;
	static Bitboard white_queenside_castle_king_attack_mask;
	static Bitboard black_kingside_castle_king_attack_mask;
	static Bitboard black_queenside_castle_king_attack_mask;


	static constexpr size_t max_legal_moves_count = 256;//218 is a theoritiacl limit
	Move legal_moves[MoveGenerator::max_legal_moves_count];

	uint8_t checks;//each bit represents whether specific pices gives a check to the king of the side having a move. order is the same as in PieceType enum (e.g. 3rd bit (check & (1<<2)) coresponds to bishop because Bishop==2)


	static Bitboard generate_rook_attack_table(uint8_t square, uint16_t occupancy_index);
	static Bitboard generate_bishop_attack_table(uint8_t square, uint16_t occupancy_index);

	static void initialize_static_members();

	MoveGenerator(Board* board);
	template<Color color>
	void generate_pseudo_legal_moves();
	template<Color color>
	void filter_pseudo_legal_moves();
	template<Color color>
	bool in_check() const;
};

