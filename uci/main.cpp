#include "Engine.h"
#include "Utils.h"
#include "TimeManagement.h"
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <thread>
#include <atomic>
#include <syncstream>
#include <chrono>
#include <cmath>



Engine engine = Engine();
Board board_copy;
std::mutex engine_mutex;
std::mutex board_copy_mutex;

std::atomic<bool> debug_mode = false;


std::unordered_set<std::thread::id> running_threads;
std::mutex running_threads_mutex;
std::mutex running_thread_being_added_mutex;//locked before starting a thread and unclocked after adding an id. The thread locks before constructing the HandleRunningThread.

class HandleRunningThread
{
	public:
	HandleRunningThread()
	{
		running_thread_being_added_mutex.lock();
		running_thread_being_added_mutex.unlock();
	}
	~HandleRunningThread()
	{
		running_threads_mutex.lock();
		running_threads.erase(std::this_thread::get_id());
		running_threads_mutex.unlock();
	}
};

void do_nothing_command_function(std::vector<std::string> args)
{
	//does nothing, for command which for now don't need any implementation, like e.g. ucinewgame
	HandleRunningThread handle_thread;
}

void d_command_function(std::vector<std::string> args)
{
	HandleRunningThread handle_thread;
	std::osyncstream out(std::cout);//necessary to pass the out as an lvalue not rvalue
	board_copy_mutex.lock();
	Utils::display_board(&board_copy, out);
	board_copy_mutex.unlock();
}

void isready_command_function(std::vector<std::string> args)
{
	HandleRunningThread handle_thread;
	while (true)
	{
		running_threads_mutex.lock();
		if (running_threads.size() == 1)//only the current thread is running
		{
			running_threads_mutex.unlock();
			break;
		}
		running_threads_mutex.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(3));//arbitrary small sleep to avoid busy waiting without making the engine unresponsive for too long
	}
	std::osyncstream out(std::cout);
	out << "readyok" << std::endl;
}

void uci_command_function(std::vector<std::string> args)
{
	HandleRunningThread handle_thread;
	std::osyncstream out(std::cout);
	out << "id name ChessEngine2" << std::endl;
	out << "id author Avalfortz" << std::endl;
	out << "uciok" << std::endl;
}

void debug_command_function(std::vector<std::string> args)
{
	HandleRunningThread handle_thread;
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
		{ debug_mode.store(false, std::memory_order_relaxed);
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
	HandleRunningThread handle_thread;
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
		for (int i = 0;i<engine.board.positions_stack[engine.board.current_position_idx].legal_moves_length;++i)
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
		engine_mutex.lock();
		long time_passed = -1;
		if (depth_max != -1)
		{
			auto start_time = std::chrono::high_resolution_clock::now();
			for (uint8_t depth = 1;depth<=depth_max;++depth)
			{
				engine.nodes_searched = 0;
				if (engine.board.side_to_move == White)
					search_result = engine.search<White, true, true>(depth);
				else
					search_result = engine.search<Black, true, true>(depth);
				time_passed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
				out << "info depth " << static_cast<int>(depth) << " score cp " << search_result.second << " nodes " << engine.nodes_searched << " nps " << (time_passed > 0 ? engine.nodes_searched * 1'000'000 / time_passed : 0) << " time " << static_cast<int>(std::round((static_cast<float>(time_passed)/1000.0))) << std::endl;
			}
		}
		else
		{
			if (winc == -1)
				winc = 0;
			if (binc == -1)
				binc = 0;
			uint64_t time_to_think = get_time_to_think_in_ms(&engine, wtime, btime, winc, binc);
			auto start_time = std::chrono::high_resolution_clock::now();
			float effective_branching_factor_estimate = 1;
			long previous_time_passed = -1;
			for (uint8_t depth = 1;true;++depth)
			{
				engine.nodes_searched = 0;
				if (engine.board.side_to_move == White)
					search_result = engine.search<White, true, true>(depth);
				else
					search_result = engine.search<Black, true, true>(depth);
				previous_time_passed = time_passed;
				time_passed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
				if (previous_time_passed != -1)
				{
					effective_branching_factor_estimate = time_passed/static_cast<float>(previous_time_passed);
				}
				out << "info depth " << static_cast<int>(depth) << " score cp " << search_result.second << " nodes " << engine.nodes_searched << " nps " << (time_passed > 0 ? engine.nodes_searched * 1'000'000 / time_passed : 0) << " time " << static_cast<int>(std::round((static_cast<float>(time_passed)/1000.0))) << std::endl;
				long estimated_time_for_next_depth = time_passed * effective_branching_factor_estimate;
				if (estimated_time_for_next_depth * 1.2 > time_to_think * 1000)//*1000 is neccessary we measure time in microseconds but time_to_think is in milliseconds
					break;
			}
		}
		
		out << "bestmove " << Utils::move_to_string(search_result.first) << std::endl;
		engine_mutex.unlock();


	}
}
void position_command_function(std::vector<std::string> args)
{
	HandleRunningThread handle_thread;
	engine_mutex.lock();
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
	board_copy_mutex.lock();
	board_copy = engine.board;
	board_copy_mutex.unlock();
	engine_mutex.unlock();
}


void fen_command_function(std::vector<std::string> args)
{
	HandleRunningThread handle_thread;
	std::osyncstream out(std::cout);
	out << std::emit_on_flush;
	board_copy_mutex.lock();
	out << Utils::get_fen(&board_copy) << std::endl;
	board_copy_mutex.unlock();
}

void hash_command_function(std::vector<std::string> args)
{
	HandleRunningThread handle_thread;
	std::osyncstream out(std::cout);
	out << std::emit_on_flush;
	board_copy_mutex.lock();
	out << std::hex << board_copy.positions_stack[engine.board.current_position_idx].hash << std::dec << std::endl;
	board_copy_mutex.unlock();
}

void move_command_function(std::vector<std::string> args)
{
	HandleRunningThread handle_thread;
	if (args.empty())
		return;
	engine_mutex.lock();
	Move move_temp = Utils::string_to_move(&engine.board, args[0]);
	engine.board.make_move(move_temp);
	board_copy_mutex.lock();
	board_copy = engine.board;
	board_copy_mutex.unlock();
	engine_mutex.unlock();
}

void unmove_command_function(std::vector<std::string> args)
{
	HandleRunningThread handle_thread;
	engine_mutex.lock();
	engine.board.unmake_move();
	board_copy_mutex.lock();
	board_copy = engine.board;
	board_copy_mutex.unlock();
	engine_mutex.unlock();
}

void eval_command_function(std::vector<std::string> args)
{
	HandleRunningThread handle_thread;
	std::osyncstream out(std::cout);
	out << std::emit_on_flush;
	engine_mutex.lock();
	int16_t eval;
	if (engine.board.side_to_move == White)
		eval = engine.se.evaluate<White>();
	else
		eval = engine.se.evaluate<Black>();
	out << "Static evaluation: " << eval << std::endl;
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
	board_copy = engine.board;//no need to lock here since no other thread is running yet
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
	std::osyncstream out(std::cout);
	out << std::emit_on_flush;
	std::string input_line;
	std::vector<std::string> tokens;
	uint32_t command_count = 0;
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
			running_thread_being_added_mutex.lock();
			std::thread t(commands_functions[tokens[0]], std::vector<std::string>(tokens.begin()+1, tokens.end()));
			running_threads_mutex.lock();
			running_threads.insert(t.get_id());
			running_threads_mutex.unlock();
			running_thread_being_added_mutex.unlock();
			threads.emplace(t.get_id(), std::move(t));
			++command_count;
		}
		else
		{
			out << "Unknown command: " << tokens[0] << std::endl;
			continue;
		}
		//clean up finished threads
		for (auto it = threads.begin(); it != threads.end();)
		{
			running_threads_mutex.lock();
			if (!running_threads.contains(it->first))
			{
				running_threads_mutex.unlock();
				if (it->second.joinable())
					it->second.join();
				it = threads.erase(it);
			}
			else
			{
				running_threads_mutex.unlock();
				++it;
			}
		}

	}

	//join all threads
	for (auto it = threads.begin(); it != threads.end(); ++it)
	{
		if (it->second.joinable())
			it->second.join();
	}
	
	return 0;
}
