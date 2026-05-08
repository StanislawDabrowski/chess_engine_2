configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/ethereal_fen_set.fen
	${CMAKE_CURRENT_BINARY_DIR}/ethereal_fen_set.fen
	COPYONLY)

add_executable(run_unit_tests)
target_link_libraries(run_unit_tests
	PRIVATE
		engine)
target_link_libraries(run_unit_tests
	PRIVATE
		utils)
target_sources(run_unit_tests
	PRIVATE
		run_unit_tests.cpp
		unit_tests.cpp)

add_executable(run_perft_tests)
target_link_libraries(run_perft_tests
	PRIVATE
		engine)
target_link_libraries(run_perft_tests
	PRIVATE
		utils)
target_sources(run_perft_tests
	PRIVATE
		run_perft_tests.cpp
		perft_tests.cpp)
