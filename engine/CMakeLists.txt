add_library(engine)

add_subdirectory(magic_numbers_generation)

target_link_libraries(engine
	PRIVATE
		utils)

# 2. Find all .cpp files in the src directory
file(GLOB LIB_SOURCES CONFIGURE_DEPENDS "*.cpp")

# 3. Find all .h files in the include directory
file(GLOB LIB_HEADERS CONFIGURE_DEPENDS "*.h")

target_sources(engine
	PRIVATE
		${LIB_SOURCES}
	PUBLIC
		FILE_SET HEADERS
		FILES
			${LIB_HEADERS}
)
