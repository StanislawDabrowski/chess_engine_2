#include "Engine.h"
#include "Utils.h"
#include <unordered_map>
#include <functional>


Engine engine = Engine();


void d_command_function(std::vector<std::string> args)
{
	Utils::display_board(&engine.board, std::cout);
}

void exit_command_function(std::vector<std::string> args)
{
	exit(0);
}

void go_command_function(std::vector<std::string> args)
{

	if (args.size() >= 2 && args[0]=="perft")
	{
		uint8_t depth = std::stoi(args[1]);
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
			std::cout << Utils::move_to_string(move) << ": " << perft_result << std::endl;
			engine.board.unmake_move();
		}
		std::cout << count << std::endl;
	}
	else
	{
		int16_t depth = -1;
		bool count_searched_nodes = false;
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
					std::cout << "No depth specified after 'depth'" << std::endl;
					return;
				}
			}
			else if (args[i] == "count_nodes")
			{
				count_searched_nodes = true;
			}
			else
			{
				std::cout << "Unknown argument: " << args[i] << std::endl;
				return;
			}
		}
		if (depth == -1)
		{
			std::cout << "No depth specified" << std::endl;
			return;
		}
		std::pair<Move, int16_t> search_result;
		if (count_searched_nodes)
			engine.nodes_searched = 0;
		if (engine.board.side_to_move == White)
		{
			if (count_searched_nodes)
				search_result = engine.search<White, true, true>(depth);
			else
				search_result = engine.search<White, true, false>(depth);
		}
		else
		{
			if (count_searched_nodes)
				search_result = engine.search<Black, true, true>(depth);
			else
				search_result = engine.search<Black, true, false>(depth);
		}
		std::cout << "bestmove " << Utils::move_to_string(search_result.first) << std::endl;
		std::cout << "score " << search_result.second << std::endl;
		if (count_searched_nodes)
		{
			std::cout << "nodes searched " << engine.nodes_searched << std::endl;
			engine.nodes_searched = 0;
		}


	}
}
void position_command_function(std::vector<std::string> args)
{
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
			return;
		}
		
	}
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
	std::unordered_map<std::string, std::function<void(std::vector<std::string>)>> commands_functions = {
		{"d", d_command_function},
		{"exit", exit_command_function},
		{"e", exit_command_function},
		{"quit", exit_command_function},
		{"q", exit_command_function},
		{"go", go_command_function},
		{"position", position_command_function}
	};
	

	std::string input_line;
	std::vector<std::string> tokens;
	while (true)
	{
		
		std::getline(std::cin, input_line);
		tokens = tokenize(input_line);
		if (tokens.empty())
			continue;
		if (commands_functions.contains(tokens[0]))
		{
			commands_functions[tokens[0]](std::vector<std::string>(tokens.begin()+1, tokens.end()));
		}
		else
		{
			std::cout << "Unknown command: " << tokens[0] << std::endl;
			continue;
		}

	}

	return 0;
}
