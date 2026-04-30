#include "Engine.h"
#include "Utils.h"
#include <ostream>
#include <unordered_map>
#include <functional>
#include <thread>
#include <atomic>
#include <syncstream>


Engine engine = Engine();
Board board_copy;
std::mutex engine_mutex;

std::atomic<bool> debug_mode = false;

void do_nothing_command_function(std::vector<std::string> args)
{
	//does nothing, for command which for now don't need any implementation, like e.g. ucinewgame
}

void d_command_function(std::vector<std::string> args)
{
	std::osyncstream out(std::cout);//necessary to pass the out as an lvalue not rvalue
	Utils::display_board(&board_copy, out);
}

void isready_command_function(std::vector<std::string> args)
{
	std::osyncstream out(std::cout);
	out << "readyok" << std::endl;
}

void uci_command_function(std::vector<std::string> args)
{
	std::osyncstream out(std::cout);
	out << "id name ChessEngine2" << std::endl;
	out << "id author Avalfortz" << std::endl;
	out << "uciok" << std::endl;
}

void debug_command_function(std::vector<std::string> args)
{
	std::osyncstream out(std::cout);
	bool value_provided = false;
	for (int i = 0;i<args.size();++i)
	{
		if (args[i]=="on")
		{
			debug_mode.store(true, std::memory_order_relaxed);
			value_provided = true;
			break;
		}
		else if (args[i]=="off")
		{
			debug_mode.store(false, std::memory_order_relaxed);
			value_provided = true;
			break;
		}
	}
	if (!value_provided)
	{
		out << "No value provided for debug command. Use 'debug on' or 'debug off'." << std::endl;
		return;
	}
	out << "Debug mode " << (debug_mode.load(std::memory_order_relaxed) ? "enabled" : "disabled") << std::endl;
}


void go_command_function(std::vector<std::string> args)
{
	std::osyncstream out(std::cout);
	out << std::emit_on_flush;
	if (args.size() >= 2 && args[0]=="perft")
	{
		uint8_t depth = std::stoi(args[1]);
		engine_mutex.lock();
		if (engine.board.side_to_move == White)
		{
			engine.mg.generate_pseudo_legal_moves<White>();
			engine.mg.filter_pseudo_legal_moves<White>();
		}
		else
		{
			engine.mg.generate_pseudo_legal_moves<Black>();
			engine.mg.filter_pseudo_legal_moves<Black>();

		}
		uint64_t count = 0;
		uint64_t perft_result;
		for (int i = 0;i<engine.board.positions_stack[engine.board.current_position_idx].legal_move_next_idx;++i)
		{
			Move move = engine.board.positions_stack[engine.board.current_position_idx].legal_moves[i];
			engine.board.make_move(move);
			if (depth>1)
			{
				if (engine.board.side_to_move == White)
					perft_result = engine.perft<White>(depth - 1);
				else
					perft_result = engine.perft<Black>(depth - 1);
			}
			else
				perft_result = 1;
			count += perft_result;
			out << Utils::move_to_string(move) << ": " << perft_result << std::endl;
			engine.board.unmake_move();
		}
		engine_mutex.unlock();
		out << count << std::endl;
	}
	else
	{
		int16_t depth = -1;
		for (int i = 0;i<args.size();++i)
		{
			if (args[i] == "depth")
			{
				++i;
				if (i < args.size())
				{
					depth = std::stoi(args[i]);
				}
				else
				{
					out << "No depth specified after 'depth'" << std::endl;
					return;
				}
			}
		}
		if (depth == -1)
		{
			out << "No depth specified" << std::endl;
			return;
		}
		std::pair<Move, int16_t> search_result;
		engine_mutex.lock();
		bool debug_mode_local = debug_mode.load(std::memory_order_relaxed);//made to avoid debug_mode changing in the middle of the search causing inconsistent debug output
		if (debug_mode_local)
			engine.nodes_searched = 0;
		if (engine.board.side_to_move == White)
		{
			if (debug_mode_local)
				search_result = engine.search<White, true, true>(depth);
			else
				search_result = engine.search<White, true, false>(depth);
		}
		else
		{
			if (debug_mode_local)
				search_result = engine.search<Black, true, true>(depth);
			else
				search_result = engine.search<Black, true, false>(depth);
		}
		out << "bestmove " << Utils::move_to_string(search_result.first) << std::endl;
		out << "score " << search_result.second << std::endl;
		if (debug_mode_local)
		{
			out << "nodes searched: " << engine.nodes_searched << std::endl;
			engine.nodes_searched = 0;
		}
		engine_mutex.unlock();


	}
}
void position_command_function(std::vector<std::string> args)
{
	engine_mutex.lock();
	for (int i = 0;i<args.size();++i)
	{
		if (args[i]=="fen")
		{
			++i;
			engine.board.load_fen(args[i]);
		}
		else if (args[i]=="startpos")
		{
			Utils::initialize_board(&engine.board);
		}
		else if (args[i]=="moves")
		{
			++i;
			for(;i<args.size();++i)
			{
				Move move_temp = Utils::string_to_move(&engine.board, args[i]);
				engine.board.make_move(move_temp);
			}
			break;
		}
		
	}
	board_copy = engine.board;
	engine_mutex.unlock();
}

std::vector<std::string> tokenize(const std::string& input)
{
	std::vector<std::string> tokens;
	std::string temp = "";

	bool in_quotes = false;

	for (int i = 0; i < input.size(); ++i)
	{
		char c = input[i];
		if (c == '\"')
		{
			in_quotes = !in_quotes;
			continue;
		}
		if (c == ' ' && !in_quotes)
		{
			if (!temp.empty())
			{
				tokens.push_back(temp);
				temp.clear();
			}
		}
		else
		{
			temp += c;
		}
	}
	if (!temp.empty())
	{
		tokens.push_back(temp);
	}

	return tokens;
}

int main()
{
	Utils::initialize_board(&engine.board);
	board_copy = engine.board;
	std::unordered_map<std::string, std::function<void(std::vector<std::string>)>> commands_functions = {
		{"d", d_command_function},
		{"go", go_command_function},
		{"position", position_command_function},
		{"isready", isready_command_function},
		{"uci", uci_command_function},
		{"debug", debug_command_function},
		{"ucinewgame", do_nothing_command_function},
	};
	
	std::osyncstream out(std::cout);
	std::string input_line;
	std::vector<std::string> tokens;
	while (true)
	{
		
		std::getline(std::cin, input_line);
		tokens = tokenize(input_line);
		if (tokens.empty())
			continue;
		if (tokens[0] == "exit" || tokens[0] == "e" || tokens[0] == "quit" || tokens[0] == "q")
		{
			break;//exit
		}
		if (commands_functions.contains(tokens[0]))
		{
			std::thread t(commands_functions[tokens[0]], std::vector<std::string>(tokens.begin()+1, tokens.end()));
			t.detach();
		}
		else
		{
			out << "Unknown command: " << tokens[0] << std::endl;
			continue;
		}
	}

	return 0;
}
