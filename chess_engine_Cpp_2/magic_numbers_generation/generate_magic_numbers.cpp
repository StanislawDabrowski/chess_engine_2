#include <iostream>
#include <vector>
#include <unordered_map>
#include <bit>
#include <fstream>
#include <iomanip>
#include <random>
#include <chrono>
#include "MoveGenerator.h"
#include "Utils.h"


uint64_t random_magic_candidate()
{
	static std::mt19937_64 rng(
		(uint64_t)std::chrono::high_resolution_clock::now().time_since_epoch().count()
	);
	return rng() & rng() & rng();
}




std::vector<uint64_t> generate_blocker_subsets(MoveGenerator const& mg, int square, bool bishop) {
	std::vector<uint64_t> blockers;
	Bitboard mask = bishop ? mg.bishop_relevant_blockers[square] : mg.rook_relevant_blockers[square];
	int bits = std::popcount(mask);
	uint64_t subset_count = 1ULL << bits;
	for (int i = 0; i < subset_count; i++) {
		blockers.push_back(Utils::occupancy_from_index(i, mask));
	}
	return blockers;
}


size_t find_magic(MoveGenerator &mg, int square, int relevant_bits, bool bishop) {
	//hypothetically can fail if size_t has less than 16 bits. Since C++11 it's guaranteed size_t has at least 16 bits
	std::vector<uint64_t> blockers = generate_blocker_subsets(mg, square, bishop);
	Bitboard* attacks = new Bitboard[blockers.size()];
	for (int i = 0;i<blockers.size();++i)
	{
		if (bishop)
		{
			attacks[i] = mg.generate_bishop_attack_table(square, i);
		}
		else
		{
			attacks[i] = mg.generate_rook_attack_table(square, i);
		}
	}

	for(int counter = 0;;counter++) {
		size_t magic = random_magic_candidate();
		std::unordered_map<uint32_t, Bitboard> used;

		bool fail = false;
		for (int i = 0; i < blockers.size(); i++) {
			size_t index = (blockers[i] * magic) >> (64 - relevant_bits);
			
			if (used.contains(index) && used[index] != attacks[i])
			{
				fail = true;
				break;
			} used[index] = attacks[i];
		}
		if (!fail)
		{
			delete[] attacks;
			return magic;
		}
		if (counter % 10000 == 0)
			std::cout << "Still searching for square " << square << " (" << (bishop ? "Bishop" : "Rook") << ") after " << counter << " tries\r" << std::flush;
	}
}

int main()
{
	std::ofstream magic_numbers_descriptive_file("magic_numbers_descriptive.txt");
	std::ofstream magic_numbers_file("magic_numbers.txt");
	std::ofstream magic_numbers_code_file("magic_numbers.cpp");
	if (!magic_numbers_descriptive_file) {
		std::cerr << "Error opening file\n";
		return 1;
	}
	if (!magic_numbers_file) {
		std::cerr << "Error opening file\n";
		return 1;
	}
	if (!magic_numbers_code_file) {
		std::cerr << "Error opening file\n";
	}

	MoveGenerator mg(nullptr);


	uint64_t bishop_magic_numbers[64];
	uint64_t rook_magic_numbers[64];
	std::cout << "searching started" << std::endl;
	for (int square = 0; square < 64; square++) {
		int bishop_bits = std::popcount(mg.bishop_relevant_blockers[square]);
		int rook_bits = std::popcount(mg.rook_relevant_blockers[square]);
		uint64_t bishop_magic = find_magic(mg, square, bishop_bits, true);
		uint64_t rook_magic = find_magic(mg, square, rook_bits, false);
		std::cout << "Square: " << square
					<< " Bishop Magic: " << std::hex << bishop_magic
					<< " Rook Magic: " << std::hex << rook_magic << std::dec << std::endl;

		bishop_magic_numbers[square] = bishop_magic;
		rook_magic_numbers[square] = rook_magic;


		//print progress
		std::cout << std::fixed << std::setprecision(2) << "Progress: " << (square + 1) / 64.0f * 100 << "%\r" << std::flush;


	}
	for (int square = 0;square < 64;++square)
	{
		uint64_t bishop_magic = bishop_magic_numbers[square];
		uint64_t rook_magic = rook_magic_numbers[square];
		magic_numbers_descriptive_file << "Square: " << square
			<< " Bishop Magic: " << std::hex << bishop_magic
		<< " Rook Magic: " << std::hex << rook_magic << std::dec << std::endl;
		magic_numbers_file << std::hex << bishop_magic << "," << rook_magic << std::dec << std::endl;
	}
	magic_numbers_code_file << "#include \"MoveGenerator.h\"\n\n";
	magic_numbers_code_file << "size_t MoveGenerator::bishop_magic_numbers[64] = {"<<std::hex<<"\n";
	for (int square = 0;square < 64;++square)
	{
		magic_numbers_code_file << "0x" << bishop_magic_numbers[square] << ",\n";
	}
	magic_numbers_code_file << "};\n";
	magic_numbers_code_file << "size_t MoveGenerator::rook_magic_numbers[64] = {"<<std::hex<<"\n";
	for (int square = 0;square < 64;++square)
	{
		magic_numbers_code_file << "0x" << rook_magic_numbers[square] << ",\n";
	}
	magic_numbers_code_file << "};"<<std::dec;
	



	magic_numbers_descriptive_file.close(); // close the file
	magic_numbers_file.close(); // close the file

	return 0;
}
