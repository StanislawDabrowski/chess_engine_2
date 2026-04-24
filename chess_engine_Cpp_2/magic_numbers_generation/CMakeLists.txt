add_executable(generate_magic_numbers)


target_link_libraries(generate_magic_numbers
	PRIVATE
		chess_engine)
target_link_libraries(generate_magic_numbers
	PRIVATE
		utils)

target_sources(generate_magic_numbers
	PRIVATE
		"generate_magic_numbers.cpp")
