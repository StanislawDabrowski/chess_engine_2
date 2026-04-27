add_library(utils)

target_link_libraries(utils
	PRIVATE
		engine)
target_sources(utils
	PRIVATE
		"Utils.cpp"
	PUBLIC
		FILE_SET HEADERS
		FILES
			Utils.h)
