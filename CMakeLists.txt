cmake_minimum_required(VERSION 2.8.5)
project(aminer)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Lowercase project name for binaries and packaging
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

if(NOT AMIGA)
	message(SEND_ERROR "This project only compiles for Amiga")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DAMIGA -Wall -Wextra -fomit-frame-pointer")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DACE_DEBUG")
file(GLOB_RECURSE SOURCES src/*.c)
file(GLOB_RECURSE HEADERS src/*.h)

include_directories(
	${PROJECT_SOURCE_DIR}/src
)

if(GAME_DEBUG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGAME_DEBUG")
endif()
if(ACE_DEBUG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DACE_DEBUG") # For ACE headers with ifdefs
endif()

# ACE
add_subdirectory(deps/ace ace)
include_directories(deps/ace/include)

# Linux/other UNIX get a lower-case binary name
set(TARGET_NAME ${PROJECT_NAME_LOWER})
add_executable(${TARGET_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${TARGET_NAME} ace)

# Palette
SET(data_path "${CMAKE_CURRENT_SOURCE_DIR}/data")
file(MAKE_DIRECTORY ${data_path})
SET(TOOLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/deps/ace/tools")
SET(PALETTE_CONV "${TOOLS_PATH}/palette_conv/palette_conv")
SET(BITMAP_CONV "${TOOLS_PATH}/bitmap_conv/bitmap_conv")

file(GLOB palette_gimp _res/aminer.gpl)
set(palette_plt data/aminer.plt)
add_custom_command(
	OUTPUT ${palette_plt}
	COMMAND ${PALETTE_CONV}
		${palette_gimp}
		${palette_plt}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${palette_gimp}
)
target_sources(${TARGET_NAME} PUBLIC ${palette_plt})

# Non-alpha bitmaps
file(GLOB bitmaps _res/tiles.png)
foreach(bitmap_path ${bitmaps}) # /path/file.png
	get_filename_component(bitmap_file_png ${bitmap_path} NAME) # file.png
	STRING(REPLACE ".png" ".bm" bitmap_file_bm ${bitmap_file_png}) # file.bm
	SET(bitmap_out_name "${data_path}/${bitmap_file_bm}") # data/file.bm
	add_custom_command(
		OUTPUT ${bitmap_out_name}
		COMMAND ${BITMAP_CONV}
			${palette_plt}
			${bitmap_path}
			-o ${bitmap_out_name} -i
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS ${bitmap_path} ${palette_plt}
	)
	target_sources(${TARGET_NAME} PUBLIC ${bitmap_out_name})
endforeach()

# Alpha bitmaps

file(GLOB bitmaps
	_res/drill.png _res/tool.png
	_res/drill_2.png _res/tool_2.png
	_res/track.png _res/jet.png
  _res/logo.png
)

set(color_transparent "\\#993399")
foreach(bitmap_path ${bitmaps}) # /path/file.png
	get_filename_component(bitmap_file_png ${bitmap_path} NAME) # file.png
	STRING(REPLACE ".png" ".bm" bitmap_file_bm ${bitmap_file_png}) # file.bm
	STRING(REPLACE ".png" "_mask.bm" bitmap_file_mask ${bitmap_file_png}) # file.bm
	SET(bitmap_out_name "${data_path}/${bitmap_file_bm}") # data/file.bm
	SET(bitmap_out_mask "${data_path}/${bitmap_file_mask}") # data/file.bm
	add_custom_command(
		OUTPUT ${bitmap_out_name} ${bitmap_out_mask}
		COMMAND ${BITMAP_CONV}
			${palette_plt}
			${bitmap_path}
			-o ${bitmap_out_name} -i -mc ${color_transparent}
			-mo ${bitmap_out_mask}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS ${bitmap_path} ${palette_plt}
	)
	target_sources(${TARGET_NAME} PUBLIC ${bitmap_out_name} ${bitmap_out_mask})
endforeach()
