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
		engine_for_go_command.stop_search.store(false, std::memory_order_release);
	}
};


void update_engine_for_go_command()
{
	engine_for_go_command.board = engine.board;
}


static constexpr uint64_t min_hash_size_in_mb = 1;
static constexpr uint64_t max_hash_size_in_mb = 18446744073709551615ULL;//2^64-1
static constexpr uint64_t default_hash_size_in_mb = Engine::DEFAULT_TT_SIZE * sizeof(TTEntry) / (1024 * 1024);

void set_hash(std::string val)
{
	//we don't call update_TT_size on object engine, we only call it on engine_for_go_command so the engine object doesn't wase memory
	try
	{
		uint64_t hash_size_in_mb = std::stoull(val);
		//check if within bounds
		if (hash_size_in_mb < min_hash_size_in_mb || hash_size_in_mb > max_hash_size_in_mb)
		{
			return;
		}
		uint64_t hash_size_in_bytes = hash_size_in_mb * 1024 * 1024;
		engine_for_go_command.TT_size = hash_size_in_bytes / sizeof(TTEntry);
		engine_for_go_command.update_TT_size();
	}
	catch (...)
	{
		return;
	}
}

static std::unordered_map<std::string, std::function<void(std::string)>> option_setters = {
	{"Hash", set_hash},
};

void ucinewgame_command_function(std::vector<std::string> args)
{
	//clear TT
	for (size_t i = 0;i < engine_for_go_command.TT_size;++i)
	{
		engine_for_go_command.TT[i].best_move = 0;//set to illegal moves so it's never used
		engine_for_go_command.TT[i].hash = 0;//hash which is less likely to occur than a hash which actually occured in some game
		engine_for_go_command.TT[i].eval = Engine::MIN_EVAL;//min eval so with eval type being lower bound leads to entry being never used for eval
		engine_for_go_command.TT[i].eval_type = TTEvalType::LowerBound;
		engine_for_go_command.TT[i].depth = 0;//set to 0 to minimise usage
	}
}

void setoption_command_function(std::vector<std::string> args)
{
	std::string name;
	std::string value;
	for (int i = 0;i<args.size();++i)
	{
		if (args[i]=="name")
		{
			++i;
			if (i<args.size())
				name = args[i];
			else
			{
				std::cout << "No name specified after 'name'" << std::endl;
				return;
			}
		}
		else if (args[i]=="value")
		{
			++i;
			if (i<args.size())
				value = args[i];
			else
			{
				std::cout << "No value specified after 'value'" << std::endl;
				return;
			}
		}
	}
	if (option_setters.contains(name))
	{
		option_setters[name](value);
	}
	else
	{
		std::cout << "Unknown option name: " << name << std::endl;
	}
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
	std::cout << "option name Hash type spin default " << default_hash_size_in_mb << " min " << min_hash_size_in_mb << " max " << max_hash_size_in_mb << std::endl;
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


std::string get_score_string(int16_t score)
{
	std::string score_string;
	if (score > Engine::MATE_THRESHOLD || score < -Engine::MATE_THRESHOLD)
	{
		if (score > 0)
		{
			int mate_in_ply = Engine::MAX_EVAL - score;
			return "mate " + std::to_string((mate_in_ply+1)/2);
		}
		else
		{
			int mate_in_ply = Engine::MIN_EVAL - score;
			return "mate " + std::to_string((mate_in_ply)/2);//no need to adjust with +1 or -1 since if we are getting mated we make the first move but the opponent makes the last move so the ply is even
		}
	}
	else
	{
		return "cp " + std::to_string(score);
	}
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
		for (int i = 0;i<engine_for_go_command.board.positions_stack[engine_for_go_command.board.current_position_idx].legal_moves_length;++i)
		{
			Move move = engine_for_go_command.board.positions_stack[engine_for_go_command.board.current_position_idx].legal_moves[i];
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
		if (depth_max == -1)
		{
			if (engine.board.side_to_move == White)
			{
				if (wtime == -1)
				{
					return;
				}
			}
			else
			{
				if (btime == -1)
				{
					return;
				}
			}
		}
		std::pair<Move, int16_t> search_result = std::make_pair(0, 0);
		std::pair<Move, int16_t> search_result_temp;
		long time_passed = -1;
		if (depth_max != -1)
		{
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
				if (engine_for_go_command.board.positions_stack[engine_for_go_command.board.current_position_idx].legal_moves_length == 1)
			{
				search_result.first = engine_for_go_command.board.positions_stack[engine_for_go_command.board.current_position_idx].legal_moves[0];
				if (engine_for_go_command.board.side_to_move == White)
					search_result.second = engine_for_go_command.se.evaluate<White>();
				else
					search_result.second = engine_for_go_command.se.evaluate<Black>();
				out << "info depth 1 score cp " << search_result.second << " nodes 1 time 0" << std::endl;
				out << "bestmove " << Utils::move_to_string(search_result.first) << std::endl;
				return;
			}
			auto start_time = std::chrono::high_resolution_clock::now();
			engine_for_go_command.search_time_hard_bound = start_time + std::chrono::years(128);//we add 128 years as an infinit so the search never stops because of hard bound
			for (uint8_t depth = 1;depth<=depth_max && std::abs((engine.board.side_to_move == White ? Engine::MAX_EVAL : Engine::MIN_EVAL)-search_result.second)>=depth;++depth)
			{
				engine_for_go_command.normal_search_nodes_searched = 0;
				engine_for_go_command.quiescence_search_nodes_searched = 0;
				engine_for_go_command.TT_hits = 0;
				engine_for_go_command.TT_writes = 0;
				if (engine_for_go_command.board.side_to_move == White)
					search_result_temp = engine_for_go_command.search<White, true, false, true>(depth);
				else
					search_result_temp = engine_for_go_command.search<Black, true, false, true>(depth);
				if (engine_for_go_command.stop_search.load(std::memory_order_relaxed))
					break;
				else
					search_result = search_result_temp;

				time_passed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
				std::string score_string = get_score_string(search_result.second);
				out << "info depth " << static_cast<int>(depth);
				out << " score ";
				out << score_string;
				out << " nodes " << engine_for_go_command.normal_search_nodes_searched+engine_for_go_command.quiescence_search_nodes_searched;
				out << " nps " << (time_passed > 0 ? (engine_for_go_command.normal_search_nodes_searched+engine_for_go_command.quiescence_search_nodes_searched) * 1'000'000 / time_passed : 0);
				out << " tthits " << engine_for_go_command.TT_hits;
				out << " ttwrites " << engine_for_go_command.TT_writes;
				out << " time " << static_cast<int>(std::round((static_cast<float>(time_passed)/1000.0))) << std::endl;
			}
		}
		else
		{
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
			if (engine_for_go_command.board.positions_stack[engine_for_go_command.board.current_position_idx].legal_moves_length == 1)
			{
				search_result.first = engine_for_go_command.board.positions_stack[engine_for_go_command.board.current_position_idx].legal_moves[0];
				if (engine_for_go_command.board.side_to_move == White)
					search_result.second = engine_for_go_command.se.evaluate<White>();
				else
					search_result.second = engine_for_go_command.se.evaluate<Black>();
				out << "info depth 1 " << "score cp " << search_result.second << " nodes 1 time 0" << std::endl;
				out << "bestmove " << Utils::move_to_string(search_result.first) << std::endl;
				return;
			}
			if (winc == -1)
				winc = 0;
			if (binc == -1)
				binc = 0;
			std::pair<uint64_t, uint64_t> time_to_think = get_time_to_think_in_ms(&engine_for_go_command, wtime, btime, winc, binc);
			auto start_time = std::chrono::high_resolution_clock::now();
			engine_for_go_command.search_time_hard_bound = start_time + std::chrono::milliseconds(static_cast<long>(time_to_think.second));
			std::chrono::time_point<std::chrono::high_resolution_clock> search_time_soft_bound = start_time + std::chrono::milliseconds(static_cast<long>(time_to_think.first));
			long previous_time_passed = -1;
			for (uint8_t depth = 1;std::abs(search_result.second)<=Engine::MATE_THRESHOLD;++depth)
			{
				engine_for_go_command.normal_search_nodes_searched = 0;
				engine_for_go_command.quiescence_search_nodes_searched = 0;
				engine_for_go_command.TT_hits = 0;
				engine_for_go_command.TT_writes = 0;
				if (engine_for_go_command.board.side_to_move == White)
					search_result_temp = engine_for_go_command.search<White, true, false, true>(depth);
				else
					search_result_temp = engine_for_go_command.search<Black, true, false, true>(depth);
				if (engine_for_go_command.stop_search.load(std::memory_order_relaxed) || std::chrono::high_resolution_clock::now() >= engine_for_go_command.search_time_hard_bound)
				{
					if (search_result.first == 0)
						search_result = search_result_temp;
					break;
				}
				else
					search_result = search_result_temp;
				previous_time_passed = time_passed;
				time_passed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
				
				std::string score_string = get_score_string(search_result.second);
				out << "info depth " << static_cast<int>(depth);
				out << " score " << score_string;
				out << " nodes " << engine_for_go_command.normal_search_nodes_searched+engine_for_go_command.quiescence_search_nodes_searched;
				out << " nps " << (time_passed > 0 ? (engine_for_go_command.normal_search_nodes_searched+engine_for_go_command.quiescence_search_nodes_searched) * 1'000'000 / time_passed : 0);
				out << " tthits " << engine_for_go_command.TT_hits;
				out << " ttwrites " << engine_for_go_command.TT_writes;
				out << " time " << static_cast<int>(std::round((static_cast<float>(time_passed)/1000.0))) << std::endl;
				if (time_passed >= time_to_think.first / 4 * 1000)
				{
					break;
				}
			}
		}
		
		out << "bestmove " << Utils::move_to_string(search_result.first) << std::endl;


	}
}

void stop_command_function(std::vector<std::string> args)
{
	engine_for_go_command.stop_search.store(true, std::memory_order_release);
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
	MoveGenerator::initialize_static_members();
	Utils::initialize_board(&engine.board);
	std::unordered_map<std::string, std::function<void(std::vector<std::string>)>> commands_functions = {
		{"d", d_command_function},
		{"go", go_command_function},
		{"stop", stop_command_function},
		{"position", position_command_function},
		{"isready", isready_command_function},
		{"uci", uci_command_function},
		{"debug", debug_command_function},
		{"ucinewgame", ucinewgame_command_function},
		{"setoption", setoption_command_function},
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
				update_engine_for_go_command();
				go_command_is_running.store(true, std::memory_order_seq_cst);
				t = std::thread(commands_functions[tokens[0]], std::vector<std::string>(tokens.begin()+1, tokens.end()));
			}
		}
		else
		{
			std::cout << "Unknown command: " << tokens[0] << std::endl;
			continue;
		}
	}
	engine_for_go_command.stop_search.store(true, std::memory_order_relaxed);
	t.join();

	
	return 0;
}
