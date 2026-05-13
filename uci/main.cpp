#include <iostream>
#include <functional>
#include <thread>
#include <atomic>
#include <syncstream>
#include <chrono>
#include <cmath>
#include "Engine.h"
#include "TimeManagement.h"
#include "Utils.h"


Engine engine = Engine();
Engine engine_for_go_command = Engine(engine);

std::atomic<bool> go_command_is_running = false;

bool debug_mode = false;


class HandleGoCommandExit
{
public:
	~HandleGoCommandExit()
	{
		go_command_is_running.store(false, std::memory_order_release);
	}
};


void do_nothing_command_function(std::vector<std::string> args)
{
	//does nothing, for command which for now don't need any implementation, like e.g. ucinewgame
}

void d_command_function(std::vector<std::string> args)
{
	Utils::display_board(&engine.board);
}

void isready_command_function(std::vector<std::string> args)
{
	std::cout << "readyok" << std::endl;
}

void uci_command_function(std::vector<std::string> args)
{
	std::cout << "id name ChessEngine2" << std::endl;
	std::cout << "id author Avalfortz" << std::endl;
	std::cout << "uciok" << std::endl;
}

void debug_command_function(std::vector<std::string> args)
{
	bool value_provided = false;
	for (int i = 0;i<args.size();++i)
	{
		if (args[i]=="on")
		{
			debug_mode = true;
			value_provided = true;
			break;
		}
		else if (args[i]=="off")
		{
			debug_mode = false;
			value_provided = true;
			break;
		}
	}
	if (!value_provided)
	{
		std::cout << "No value provided for debug command. Use 'debug on' or 'debug off'." << std::endl;
		return;
	}
	std::cout << "Debug mode " << (debug_mode ? "enabled" : "disabled") << std::endl;
}


void go_command_function(std::vector<std::string> args)
{
	HandleGoCommandExit handle_go_command_exit;
	std::osyncstream out(std::cout);
	out << std::emit_on_flush;
	if (args.size() >= 2 && args[0]=="perft")
	{
		uint8_t depth;
		try
		{
			depth = std::stoi(args[1]);
		}
		catch (...)
		{
			return;
		}
		
		if (engine_for_go_command.board.side_to_move == White)
		{
			engine_for_go_command.mg.generate_pseudo_legal_moves<White>();
			engine_for_go_command.mg.filter_pseudo_legal_moves<White>();
		}
		else
		{
			engine_for_go_command.mg.generate_pseudo_legal_moves<Black>();
			engine_for_go_command.mg.filter_pseudo_legal_moves<Black>();

		}
		uint64_t count = 0;
		uint64_t perft_result;
		for (int i = 0;i<engine_for_go_command.board.positions_stack[engine.board.current_position_idx].legal_moves_length;++i)
		{
			Move move = engine_for_go_command.board.positions_stack[engine.board.current_position_idx].legal_moves[i];
			engine_for_go_command.board.make_move(move);
			if (depth>1)
			{
				if (engine_for_go_command.board.side_to_move == White)
					perft_result = engine_for_go_command.perft<White>(depth - 1);
				else
					perft_result = engine_for_go_command.perft<Black>(depth - 1);
			}
			else
				perft_result = 1;
			count += perft_result;
			out << Utils::move_to_string(move) << ": " << perft_result << std::endl;
			engine_for_go_command.board.unmake_move();
		}
		out << count << std::endl;
	}
	else
	{
		int16_t depth_max = -1;
		uint64_t wtime = -1, btime = -1, winc = -1, binc = -1;
		for (int i = 0;i<args.size();++i)
		{
			if (args[i] == "depth")
			{
				++i;
				if (i < args.size())
				{
					try
					{
						depth_max = std::stoi(args[i]);
					}
					catch (...)
					{
						return;
					}
					
				}
				else
				{
					out << "No depth specified after 'depth'" << std::endl;
					return;
				}
			}
			else if (args[i] == "wtime")
			{
				++i;
				if (i < args.size())
				{
					try
					{
						wtime = std::stoull(args[i]);
					}
					catch (...)
					{
						return;
					}
					
				}
				else
				{
					out << "No wtime specified after 'wtime'" << std::endl;
					return;
				}
			}
			else if (args[i] == "btime")
			{
				++i;
				if (i < args.size())
				{
					try
					{
						btime = std::stoull(args[i]);
					}
					catch (...)
					{
						return;
					}
					
				}
				else
				{
					out << "No btime specified after 'btime'" << std::endl;
					return;
				}
			}
			else if (args[i] == "winc")
			{
				++i;
				if (i < args.size())
				{
					try
					{
						winc = std::stoull(args[i]);
					}
					catch (...)
					{
						return;
					}
					
				}
				else
				{
					out << "No winc specified after 'winc'" << std::endl;
					return;
				}
			}
			else if (args[i] == "binc")
			{
				++i;
				if (i < args.size())
				{
					try
					{
						binc = std::stoull(args[i]);
					}
					catch (...)
					{
						return;
					}
					
				}
				else
				{
					out << "No binc specified after 'binc'" << std::endl;
					return;
				}
			}
		}
		if (depth_max == -1 && (wtime == -1 || btime == -1))//neither depth nor time control specified
		{
			return;
		}
		std::pair<Move, int16_t> search_result;
		long time_passed = -1;
		if (depth_max != -1)
		{
			auto start_time = std::chrono::high_resolution_clock::now();
			for (uint8_t depth = 1;depth<=depth_max;++depth)
			{
				engine_for_go_command.nodes_searched = 0;
				if (engine_for_go_command.board.side_to_move == White)
					search_result = engine_for_go_command.search<White, true, true>(depth);
				else
					search_result = engine_for_go_command.search<Black, true, true>(depth);
				time_passed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
				out << "info depth " << static_cast<int>(depth) << " score cp " << search_result.second << " nodes " << engine_for_go_command.nodes_searched << " nps " << (time_passed > 0 ? engine.nodes_searched * 1'000'000 / time_passed : 0) << " time " << static_cast<int>(std::round((static_cast<float>(time_passed)/1000.0))) << std::endl;
			}
		}
		else
		{
			if (winc == -1)
				winc = 0;
			if (binc == -1)
				binc = 0;
			uint64_t time_to_think = get_time_to_think_in_ms(&engine_for_go_command, wtime, btime, winc, binc);
			auto start_time = std::chrono::high_resolution_clock::now();
			float effective_branching_factor_estimate = 1;
			long previous_time_passed = -1;
			for (uint8_t depth = 1;true;++depth)
			{
				engine_for_go_command.nodes_searched = 0;
				if (engine_for_go_command.board.side_to_move == White)
					search_result = engine_for_go_command.search<White, true, true>(depth);
				else
					search_result = engine_for_go_command.search<Black, true, true>(depth);
				previous_time_passed = time_passed;
				time_passed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
				if (previous_time_passed != -1)
				{
					effective_branching_factor_estimate = time_passed/static_cast<float>(previous_time_passed);
				}
				out << "info depth " << static_cast<int>(depth) << " score cp " << search_result.second << " nodes " << engine_for_go_command.nodes_searched << " nps " << (time_passed > 0 ? engine.nodes_searched * 1'000'000 / time_passed : 0) << " time " << static_cast<int>(std::round((static_cast<float>(time_passed)/1000.0))) << std::endl;
				long estimated_time_for_next_depth = time_passed * effective_branching_factor_estimate;
				if (estimated_time_for_next_depth * 1.2 > time_to_think * 1000)//*1000 is neccessary we measure time in microseconds but time_to_think is in milliseconds
					break;
			}
		}
		
		out << "bestmove " << Utils::move_to_string(search_result.first) << std::endl;


	}
}
void position_command_function(std::vector<std::string> args)
{
	for (int i = 0;i<args.size();++i)
	{
		if (args[i]=="fen")
		{
			++i;
			std::string fen;
			if (i+5<args.size())//posibly fen given without quotes
			{
				if (args[i+1]=="b" || args[i+1]=="w")//we assume fen is given without quotes since next token matches fen color indication
				{
					fen = args[i] + " " + args[i+1] + " " + args[i+2] + " " + args[i+3] + " " + args[i+4] + " " + args[i+5];
					i+=5;
				}
				else//fen in quotes
				{
					fen = args[i];
				}
			}
			else//fen in quotes
			{
				fen = args[i];
			}
			//convert all whitespace characters in fen to space
			for (char& c : fen)
			{
				if (std::isspace(c))
					c = ' ';
			}
			engine.board.load_fen(fen);
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
}


void fen_command_function(std::vector<std::string> args)
{
	std::cout << std::emit_on_flush;
	std::cout << Utils::get_fen(&engine.board) << std::endl;
}

void hash_command_function(std::vector<std::string> args)
{
	std::cout << std::emit_on_flush;
	std::cout << std::hex << engine.board.positions_stack[engine.board.current_position_idx].hash << std::dec << std::endl;
}

void move_command_function(std::vector<std::string> args)
{
	if (args.empty())
		return;
	Move move_temp = Utils::string_to_move(&engine.board, args[0]);
	engine.board.make_move(move_temp);
}

void unmove_command_function(std::vector<std::string> args)
{
	engine.board.unmake_move();
}

void eval_command_function(std::vector<std::string> args)
{
	std::cout << std::emit_on_flush;
	int16_t eval;
	if (engine.board.side_to_move == White)
		eval = engine.se.evaluate<White>();
	else
		eval = engine.se.evaluate<Black>();
	std::cout << "Static evaluation: " << eval << std::endl;
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
		if (std::isspace(c) && !in_quotes)
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
	std::unordered_map<std::string, std::function<void(std::vector<std::string>)>> commands_functions = {
		{"d", d_command_function},
		{"go", go_command_function},
		{"position", position_command_function},
		{"isready", isready_command_function},
		{"uci", uci_command_function},
		{"debug", debug_command_function},
		{"ucinewgame", do_nothing_command_function},
		//non uci commands
		{"fen", fen_command_function},
		{"hash", hash_command_function},
		{"move", move_command_function},
		{"unmove", unmove_command_function},
		{"eval", eval_command_function},
	};
	
	std::unordered_map<std::thread::id, std::thread> threads;
	std::string input_line;
	std::vector<std::string> tokens;
	std::thread t;
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
		if (commands_functions.contains(tokens[0]) && tokens[0]!="go")
		{
			commands_functions[tokens[0]](std::vector<std::string>(tokens.begin()+1, tokens.end()));
		}
		else if (tokens[0] == "go")
		{
			if (!go_command_is_running.load(std::memory_order_acquire))
			{
				if (t.joinable())
					t.join();
				engine_for_go_command = Engine(engine);
				go_command_is_running.store(true, std::memory_order_release);
				t = std::thread(commands_functions[tokens[0]], std::vector<std::string>(tokens.begin()+1, tokens.end()));
			}
		}
		else
		{
			std::cout << "Unknown command: " << tokens[0] << std::endl;
			continue;
		}
	}
	t.join();

	
	return 0;
}
