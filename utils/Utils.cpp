#include "Utils.h"
#include <cassert>
#include <iostream>
#include <Board.h>
#include <PieceType.h>
#include <MoveType.h>


namespace Utils
{
	void update_collective_bitboards(Board* board)
	{
		//recalculte all_pieces and all_pieces_types fur current board state

		BoardState* current_board_state = &board->positions_stack[board->current_position_idx];
		current_board_state->all_pieces = 0;
		current_board_state->all_pieces_types[White] = 0;
		current_board_state->all_pieces_types[Black] = 0;
		Color color = White;
	enumerator_loop_top:
		for (int piece = 0;piece<=King;++piece)
		{
			current_board_state->all_pieces |= current_board_state->pieces[color][piece];
			current_board_state->all_pieces_types[color] |= current_board_state->pieces[color][piece];
		}
		if (color == White)
		{
			color = Black;
			goto enumerator_loop_top;
		}
	}

	void initialize_board(Board* board)
	{
		//initialize the board to the starting position

		board->current_position_idx = 0;

		board->positions_stack[board->current_position_idx].pieces[0][Pawn] = 0x000000000000FF00;
		board->positions_stack[board->current_position_idx].pieces[0][Knight] = 0x0000000000000042;
		board->positions_stack[board->current_position_idx].pieces[0][Bishop] = 0x0000000000000024;
		board->positions_stack[board->current_position_idx].pieces[0][Rook] = 0x0000000000000081;
		board->positions_stack[board->current_position_idx].pieces[0][Queen] = 0x0000000000000008;
		board->positions_stack[board->current_position_idx].pieces[0][King] = 0x0000000000000010;
		board->positions_stack[board->current_position_idx].pieces[1][Pawn] = 0x00FF000000000000;
		board->positions_stack[board->current_position_idx].pieces[1][Knight] = 0x4200000000000000;
		board->positions_stack[board->current_position_idx].pieces[1][Bishop] = 0x2400000000000000;
		board->positions_stack[board->current_position_idx].pieces[1][Rook] = 0x8100000000000000;
		board->positions_stack[board->current_position_idx].pieces[1][Queen] = 0x0800000000000000;
		board->positions_stack[board->current_position_idx].pieces[1][King] = 0x1000000000000000;


		board->positions_stack[board->current_position_idx].castling_rights = 0b1111;


		board->side_to_move = 0;
		board->positions_stack[board->current_position_idx].en_passant_square = 0;
		board->positions_stack[board->current_position_idx].halfmove_clock = 0;
		
		board->initial_fullmove_count = 1;

		update_collective_bitboards(board);
		board->calculate_hash();
	}

	std::string PieceType_to_string(PieceType piece_type)
	{
		switch (piece_type)
		{
		case Pawn:
			return "Pawn";
		case Knight:
			return "Knight";
		case Bishop:
			return "Bishop";
		case Rook:
			return "Rook";
		case Queen:
			return "Queen";
		case King:
			return "King";
		case Empty:
			return "Empty";
		default:
			return "None";
		}
	}

	char PieceType_to_char(PieceType piece_type, Color color)
	{
		if (color==White)
		{
			switch (piece_type)
			{
			case Pawn:
				return 'P';
			case Knight:
				return 'N';
			case Bishop:
				return 'B';
			case Rook:
				return 'R';
			case Queen:
				return 'Q';
			case King:
				return 'K';
			case Empty:
				return 'E';
			default:
				return '.';
			}
		}
		else
		{
			switch (piece_type)
			{
			case Pawn:
				return 'p';
			case Knight:
				return 'n';
			case Bishop:
				return 'b';
			case Rook:
				return 'r';
			case Queen:
				return 'q';
			case King:
				return 'k';
			case Empty:
				return 'e';
			default:
				return '.';
			}
		}
	}
	
	std::pair<PieceType, Color> char_to_PieceType_and_Color(char c)
	{
	    Color col = (c >= 'A' && c <= 'Z') ? White : Black;

	    switch (c)
	    {
		case 'P': case 'p': return {Pawn, col};
		case 'R': case 'r': return {Rook, col};
		case 'N': case 'n': return {Knight, col};
		case 'B': case 'b': return {Bishop, col};
		case 'Q': case 'q': return {Queen, col};
		case 'K': case 'k': return {King, col};
		default: throw std::runtime_error("char_to_PieceType_and_Color faild to determin piece type");
	    }
	}
	
	void put_piece(BoardState* board_state, PieceType piece, Color color, uint8_t position)
	{
		Bitboard mask = 1ULL << position;
		board_state->pieces[color][piece] |= mask;
		board_state->all_pieces |= mask;
		board_state->all_pieces_types[color] |= mask;
	}

	std::string get_fen(Board* board)
	{
		std::string fen = "";

		Bitboard all_pieces = 0;
		for (int side = 0; side <= 1; ++side)
		{
			for (int piece = Pawn; piece <= King; ++piece)
			{
				all_pieces |= board->positions_stack[board->current_position_idx].pieces[side][piece];
			}
		}

		for (int rank = 7; rank >= 0; rank--)
		{
			int empty_count = 0;
			for (int file = 0; file < 8; file++)
			{
				int square = rank * 8 + file;
				Bitboard mask = 1ULL << square;

				if (!(all_pieces & mask))
				{
					empty_count++;
				}
				else
				{
					if (empty_count > 0)
					{
						fen += std::to_string(empty_count);
						empty_count = 0;
					}

					char piece_char = '?';
					
					bool found = false;
					for (int color = 0;color<=1 && !found;++color)
					{
						for(int piece = Pawn;piece<=King;++piece)
						{
							if (board->positions_stack[board->current_position_idx].pieces[color][piece] & mask) 
							{
								piece_char = PieceType_to_char(static_cast<PieceType>(piece), color == 0 ? White : Black);
								found = true;
								break;
							}
						}
					}
					assert(piece_char!='?');

					fen += piece_char;
				}
			}

			if (empty_count > 0)
			{
				fen += std::to_string(empty_count);
			}

			//separator
			if (rank > 0)
			{
				fen += "/";
			}
		}

		//side to move
		fen += (board->side_to_move == 0) ? " w " : " b ";

		//castling Rights
		std::string castling = "";
		if (board->positions_stack[board->current_position_idx].castling_rights & 1) castling += "K";
		if (board->positions_stack[board->current_position_idx].castling_rights & 2) castling += "Q";
		if (board->positions_stack[board->current_position_idx].castling_rights & 4) castling += "k";
		if (board->positions_stack[board->current_position_idx].castling_rights & 8) castling += "q";

		if (castling.empty()) castling = "-";
		fen += castling + " ";

		//en passant target
		if (board->positions_stack[board->current_position_idx].en_passant_square == 0)
		{
			fen += "-";
		}
		else
		{
			std::string ep = "";
			ep += (char)('a' + (board->positions_stack[board->current_position_idx].en_passant_square % 8));
			ep += (char)('1' + (board->positions_stack[board->current_position_idx].en_passant_square / 8));
			fen += ep;
		}

		//halfmove clock and fullmove number
		fen += " " + std::to_string(board->positions_stack[board->current_position_idx].halfmove_clock) + " " + std::to_string((board->current_position_idx) / 2 + 1);

		return fen;
	}

	void display_board(Board* board, std::ostream& output)
	{
		output << "Side to move: " << (board->side_to_move == 0 ? "White" : "Black") << "\n";
		output << "En passant square: " << std::to_string(board->positions_stack[board->current_position_idx].en_passant_square) << "\n";
		output << "Halfmove clock: " << std::to_string(board->positions_stack[board->current_position_idx].halfmove_clock) << "\n";
		output << "Fullmove clock: " << std::to_string(board->initial_fullmove_count) << "\n";
		output << "Draw by repetition: " << (board->positions_stack[board->current_position_idx].draw_by_repetition ? "Yes" : "No") << "\n";

		for (int rank = 7; rank >= 0; rank--)
		{
			output << rank + 1 << " "; // rank on the left
			for (int file = 0; file < 8; file++)
			{
				int square = rank * 8 + file;
				Bitboard mask = 1ULL << square;
				char piece_char = '.';
				bool found = false;
				for (int color = 0;color<=1 && !found;++color)
				{
					for(int piece = Pawn;piece<=King;++piece)
					{
						if (board->positions_stack[board->current_position_idx].pieces[color][piece] & mask) 
						{
							piece_char = PieceType_to_char(static_cast<PieceType>(piece), color == 0 ? White : Black);
							found = true;
							break;
						}
					}
				}
				
				output << piece_char << " ";
			}
			output << "\n";
		}

		//file letters
		output << "  a b c d e f g h" << "\n";
		output << "fen: " << get_fen(board) << "\n";
		output << "hash: " << std::hex << board->positions_stack[board->current_position_idx].hash << std::dec << "\n";
		output << "\n";
		output << std::flush;
	}

	void display_board_each_piece_and_side_separately(Board* board, std::ostream& output)
	{
		output << "Side to move: " << (board->side_to_move == 0 ? "White" : "Black") << "\n";
		output << "En passant square: " << board->positions_stack[board->current_position_idx].en_passant_square << "\n";
		output << "Halfmove clock: " << board->positions_stack[board->current_position_idx].halfmove_clock << "\n";


		for (uint8_t piece = Pawn; piece <= King; ++piece)
		{
			for (uint8_t side = 0; side <= 1; ++side)
			{
				output << "Piece type: " << PieceType_to_string(static_cast<PieceType>(piece)) << "\n";
				for (int rank = 7; rank >= 0; rank--)
				{
					output << rank + 1 << " "; // rank on the left
					for (int file = 0; file < 8; file++)
					{
						int square = rank * 8 + file;
						Bitboard mask = 1ULL << square;
						char piece_char;
						if (board->positions_stack[board->current_position_idx].pieces[side][piece] & mask)
							piece_char = PieceType_to_char(static_cast<PieceType>(piece), side == 0 ? Black : White);
						else
							piece_char = '.';

						output << piece_char << " ";
					}
					output << "\n";
				}
				// print file letters at bottom
				output << "  a b c d e f g h" << "\n";
				output << "\n";
			}
		}
		display_board(board, output);

	}
	std::string move_to_string(Move move)
	{
		std::string files = "abcdefgh";
		std::string ranks = "12345678";
		std::string move_str = "";
		move_str += files[(move & 0b111111) % 8];
		move_str += ranks[(move & 0b111111) / 8];
		move_str += files[((move >> 6) & 0b111111) % 8];
		move_str += ranks[((move >> 6) & 0b111111) / 8];
		switch (move >> 12)
		{
			case PromotionToQueen:
				move_str += "q";
				break;
			case PromotionToRook:
				move_str += "r";
				break;
			case PromotionToBishop:
				move_str += "b";
				break;
			case PromotionToKnight:
				move_str += "n";
				break;
			default:
				break;
		}
		return move_str;
	}
	PieceType get_piece_on_square(Board* board, uint8_t square)
	{
		Bitboard mask = 1ULL << square;
		for (int piece = Pawn; piece <= King; ++piece)
		{
			if (board->positions_stack[board->current_position_idx].pieces[0][piece] & mask)
				return static_cast<PieceType>(piece);
			if (board->positions_stack[board->current_position_idx].pieces[1][piece] & mask)
				return static_cast<PieceType>(piece);
		}
		return Empty;
	}
	MoveType get_move_type_from_squares(Board* board, uint8_t from_square, uint8_t to_square)
	{
		PieceType piece = get_piece_on_square(board, from_square);
		PieceType target_piece = get_piece_on_square(board, to_square);

		if (piece == Pawn)
		{
			bool is_promotion_rank = (to_square >= 56 || to_square < 8);

			// simple forward move (not capture)
			if (to_square == from_square + 8 || to_square == from_square - 8)
			{
				if (is_promotion_rank)
					return PromotionToQueen;
				return PawnSinglePush;
			}
			if (to_square == from_square + 16 || to_square == from_square - 16)
			{
				if (is_promotion_rank)
					return PromotionToQueen;
				return PawnDoublePush;
			}
			else // capture
			{
				if (is_promotion_rank)
					return PromotionToQueen;
				if (to_square == board->positions_stack[board->current_position_idx].en_passant_square)
					return EnPassant;
				return PawnCapture;
			}
		}
		else if (piece == Knight)
		{
			return KnightMove;
		}
		else if (piece == Bishop)
		{
			return BishopMove;
		}
		else if (piece == Rook)
		{
			return RookMove;
		}
		else if (piece == Queen)
		{
			return QueenMove;
		}
		else if (piece == King)
		{
			if (to_square-from_square==2 || to_square-from_square==-2)
				return Castle;
			return KingMove;
		}

		throw std::runtime_error("error: get_move_type_from_squares failed to determine move type");
	}
	Move string_to_move(Board* board, std::string move)
	{
		std::string files = "abcdefgh";
		std::string ranks = "12345678";
		int from_file = files.find(move[0]);
		int from_rank = ranks.find(move[1]);
		int to_file = files.find(move[2]);
		int to_rank = ranks.find(move[3]);
		int from_square = from_rank * 8 + from_file;
		int to_square = to_rank * 8 + to_file;
		MoveType move_type;
		switch (move[4])
		{
			case 'q':
				move_type = PromotionToQueen;
				break;
			case 'r':
				move_type = PromotionToRook;
				break;
			case 'b':
				move_type = PromotionToBishop;
				break;
			case 'n':
				move_type = PromotionToKnight;
				break;
			default:
				move_type = get_move_type_from_squares(board, from_square, to_square);
		}
		return (to_square << 6) | from_square | (static_cast<Move>(move_type) << 12);
	}
	bool MoveType_is_not_promotion(MoveType move_type)
	{
		return move_type<PromotionToQueen;
	}
	Bitboard occupancy_from_index(uint32_t index, uint64_t mask)
	{
		uint64_t occ = 0;
		uint32_t bit = 0;
		for (int sq = 0; sq < 64; ++sq) {
			if (mask & (1ULL << sq)) {
				if (index & (1U << bit)) occ |= (1ULL << sq);
				++bit;
			}
		}
		return occ;
	}
}
