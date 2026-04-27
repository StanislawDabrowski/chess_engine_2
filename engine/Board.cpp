#include "Color.h"
#include "Defs.h"
#include <cctype>
#include <cstdint>
#include <cstring>
#include "Board.h"
#include "MoveType.h"
#include "PieceType.h"
#include "BoardRecord.h"
#include "Utils.h"

Board::Board()
{
	initialize_castling_mask();
}

void Board::initialize_castling_mask()
{
	for (uint8_t square = 0; square < 64; ++square)
	{
		castling_mask[square] = 0b1111;
		if (square == 0)
		{
			castling_mask[square] &= 0b1101;//white queenside
		}
		else if (square == 7)
		{
			castling_mask[square] &= 0b1110;//white kingside
		}
		else if (square == 56)
		{
			castling_mask[square] &= 0b0111;//black queenside
		}
		else if (square == 63)
		{
			castling_mask[square] &= 0b1011;//black kingside
		}
		else if (square == 4)
		{
			castling_mask[square] &= 0b1100;//white castle
		}
		else if (square == 60)
		{
			castling_mask[square] &= 0b0011;//black castle
		}
	}
}

void Board::load_fen(std::string fen)
{
	current_position_idx = 0;
	std::memset(positions_stack[current_position_idx].pieces, 0, sizeof(positions_stack[0].pieces));
	positions_stack[current_position_idx].castling_rights = 0;
	positions_stack[current_position_idx].all_pieces = 0;
	positions_stack[current_position_idx].all_pieces_types[White] = 0;
	positions_stack[current_position_idx].all_pieces_types[Black] = 0;
	initial_fullmove_count = 0;
	std::pair<PieceType, Color> temp;
	uint8_t position_idx = 0;
	size_t i = 0;
	for (;i < fen.length();++i)//pieces
	{
		if (fen[i]==' ') break;
		if (fen[i]=='/') continue;
		if (std::isdigit(fen[i]))
		{
			position_idx+=fen[i]-'0';
			continue;
		}
		temp = Utils::char_to_PieceType_and_Color(fen[i]);
		uint8_t converted_position_index = ((63 - position_idx) & ~0b111) + (7-((63 - position_idx) % 8));
		Utils::put_piece(&positions_stack[current_position_idx], temp.first, temp.second, converted_position_index);
		++position_idx;
	}
	++i;
	if (fen[i]=='w') side_to_move = White;//color
	else side_to_move = Black;
	i+=2;
	if (fen[i]!='-')//castles
	{
		bool flag = true;
		for(;i<fen.length() && flag;++i)
		{
			switch (fen[i])
			{
			case 'K':
				positions_stack[current_position_idx].castling_rights |= 0b0001;
				break;
			case 'Q':
				positions_stack[current_position_idx].castling_rights |= 0b0010;
				break;
			case 'k':
				positions_stack[current_position_idx].castling_rights |= 0b0100;
				break;
			case 'q':
				positions_stack[current_position_idx].castling_rights |= 0b1000;
				break;
			case ' ':
				flag = false;
				break;
			}
		}
	}
	else
		i+=2;
	if (fen[i]!='-')//en passant
	{
		positions_stack[current_position_idx].en_passant_square = fen[i] - 'a';
		positions_stack[current_position_idx].en_passant_square += (fen[++i] - '1')*8;
		i+=2;
	}
	else
		i+=2;
	positions_stack[current_position_idx].halfmove_clock = 0;
	while (fen[i]!=' ')//halfmove count
	{
		positions_stack[current_position_idx].halfmove_clock *= 10;
		positions_stack[current_position_idx].halfmove_clock += fen[i++]-'0';
	}
	++i;
	while (i<fen.length())//fullmove count
	{
		initial_fullmove_count *= 10;
		initial_fullmove_count += fen[i++]-'0';
	}
}

void Board::make_move(Move move)
{
	uint8_t from_square = move & 0b111111;
	uint8_t to_square = (move >> 6) & 0b111111;
	MoveType move_type = static_cast<MoveType>(move >> 12);

	Bitboard from_mask = 1ULL << from_square;
	Bitboard to_mask = 1ULL << to_square;
	Bitboard from_to_mask = from_mask | to_mask;

	uint8_t opp = side_to_move ^ 1;

	std::memcpy(&positions_stack[current_position_idx+1], &positions_stack[current_position_idx], sizeof(BoardState));
	++current_position_idx;

	positions_stack[current_position_idx].en_passant_square = 0;
	positions_stack[current_position_idx].all_pieces_types[side_to_move] ^= from_to_mask;
	
	uint8_t piece_moved = Utils::MoveType_to_PieceType[move_type];
	if (Utils::MoveType_is_not_promotion(move_type))
		positions_stack[current_position_idx].pieces[side_to_move][piece_moved] ^= from_to_mask;
	else
	{
		positions_stack[current_position_idx].pieces[side_to_move][piece_moved] ^= from_mask;
		positions_stack[current_position_idx].pieces[side_to_move][Utils::MoveType_to_promotion_piece[move_type]] ^= to_mask;
	}

	positions_stack[current_position_idx].castling_rights &= castling_mask[from_square];

	uint8_t captured_piece;
	for (captured_piece = Pawn; captured_piece < King; ++captured_piece)
	{
		if (positions_stack[current_position_idx].pieces[opp][captured_piece] & to_mask)
		{
			positions_stack[current_position_idx].pieces[opp][captured_piece] ^= to_mask;
			positions_stack[current_position_idx].all_pieces ^= from_mask;
			positions_stack[current_position_idx].all_pieces_types[opp] ^= to_mask;
			positions_stack[current_position_idx].castling_rights &= castling_mask[to_square];
			break;
		}
	}
	if (move_type==EnPassant)
	{
		Bitboard en_passant_mask = side_to_move == White ? (to_mask >> 8) : (to_mask << 8);
		positions_stack[current_position_idx].pieces[opp][Pawn] ^= en_passant_mask;
		positions_stack[current_position_idx].all_pieces ^= en_passant_mask;
		positions_stack[current_position_idx].all_pieces_types[opp] ^= en_passant_mask;
	}
	if (captured_piece==King)
		positions_stack[current_position_idx].all_pieces ^= from_to_mask;
	if (move_type==Castle)
	{
		Bitboard rook_from_to_mask;
		if (to_square>from_square)
		{
			rook_from_to_mask = from_to_mask << 1;
		}
		else
		{
			rook_from_to_mask = (from_mask >> 1) | (to_mask >> 2);
		}
		positions_stack[current_position_idx].pieces[side_to_move][Rook] ^= rook_from_to_mask;
		positions_stack[current_position_idx].all_pieces ^= rook_from_to_mask;
		positions_stack[current_position_idx].all_pieces_types[side_to_move] ^= rook_from_to_mask;
	}
	if (move_type==PawnDoublePush)
	{
		positions_stack[current_position_idx].en_passant_square = (from_square+to_square)>>1;
	}
	

	side_to_move ^= 1;
}

void Board::unmake_move()
{
	--current_position_idx;
	side_to_move ^= 1;
}
