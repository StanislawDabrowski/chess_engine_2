#include "MoveGenerator.h"
#include "Board.h"
#include <bit>
#include <cstdint>
#include <cstring>
#include "MoveType.h"
#include "Utils.h"
#include "PieceType.h"
#include "Color.h"
#include <cassert>



typedef uint16_t SimpleMove;

MoveGenerator::MoveGenerator(Board* board)
	: board(board)
{
	generate_relevant_blockers_and_square_blockers();

	
}

void MoveGenerator::generate_relevant_blockers_and_square_blockers()
{
	int rook_directions[] = { 8, -8, 1, -1 };
	int bishop_directions[] = { 7, -7, 9, -9 };
	Bitboard rook_mask;
	Bitboard bishop_mask;
	int current_square;
	int cs_mod8;
	int temp;
	for (int square = 0; square < 64; ++square)
	{
		rook_mask = 0;
		// Rook relevant blockers
		for (int direction : rook_directions)
		{
			current_square = square;
			while (true)
			{
				temp = current_square + direction;
				if (temp > 63 || temp < 0) break; // stop if out of bounds
				cs_mod8 = current_square % 8;
				if (cs_mod8 == 0 && direction == -1) break; // stop if we hit left edge going left
				if (cs_mod8 == 7 && direction == 1) break; // stop if we hit right edge going right
				if (current_square > 55 && direction == 8) break; // stop if we hit top edge going up
				if (current_square < 8 && direction == -8) break; // stop if we hit bottom edge going down
				if (current_square !=square)
					rook_mask |= (1ULL << current_square);
				current_square += direction;
			}
		}
		rook_relevant_blockers[square] = rook_mask;

		// Rook square bolckers
		for (int j = 0;j<64;++j)
		{
			bool hit_the_square = false;
			for (int direction : rook_directions)
			{
				rook_mask = 0;//reset the mask so we can have a clean mask for blockers of the square
				current_square = square;
				while (true)
				{
					if (current_square == j) //stop if we hit the square
					{
						hit_the_square = true;
						break;
					}
					temp = current_square + direction;
					if (temp > 63 || temp < 0) break; // stop if out of bounds
					
					cs_mod8 = current_square % 8;
					if (cs_mod8 == 0 && direction == -1) break; // stop if we hit left edge going left
					if (cs_mod8 == 7 && direction == 1) break; // stop if we hit right edge going right
					if (current_square > 55 && direction == 8) break; // stop if we hit top edge going up
					if (current_square < 8 && direction == -8) break; // stop if we hit bottom edge going down
					
					if (current_square !=square)
						rook_mask |= (1ULL << current_square);
					current_square += direction;
				}
				if (hit_the_square)
				{
					rook_square_blockers[j][square] = rook_mask;
					break;
				}
			}
		}

		// Bishop relevant blockers
		bishop_mask = 0;
		for (int direction : bishop_directions)
		{

			current_square = square;
			while (true)
			{
				temp = current_square + direction;
				if (temp > 63 || temp < 0) break; // stop if out of bounds
				cs_mod8 = current_square % 8;
				if (cs_mod8 == 0 && (direction == -9 || direction == 7)) break; // stop if we hit left edge going left
				if (cs_mod8 == 7 && (direction == 9 || direction == -7)) break; // stop if we hit right edge going right
				if (current_square > 55 && (direction == 9 || direction == 7)) break; // stop if we hit top edge going up
				if (current_square < 8 && (direction == -7 || direction == -9)) break; // stop if we hit bottom edge going down
				if (current_square != square)
					bishop_mask |= (1ULL << current_square);
				current_square += direction;
			}
		}
		bishop_relevant_blockers[square] = bishop_mask;

		// Bishop square bolckers
		for (int j = 0;j<64;++j)
		{
			bool hit_the_square = false;
			for (int direction : bishop_directions)
			{
				bishop_mask = 0;//reset the mask so we can have a clean mask for blockers of the square

				current_square = square;
				while (true)
				{
					if (current_square == j) //stop if we hit the square
					{
						hit_the_square = true;
						break;
					}
					temp = current_square + direction;
					if (temp > 63 || temp < 0) break; // stop if out of bounds
					
					cs_mod8 = current_square % 8;
					if (cs_mod8 == 0 && (direction == -9 || direction == 7)) break; // stop if we hit left edge going left
					if (cs_mod8 == 7 && (direction == 9 || direction == -7)) break; // stop if we hit right edge going right
					if (current_square > 55 && (direction == 9 || direction == 7)) break; // stop if we hit top edge going up
					if (current_square < 8 && (direction == -7 || direction == -9)) break; // stop if we hit bottom edge going down
					
					if (current_square != square)
						bishop_mask |= (1ULL << current_square);
					current_square += direction;
				}
				if (hit_the_square)
				{
					bishop_square_blockers[j][square] = bishop_mask;
					break;
				}
			}
		}
	}
}

Bitboard MoveGenerator::generate_rook_attack_table(uint8_t square, uint16_t occupancy_index)
{
	constexpr int rook_directions[] = { 8, -8, 1, -1 };

	Bitboard occupancy = Utils::occupancy_from_index(occupancy_index, rook_relevant_blockers[square]);

	uint8_t current_square;
	uint8_t cs_mod8;
	Bitboard rook_mask = 0;
	for (int direction : rook_directions)
	{
		current_square = square;
		while (true)
		{
			cs_mod8 = current_square % 8;
			if (current_square!=square)
				rook_mask |= (1ULL << current_square);
			if (occupancy & (1ULL << current_square)) break; // stop if we hit a blocker
			if (cs_mod8 == 0 && direction == -1) break; // stop if we hit left edge going left
			if (cs_mod8 == 7 && direction == 1) break; // stop if we hit right edge going right
			if (current_square > 55 && direction == 8) break; // stop if we hit top edge going up
			if (current_square < 8 && direction == -8) break; // stop if we hit bottom edge going down

			current_square += direction;
			if (current_square > 63 || current_square < 0) break; // stop if out of bounds
		}
	}
	return rook_mask;
}

Bitboard MoveGenerator::generate_bishop_attack_table(uint8_t square, uint16_t occupancy_index)
{
	constexpr int bishop_directions[] = { 7, -7, 9, -9 };

	Bitboard occupancy = Utils::occupancy_from_index(occupancy_index, bishop_relevant_blockers[square]);

	uint8_t current_square;
	uint8_t cs_mod8;

	Bitboard bishop_mask = 0;
	for (int direction : bishop_directions)
	{
		current_square = square;// + direction;
		while (true)
		{
			cs_mod8 = current_square % 8;
			if (current_square < 0 || current_square >= 64) break; // stop if out of bounds
			if (current_square != square)
				bishop_mask |= (1ULL << current_square);
			if (occupancy & (1ULL << current_square)) break; // stop if we hit a blocker
			if (cs_mod8 == 0 && (direction == -9 || direction == 7)) break; // stop if we hit left edge going left
			if (cs_mod8 == 7 && (direction == 9 || direction == -7)) break; // stop if we hit right edge going right

			current_square += direction;
		}
	}
	return bishop_mask;
}

void MoveGenerator::initialize_attack_tables()
{
	constexpr int rook_directions[] = { 8, -8, 1, -1 };
	constexpr int bishop_directions[] = { 7, -7, 9, -9 };
	Bitboard rook_mask;
	Bitboard bishop_mask;
	Bitboard knight_mask;
	Bitboard king_mask;
	Bitboard blockers_bishop_mask;
	Bitboard blockers_rook_mask;
	Bitboard occupancy;
	int current_square;
	int cs_mod8;
	memset(bishop_attack_tables, 0, sizeof(bishop_attack_tables));
	memset(rook_attack_tables, 0, sizeof(rook_attack_tables));
	for (int square = 0; square < 64; ++square)
	{
		//rook attacks
		for (int i = 0; i < 1u << std::popcount(rook_relevant_blockers[square]); ++i)
		{
			occupancy = Utils::occupancy_from_index(i, rook_relevant_blockers[square]);

			rook_mask = 0;
			blockers_rook_mask = 0;
			for (int direction : rook_directions)
			{
				current_square = square;// + direction;
				while (true)
				{
					cs_mod8 = current_square % 8;
					if (current_square!=square)
						rook_mask |= (1ULL << current_square);
					if (occupancy & (1ULL << current_square))
					{
						blockers_rook_mask |= (1ULL << current_square);//add a bolcker
						break; // stop if we hit a blocker
					}
					if (cs_mod8 == 0 && direction == -1) break; // stop if we hit left edge going left
					if (cs_mod8 == 7 && direction == 1) break; // stop if we hit right edge going right
					if (current_square > 55 && direction == 8) break; // stop if we hit top edge going up
					if (current_square < 8 && direction == -8) break; // stop if we hit bottom edge going down

					current_square += direction;
					if (current_square > 63 || current_square < 0) break; // stop if out of bounds
				}
			}

			
			//get with magic numbers index to save the attack table to
			Bitboard relevant_blockers = occupancy & rook_relevant_blockers[square];
			size_t index = (relevant_blockers * rook_magic_numbers[square]) >> (64 - std::popcount(rook_relevant_blockers[square]));

			if (rook_attack_tables[square][index] != 0 && rook_attack_tables[square][index] != rook_mask) {
				std::cout << "Rook collision: square=" << square
					<< " index=" << index
					<< " existing=" << rook_attack_tables[square][index]
					<< " new=" << rook_mask
					<< " occ=" << occupancy
					<< " popcount=" << std::popcount(rook_relevant_blockers[square])
					<< std::endl;
			}
			rook_attack_tables[square][index] = rook_mask;
			rook_blockers[square][index] = blockers_rook_mask;

		}

		//bishop attacks
		for (int i = 0; i < 1u << std::popcount(bishop_relevant_blockers[square]); ++i)
		{
			occupancy = Utils::occupancy_from_index(i, bishop_relevant_blockers[square]);
			bishop_mask = 0;
			blockers_bishop_mask = 0;
			for (int direction : bishop_directions)
			{
				int current_square = square;// + direction;
				while (true)
				{
					cs_mod8 = current_square % 8;
					if (current_square < 0 || current_square >= 64) break; // stop if out of bounds
					if (current_square != square)
						bishop_mask |= (1ULL << current_square);
					if (occupancy & (1ULL << current_square))
					{
						blockers_bishop_mask |= (1ULL << current_square);//add a bolcker
						break; // stop if we hit a blocker
					}
					if (cs_mod8 == 0 && (direction == -9 || direction == 7)) break; // stop if we hit left edge going left
					if (cs_mod8 == 7 && (direction == 9 || direction == -7)) break; // stop if we hit right edge going right

					current_square += direction;
				}
			}
			//get with magic numbers index to save the attack table to
			
			Bitboard relevant_blockers = occupancy & bishop_relevant_blockers[square];
			size_t index = (relevant_blockers * bishop_magic_numbers[square]) >> (64 - std::popcount(bishop_relevant_blockers[square]));
			

			if (bishop_attack_tables[square][index] != 0 && bishop_attack_tables[square][index] != bishop_mask) {
				std::cout << "Bishop collision: square=" << square
					<< " index=" << index
					<< " existing=" << bishop_attack_tables[square][index]
					<< " new=" << bishop_mask
					<< " occ=" << occupancy
					<< " popcount=" << std::popcount(bishop_relevant_blockers[square])
					<< std::endl;
			}
			bishop_attack_tables[square][index] = bishop_mask;
			bishop_blockers[square][index] = blockers_bishop_mask;


		}
		//knight attacks
		knight_mask = 0;
		int rank = square / 8;
		int file = square % 8;
		int knight_moves[8][2] = { {2,1}, {1,2}, {-1,2}, {-2,1}, {-2,-1}, {-1,-2}, {1,-2}, {2,-1} };
		for (auto& move : knight_moves)
		{
			int new_rank = rank + move[0];
			int new_file = file + move[1];
			if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8)
			{
				knight_mask |= (1ULL << (new_rank * 8 + new_file));
			}
		}
		knight_attack_tables[square] = knight_mask;

		//king attacks
		king_mask = 0;
		int king_moves[8][2] = { {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1} };
		for (auto& move : king_moves)
		{
			int new_rank = rank + move[0];
			int new_file = file + move[1];
			if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8)
			{
				king_mask |= (1ULL << (new_rank * 8 + new_file));
			}
		}
		king_attack_tables[square] = king_mask;
	}
	//pawns
	for (int sq = 0; sq < 64; ++sq)
	{
		pawn_attack_tables[0][sq] = 0;
		pawn_attack_tables[1][sq] = 0;
		int file = sq % 8;
		//white
		if (sq <= 48)
		{
			if (file > 0)
				pawn_attack_tables[0][sq] |= (1ULL << (sq + 7));
			if (file < 7)
				pawn_attack_tables[0][sq] |= (1ULL << (sq + 9));
		}
		//black
		if (sq >= 15)
		{
			if (file > 0)
				pawn_attack_tables[1][sq] |= (1ULL << (sq - 9));
			if (file < 7)
				pawn_attack_tables[1][sq] |= (1ULL << (sq - 7));
		}
	}


	//castles
	//knights
	
	//white kingside
	Bitboard knight_from_e1 = knight_attack_tables[4];
	Bitboard knight_from_f1 = knight_attack_tables[5];
	Bitboard knight_from_g1 = knight_attack_tables[6];
	white_kingside_castle_knight_attack_mask = knight_from_e1 | knight_from_f1 | knight_from_g1;

	//white queenside
	Bitboard knight_from_d1 = knight_attack_tables[3];
	Bitboard knight_from_c1 = knight_attack_tables[2];
	white_queenside_castle_knight_attack_mask = knight_from_e1 | knight_from_d1 | knight_from_c1;

	//black kingside
	Bitboard knight_from_e8 = knight_attack_tables[60];
	Bitboard knight_from_f8 = knight_attack_tables[61];
	Bitboard knight_from_g8 = knight_attack_tables[62];
	black_kingside_castle_knight_attack_mask = knight_from_e8 | knight_from_f8 | knight_from_g8;

	//black queenside
	Bitboard knight_from_d8 = knight_attack_tables[59];
	Bitboard knight_from_c8 = knight_attack_tables[58];
	black_queenside_castle_knight_attack_mask = knight_from_e8 | knight_from_d8 | knight_from_c8;


	//pawns
	white_kingside_castle_pawn_attack_mask = 0b11111ULL << 11;
	white_queenside_castle_pawn_attack_mask = 0b11111ULL << 9;
	black_kingside_castle_pawn_attack_mask = 0b11111ULL << 51;
	black_queenside_castle_pawn_attack_mask = 0b11111ULL << 49;

	//king
	white_kingside_castle_king_attack_mask = king_attack_tables[4] | king_attack_tables[5] | king_attack_tables[6];
	white_queenside_castle_king_attack_mask = king_attack_tables[4] | king_attack_tables[3] | king_attack_tables[2];
	black_kingside_castle_king_attack_mask = king_attack_tables[60] | king_attack_tables[61] | king_attack_tables[62];
	black_queenside_castle_king_attack_mask = king_attack_tables[60] | king_attack_tables[59] | king_attack_tables[58];
}

template<uint8_t castle_type>//0 - white kingside, 1 - white queenside, 2 - black kingside, 3 - black queenside
bool inline MoveGenerator::can_castle()
{
	Color side_to_move = (castle_type==0 || castle_type==1) ? White : Black;
	size_t opp = side_to_move == White ? Black : White;

	Bitboard castle_knight_attack_mask = castle_type == 0 ? white_kingside_castle_knight_attack_mask : (castle_type == 1 ? white_queenside_castle_knight_attack_mask : (castle_type == 2 ? black_kingside_castle_knight_attack_mask : (black_queenside_castle_knight_attack_mask)));
	Bitboard castle_pawn_attack_mask = castle_type == 0 ? white_kingside_castle_pawn_attack_mask : (castle_type == 1 ? white_queenside_castle_pawn_attack_mask : (castle_type == 2 ? black_kingside_castle_pawn_attack_mask : (black_queenside_castle_pawn_attack_mask)));
	Bitboard castle_king_attack_mask = castle_type == 0 ? white_kingside_castle_king_attack_mask : (castle_type == 1 ? white_queenside_castle_king_attack_mask : (castle_type == 2 ? black_kingside_castle_king_attack_mask : (black_queenside_castle_king_attack_mask)));

	if (castle_knight_attack_mask & board->positions_stack[board->current_position_idx].pieces[opp][Knight]) return false;
	if (castle_pawn_attack_mask & board->positions_stack[board->current_position_idx].pieces[opp][Pawn]) return false;
	if (castle_king_attack_mask & board->positions_stack[board->current_position_idx].pieces[opp][King]) return false;


	Bitboard bishop_potential_attacks;
	Bitboard rook_potential_attacks;

	Bitboard squares_indexes[3];
	squares_indexes[0] = (side_to_move == White) ? 4 : 60;
	squares_indexes[1] = (castle_type == 0) ? 5 : ((castle_type == 1) ? 3 : ((castle_type == 2) ? 61 : 59));
	squares_indexes[2] = (castle_type == 0) ? 6 : ((castle_type == 1) ? 2 : ((castle_type == 2) ? 62 : 58));

	size_t index;

	for (size_t i = 0;i<3;++i)
	{
		index = ((board->positions_stack[board->current_position_idx].all_pieces & bishop_relevant_blockers[squares_indexes[i]])*bishop_magic_numbers[squares_indexes[i]]) >> (64 - std::popcount(bishop_relevant_blockers[squares_indexes[i]]));
		if (bishop_attack_tables[squares_indexes[i]][index] & (board->positions_stack[board->current_position_idx].pieces[opp][Bishop] | board->positions_stack[board->current_position_idx].pieces[opp][Queen]))
			return false;
		index = ((board->positions_stack[board->current_position_idx].all_pieces & rook_relevant_blockers[squares_indexes[i]])*rook_magic_numbers[squares_indexes[i]]) >> (64 - std::popcount(rook_relevant_blockers[squares_indexes[i]]));
		if (rook_attack_tables[squares_indexes[i]][index] & (board->positions_stack[board->current_position_idx].pieces[opp][Rook] | board->positions_stack[board->current_position_idx].pieces[opp][Queen]))
			return false;
	}
	return true;
}

template<Color color>
void MoveGenerator::generate_pseudo_legal_moves()
{
	pseudo_legal_moves_length = 0;
	Color opp = (color == White) ? Black : White;
	
	Bitboard piece_copy;
	size_t to;
	size_t from;
	Bitboard attacks;
	Bitboard relevant_blockers;
	Bitboard relevant_blockers_2;
	uint64_t index;
	uint64_t index_2;


	//PAWN
	piece_copy = board->positions_stack[board->current_position_idx].pieces[color][Pawn];
	Bitboard single_push = (color == White ? ((piece_copy & Utils::RANK_7_NEGATION) << 8) : ((piece_copy & Utils::RANK_2_NEGATION) >> 8)) & ~board->positions_stack[board->current_position_idx].all_pieces;
	Bitboard double_push = (color == White ? ((single_push & Utils::RANK_3) << 8) : ((single_push & Utils::RANK_6) >> 8)) & ~board->positions_stack[board->current_position_idx].all_pieces;

	//left/right captures are a captures to the specific side always relative to white
	Bitboard left_captures = (color == White ? ((piece_copy & Utils::FILE_A_NEGATION & Utils::RANK_7_NEGATION) << 7) : ((piece_copy & Utils::FILE_A_NEGATION & Utils::RANK_2_NEGATION) >> 9)) & board->positions_stack[board->current_position_idx].all_pieces_types[opp];
	Bitboard right_captures = (color == White ? ((piece_copy & Utils::FILE_H_NEGATION & Utils::RANK_7_NEGATION) << 9) : ((piece_copy & Utils::FILE_H_NEGATION & Utils::RANK_2_NEGATION) >> 7)) & board->positions_stack[board->current_position_idx].all_pieces_types[opp];

	Bitboard push_promotion = (color == White ? ((piece_copy & Utils::RANK_7) << 8) : ((piece_copy & Utils::RANK_2) >> 8)) & ~board->positions_stack[board->current_position_idx].all_pieces;
	Bitboard left_capture_promotion = (color == White ? ((piece_copy & Utils::FILE_A_NEGATION & Utils::RANK_7) << 7) : ((piece_copy & Utils::FILE_A_NEGATION & Utils::RANK_2) >> 9)) & board->positions_stack[board->current_position_idx].all_pieces_types[opp];
	Bitboard right_capture_promotion = (color == White ? ((piece_copy & Utils::FILE_H_NEGATION & Utils::RANK_7) << 9) : ((piece_copy & Utils::FILE_H_NEGATION & Utils::RANK_2) >> 7)) & board->positions_stack[board->current_position_idx].all_pieces_types[opp];

	
	while (single_push)
	{
		to = std::countr_zero(single_push);
		pseudo_legal_moves[pseudo_legal_moves_length++] = ((color == White) ? (to - 8) : (to + 8)) | (to << 6) | (PawnSinglePush << 12);
		single_push &= single_push - 1;
	}
	while (double_push)
	{
		to = std::countr_zero(double_push);
		pseudo_legal_moves[pseudo_legal_moves_length++] = ((color == White) ? (to - 16) : (to + 16)) | (to << 6) | (PawnDoublePush << 12);
		double_push &= double_push - 1;
	}
	while (left_captures)
	{
		to = std::countr_zero(left_captures);
		pseudo_legal_moves[pseudo_legal_moves_length++] =  ((color == White) ? (to - 7) : (to + 9)) | (to << 6) | (PawnCapture << 12);
		left_captures &= left_captures - 1;
	}
	while (right_captures)
	{
		to = std::countr_zero(right_captures);
		pseudo_legal_moves[pseudo_legal_moves_length++] =  ((color == White) ? (to - 9) : (to + 7)) | (to << 6) | (PawnCapture << 12);
		right_captures &= right_captures - 1;
	}
	while (left_capture_promotion)
	{
		to = std::countr_zero(left_capture_promotion);
		pseudo_legal_moves[pseudo_legal_moves_length++] =  ((color == White) ? (to - 7) : (to + 9)) | (to << 6) | (PromotionToQueen << 12);
		left_capture_promotion &= left_capture_promotion - 1;
	}
	while (right_capture_promotion)
	{
		to = std::countr_zero(right_capture_promotion);
		pseudo_legal_moves[pseudo_legal_moves_length++] =  ((color == White) ? (to - 9) : (to + 7)) | (to << 6) | (PromotionToQueen << 12);
		right_capture_promotion &= right_capture_promotion - 1;
	}
	while (push_promotion)
	{
		to = std::countr_zero(push_promotion);
		pseudo_legal_moves[pseudo_legal_moves_length++] =  ((color == White) ? (to - 8) : (to + 8)) | (to << 6) | (PromotionToQueen << 12);
		push_promotion &= push_promotion - 1;
	}
	//en passant
	if (board->positions_stack[board->current_position_idx].en_passant_square != 0)
	{
		Bitboard en_passant_mask = 1ULL << board->positions_stack[board->current_position_idx].en_passant_square;
		Bitboard pawns_left = (color == White) ? (piece_copy & Utils::FILE_H_NEGATION) << 9 : (piece_copy & Utils::FILE_H_NEGATION) >> 7;
		Bitboard pawns_right = (color == White) ? (piece_copy & Utils::FILE_A_NEGATION) << 7 : (piece_copy & Utils::FILE_A_NEGATION) >> 9;
		if (pawns_left & en_passant_mask)
		{
			pseudo_legal_moves[pseudo_legal_moves_length++] = (board->positions_stack[board->current_position_idx].en_passant_square + (color == White ? -9 : 7)) | (board->positions_stack[board->current_position_idx].en_passant_square << 6) | (EnPassant << 12);
		}
		if (pawns_right & en_passant_mask)
		{
			pseudo_legal_moves[pseudo_legal_moves_length++] = (board->positions_stack[board->current_position_idx].en_passant_square + (color == White ? -7 : 9)) | (board->positions_stack[board->current_position_idx].en_passant_square << 6) | (EnPassant << 12);
		}
	}
	
	//KNIGHTS
	piece_copy = board->positions_stack[board->current_position_idx].pieces[color][Knight];
	while (piece_copy)
	{
		from = std::countr_zero(piece_copy);
		attacks = knight_attack_tables[from] & ~board->positions_stack[board->current_position_idx].all_pieces_types[color];
		while (attacks)
		{
			to = std::countr_zero(attacks);
			pseudo_legal_moves[pseudo_legal_moves_length++] = from | (to << 6) | (KnightMove << 12);
			attacks &= attacks - 1;
		}
		piece_copy &= piece_copy - 1;
	}

	//BISHOPS
	piece_copy = board->positions_stack[board->current_position_idx].pieces[color][Bishop];
	while (piece_copy)
	{
		from = std::countr_zero(piece_copy);
		relevant_blockers = board->positions_stack[board->current_position_idx].all_pieces & bishop_relevant_blockers[from];
		index = (relevant_blockers * bishop_magic_numbers[from]) >> (64 - std::popcount(bishop_relevant_blockers[from]));
		attacks = bishop_attack_tables[from][index] & ~board->positions_stack[board->current_position_idx].all_pieces_types[color];
		while (attacks)
		{
			to = std::countr_zero(attacks);
			pseudo_legal_moves[pseudo_legal_moves_length++] = from | (to << 6) | (BishopMove << 12);
			attacks &= attacks - 1;
		}
		piece_copy &= piece_copy - 1;
	}
	//ROOKS
	piece_copy = board->positions_stack[board->current_position_idx].pieces[color][Rook];
	while (piece_copy)
	{
		from = std::countr_zero(piece_copy);
		relevant_blockers = board->positions_stack[board->current_position_idx].all_pieces & rook_relevant_blockers[from];
		index = (relevant_blockers * rook_magic_numbers[from]) >> (64 - std::popcount(rook_relevant_blockers[from]));
		attacks = rook_attack_tables[from][index] & ~board->positions_stack[board->current_position_idx].all_pieces_types[color];
		while (attacks)
		{
			to = std::countr_zero(attacks);
			pseudo_legal_moves[pseudo_legal_moves_length++] = from | (to << 6) | (RookMove << 12);
			attacks &= attacks - 1;
		}
		piece_copy &= piece_copy - 1;
	}
	//QUEENS
	piece_copy = board->positions_stack[board->current_position_idx].pieces[color][Queen];
	while (piece_copy)
	{
		from = std::countr_zero(piece_copy);
		relevant_blockers = board->positions_stack[board->current_position_idx].all_pieces & rook_relevant_blockers[from];
		relevant_blockers_2 = board->positions_stack[board->current_position_idx].all_pieces & bishop_relevant_blockers[from];
		index = (relevant_blockers * rook_magic_numbers[from]) >> (64 - std::popcount(rook_relevant_blockers[from]));
		index_2 = (relevant_blockers_2 * bishop_magic_numbers[from]) >> (64 - std::popcount(bishop_relevant_blockers[from]));
		attacks = (rook_attack_tables[from][index] | bishop_attack_tables[from][index_2]) & ~board->positions_stack[board->current_position_idx].all_pieces_types[color];
		while (attacks)
		{
			to = std::countr_zero(attacks);
			pseudo_legal_moves[pseudo_legal_moves_length++] = from | (to << 6) | (QueenMove << 12);
			attacks &= attacks - 1;
		}
		piece_copy &= piece_copy - 1;
	}
	
	//KING
	from = std::countr_zero(board->positions_stack[board->current_position_idx].pieces[color][King]);
	attacks = king_attack_tables[from] & ~board->positions_stack[board->current_position_idx].all_pieces_types[color];
	while (attacks)
	{
		to = std::countr_zero(attacks);
		pseudo_legal_moves[pseudo_legal_moves_length++] = from | (to << 6) | (KingMove << 12);
		attacks &= attacks - 1;
	}
	
	if (color == White)
	{
		if (board->positions_stack[board->current_position_idx].castling_rights & 0b0001)//white king side
		{
			if ((board->positions_stack[board->current_position_idx].all_pieces & Utils::WHITE_KINGSIDE_CASTLE_MASK) == 0 && can_castle<0>())
				pseudo_legal_moves[pseudo_legal_moves_length++] = Utils::WHITE_KINGSIDE_CASTLE_FROM_TO_MASK | (Castle << 12);
		}
		if (board->positions_stack[board->current_position_idx].castling_rights & 0b0010)//white queen side
		{
			if ((board->positions_stack[board->current_position_idx].all_pieces & Utils::WHITE_QUEENSIDE_CASTLE_MASK) == 0 && can_castle<1>())
				pseudo_legal_moves[pseudo_legal_moves_length++] = Utils::WHITE_QUEENSIDE_CASTLE_FROM_TO_MASK | (Castle << 12);
		}
	}
	else
	{
		if (board->positions_stack[board->current_position_idx].castling_rights & 0b0100)//black king side
		{
			if ((board->positions_stack[board->current_position_idx].all_pieces & Utils::BLACK_KINGSIDE_CASTLE_MASK) == 0 && can_castle<2>())
				pseudo_legal_moves[pseudo_legal_moves_length++] = Utils::BLACK_KINGSIDE_CASTLE_FROM_TO_MASK | (Castle << 12);
		}
		if (board->positions_stack[board->current_position_idx].castling_rights & 0b1000)//white queen side
		{
			if ((board->positions_stack[board->current_position_idx].all_pieces & Utils::BLACK_QUEENSIDE_CASTLE_MASK) == 0 && can_castle<3>())
				pseudo_legal_moves[pseudo_legal_moves_length++] = Utils::BLACK_QUEENSIDE_CASTLE_FROM_TO_MASK | (Castle << 12);
		}
	}
}

void MoveGenerator::add_legal_move(const Move move)
{
	board->positions_stack[board->current_position_idx].legal_moves[board->positions_stack[board->current_position_idx].legal_moves_length++] = move;
	if (move >> 12 == PromotionToQueen)
	{
		//add underpromotions
		//underpromotion enums are right after promotions to queen so just adding 1, 2 or 3 to queen promotions in enough
		board->positions_stack[board->current_position_idx].legal_moves[board->positions_stack[board->current_position_idx].legal_moves_length++] = move + (1 << 12);
		board->positions_stack[board->current_position_idx].legal_moves[board->positions_stack[board->current_position_idx].legal_moves_length++] = move + (2 << 12);
		board->positions_stack[board->current_position_idx].legal_moves[board->positions_stack[board->current_position_idx].legal_moves_length++] = move + (3 << 12);
	}
}

template<Color color>
void MoveGenerator::filter_pseudo_legal_moves()
{
	BoardState* current_board_state = &board->positions_stack[board->current_position_idx];


	current_board_state->legal_moves_length = 0;

	size_t opp = color ^ 1;
	size_t king_square = std::countr_zero(current_board_state->pieces[color][King]);
	size_t to_square;
	size_t from_square;
	size_t from_mask;
	size_t to_mask;
	size_t from_to_mask;
	size_t index;

	size_t bishop_magic_index = ((current_board_state->all_pieces & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> (64 - std::popcount(bishop_relevant_blockers[king_square]));
 	Bitboard bishop_potential_attacks = bishop_attack_tables[king_square][bishop_magic_index];
	Bitboard blockers_bishop = bishop_blockers[king_square][bishop_magic_index];

	size_t rook_magic_index = ((current_board_state->all_pieces & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> (64 - std::popcount(rook_relevant_blockers[king_square]));
	Bitboard rook_potential_attacks = rook_attack_tables[king_square][rook_magic_index];
	Bitboard blockers_rook = rook_blockers[king_square][rook_magic_index];

	Bitboard queen_potential_attacks = bishop_potential_attacks | rook_potential_attacks;



	
	Bitboard pawns = current_board_state->pieces[color][Pawn];

	Bitboard checking_pawn = (pawn_attack_tables[color][king_square] & current_board_state->pieces[opp][Pawn]);
	Bitboard checking_knight = (knight_attack_tables[king_square] & current_board_state->pieces[opp][Knight]);
	Bitboard checking_bishop = (bishop_potential_attacks & current_board_state->pieces[opp][Bishop]);
	Bitboard checking_rooks = (rook_potential_attacks & current_board_state->pieces[opp][Rook]);
	Bitboard checking_queens = (queen_potential_attacks & current_board_state->pieces[opp][Queen]);
	checks = checking_pawn ? 1 << Pawn : 0;
	checks |= checking_knight ? 1 << Knight : 0;
	checks |= checking_bishop ? 1 << Bishop : 0;
	checks |= checking_rooks ? (std::popcount(checking_rooks) == 1 ? 1  << Rook : 3 << Rook) : 0;
	checks |= checking_queens ? (std::popcount(checking_queens) == 1 ? 1 << (Queen + 1) : 3 << (Queen + 1)) : 0;//Queen+1 is neccessery since weight of checking_rooks can be 2 so it can take 2 bits in variable "checks"
	//in any legal position the side opposit to the side having the move is not in check.
	//this means the only way for making a double check is to make a discover check, checking with the moved peice at the same time.
	//only queens and rooks are pieces which can by itself give doulbe check (2 rooks or 2 queens).
	//2 queens is trivial.
	//2 rooks is possible when a pawn promots to rook taking a piece at the same time moving to the line at which king is and at the same another rook needs to be at the same rand as the pwan was before promotions and if king is on the intersection of the, it's double check, example of a position when double check with only rooks is posible can be this position 3q4/3kP2R/8/8/8/8/8/7K w - - 0 1
	//2 knights or 2 pawns is trivialy impossible since these are not sliding pieces (no discover check is possible)
	//2 bishops is impossible because, similiary to rooks, the only possibility would be promotion, but it's not possible in case of bishops. If a pawn promotes to a bishop by a push forward it changes color of the square so if it did discover some square for some bishop that bishop is on other color that the color of the square the pawn moved to, hence it only could be a capture prmotion. When a panw moves reveling some squares to a bishop the only squares that are now attacked by the bishop that were not are behind a pawn, so only if a promoted biswhop dcan attack one of these squares, can it be double check. If the bishop is on the 8th rank, on the same as bishop after promotions there is only 1 square attacked by both pieces, the square the pawn was on before, so it can't be double check. If the bishop is on the other rank, the only revealed squares are squares on the 8th rank, hece squares which were already attacked by the pawn, so king could not be there either, so it can't be double promotions either. Hence double check with bishops is impossible.
	
	
	
	//no check:
	//all moves with non king not absolutely pinned pieces and king moves to non attacked squares
	//single check:
	//non sliding-pieces - king moves and captures of the checking piece
	//sliding pieces - the same + interposing
	//double check:
	//only king moves to non attacked squares

	uint8_t number_of_checks = std::popcount(checks);

	auto handle_king_move_legality_check = [&](const size_t i)
	{
		to_square = (pseudo_legal_moves[i] >> 6) & 0b111111;
		from_square = pseudo_legal_moves[i] & 0b111111;
		from_mask = 1ULL << from_square;
		if (knight_attack_tables[to_square] & current_board_state->pieces[opp][Knight]) return;
		if (pawn_attack_tables[color][to_square] & current_board_state->pieces[opp][Pawn]) return;
		if (king_attack_tables[to_square] & current_board_state->pieces[opp][King]) return;
		index = (((current_board_state->all_pieces ^ from_mask) & bishop_relevant_blockers[to_square]) * bishop_magic_numbers[to_square]) >> (64 - std::popcount(bishop_relevant_blockers[to_square]));
		bishop_potential_attacks = bishop_attack_tables[to_square][index];
		if (bishop_potential_attacks & (current_board_state->pieces[opp][Bishop] | current_board_state->pieces[opp][Queen])) return;
		index = (((current_board_state->all_pieces ^ from_mask) & rook_relevant_blockers[to_square]) * rook_magic_numbers[to_square]) >> (64 - std::popcount(rook_relevant_blockers[to_square]));
		rook_potential_attacks = rook_attack_tables[to_square][index];
		if (rook_potential_attacks & (current_board_state->pieces[opp][Rook] | current_board_state->pieces[opp][Queen])) return;
		add_legal_move(pseudo_legal_moves[i]);
	};
	auto check_if_moving_piece_is_absolutely_pinned = [&](const size_t i)
	{
		to_mask = 1ULL << ((pseudo_legal_moves[i] >> 6) & 0b111111);
		from_mask = 1ULL << (pseudo_legal_moves[i] & 0b111111);
		if (bishop_relevant_blockers[king_square] & from_mask)
		{
			index = ((((current_board_state->all_pieces | to_mask) ^ from_mask) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> (64 - std::popcount(bishop_relevant_blockers[king_square]));
			bishop_potential_attacks = bishop_attack_tables[king_square][index];
			if (bishop_potential_attacks & ((current_board_state->pieces[opp][Bishop] | current_board_state->pieces[opp][Queen]) & ~to_mask)) return true;//also does an additional check for capturing the pining piece so if the move captures the pinning piece even thought the piece is pinned functnio will return true
		}
		if (rook_relevant_blockers[king_square] & from_mask)
		{
			index = ((((current_board_state->all_pieces | to_mask) ^ from_mask) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> (64 - std::popcount(rook_relevant_blockers[king_square]));
			rook_potential_attacks = rook_attack_tables[king_square][index];
			if (rook_potential_attacks & ((current_board_state->pieces[opp][Rook] | current_board_state->pieces[opp][Queen]) & ~to_mask)) return true;//also does an additional check for capturing the pining piece so if the move captures the pinning piece even thought the piece is pinned functnio will return true
		}
		return false;

	};
	auto check_if_en_passant_capture_reveals_king = [&](const size_t i)
	{
		from_square = pseudo_legal_moves[i] & 0b111111;
		to_square = (pseudo_legal_moves[i] >> 6) & 0b111111;
		from_mask = (1ULL << from_square);
		from_to_mask = from_mask | (1ULL << to_square);
		Bitboard en_passant_mask = 1ULL << (to_square + (color == White ? -8 : 8));
		if (bishop_relevant_blockers[king_square] & from_mask)//no need to check en passant mask as well because situation in which pawn which can be taken wiht en passant is the only piece screening the king form an attacking piece is impossible
		{
			index = (((current_board_state->all_pieces ^ (from_to_mask | en_passant_mask)) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> (64 - std::popcount(bishop_relevant_blockers[king_square]));
			bishop_potential_attacks = bishop_attack_tables[king_square][index];
			if (bishop_potential_attacks & (current_board_state->pieces[opp][Bishop] | current_board_state->pieces[opp][Queen])) return true;
		}
		if (rook_relevant_blockers[king_square] & from_mask)
		{
			index = (((current_board_state->all_pieces ^ (from_to_mask | en_passant_mask)) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> (64 - std::popcount(rook_relevant_blockers[king_square]));
			rook_potential_attacks = rook_attack_tables[king_square][index];
			if (rook_potential_attacks & (current_board_state->pieces[opp][Rook] | current_board_state->pieces[opp][Queen])) return true;
		}
		return false;
	};

	if (number_of_checks==0)
	{
		for(int i = 0;i<pseudo_legal_moves_length;++i)
		{
			if (i==22)
				std::cout<<"";
			switch (pseudo_legal_moves[i] >> 12)
			{
			case Castle://castle legality is already checked in pseudo legal move generation
				add_legal_move(pseudo_legal_moves[i]);
				break;
			case KingMove://check if the square king moves to is not attacked
				handle_king_move_legality_check(i);
				break;
			case EnPassant:
			{
				if (check_if_en_passant_capture_reveals_king(i)) continue;
				add_legal_move(pseudo_legal_moves[i]);
				break;
			}
			default:
				if (check_if_moving_piece_is_absolutely_pinned(i)) continue;
				add_legal_move(pseudo_legal_moves[i]);
				break;
			}
		}
	}
	else if (number_of_checks==1)
	{
		if ((checks & (1 << Knight)) || (checks & (1 << Pawn)))//non sliding piece check, only king moves and captures are legal
		{
			Bitboard checking_piece_index = std::countr_zero(checking_pawn | checking_knight);//at least one bit of (checking_pawn | checking_knight) is 1 (not 0)
			for (int i = 0;i<pseudo_legal_moves_length;++i)
			{
				switch (pseudo_legal_moves[i] >> 12)
				{
					case KingMove:
						handle_king_move_legality_check(i);
						break;
					case EnPassant:
						//only illigal if absolutely pinned since if king is checked only once and there since an en passant capture possible the last move have had to be double push and since the check is either pawn or knight check it has to be the check by the double moved pawn and the en passant captures that pawn
						if (check_if_en_passant_capture_reveals_king(i)) continue;
						add_legal_move(pseudo_legal_moves[i]);
						break;
					default:
						//only captures of the checking piece with not absolutely pinned piece
						if (check_if_moving_piece_is_absolutely_pinned(i)) continue;
						if (((pseudo_legal_moves[i] >> 6) & 0b111111) == checking_piece_index)
						{
							add_legal_move(pseudo_legal_moves[i]);
						}
						break;
				}
			}
		}
		else//sliding piece check
		{
			Bitboard legal_moveTo_squares;
			if (checks & (1 << Bishop))
				legal_moveTo_squares = bishop_square_blockers[king_square][std::countr_zero(checking_bishop)] | checking_bishop;
			else if (bishop_potential_attacks & current_board_state->pieces[opp][Queen])
				legal_moveTo_squares = bishop_square_blockers[king_square][std::countr_zero(checking_queens)] | checking_queens;
			else if (checks & (1 << Rook))
				legal_moveTo_squares = rook_square_blockers[king_square][std::countr_zero(checking_rooks)] | checking_rooks;
			else
				legal_moveTo_squares = rook_square_blockers[king_square][std::countr_zero(checking_queens)] | checking_queens;
			

			for (int i = 0;i<pseudo_legal_moves_length;++i)
			{
				if ((pseudo_legal_moves[i] >> 12) != KingMove)
				{
					if ((1ULL << ((pseudo_legal_moves[i] >> 6) & 0b111111)) & legal_moveTo_squares)//move interposes check or takes the checking piece
					{
						if (!check_if_moving_piece_is_absolutely_pinned(i))
						{
							add_legal_move(pseudo_legal_moves[i]);
						}
					}
				}
				else
				{
					handle_king_move_legality_check(i);
				}
			}
		}
	}
	else//double check
	{
		for (int i = 0;i<pseudo_legal_moves_length;++i)
		{ if ((pseudo_legal_moves[i] >> 12) == KingMove) { handle_king_move_legality_check(i);
			}
		}
	}

}


template<Color color>
bool MoveGenerator::in_check() const
{
	Color opp = color == White ? Black : White;
	size_t king_square = std::countr_zero(board->positions_stack[board->current_position_idx].pieces[color][King]);
	Bitboard opponent_pawns = board->positions_stack[board->current_position_idx].pieces[opp][Pawn];
	if (opponent_pawns & pawn_attack_tables[static_cast<int>(color)][king_square])
		return true;
	Bitboard opponent_knights = board->positions_stack[board->current_position_idx].pieces[opp][Knight];
	if (opponent_knights & knight_attack_tables[king_square])
		return true;
	Bitboard opponent_bishops = board->positions_stack[board->current_position_idx].pieces[opp][Bishop];
	Bitboard opponent_rooks = board->positions_stack[board->current_position_idx].pieces[opp][Rook];
	Bitboard opponent_queens = board->positions_stack[board->current_position_idx].pieces[opp][Queen];
	uint64_t index = ((board->positions_stack[board->current_position_idx].all_pieces & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> (64 - std::popcount(bishop_relevant_blockers[king_square]));
	Bitboard bishop_attacks = bishop_attack_tables[king_square][index];
	if (bishop_attacks & (opponent_bishops | opponent_queens))
		return true;
	index = ((board->positions_stack[board->current_position_idx].all_pieces & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> (64 - std::popcount(rook_relevant_blockers[king_square]));
	Bitboard rook_attacks = rook_attack_tables[king_square][index];
	if (rook_attacks & (opponent_rooks | opponent_queens))
		return true;
	return false;
}


template void MoveGenerator::generate_pseudo_legal_moves<White>();
template void MoveGenerator::generate_pseudo_legal_moves<Black>();
template void MoveGenerator::filter_pseudo_legal_moves<White>();
template void MoveGenerator::filter_pseudo_legal_moves<Black>();

template bool MoveGenerator::in_check<White>() const;
template bool MoveGenerator::in_check<Black>() const;
