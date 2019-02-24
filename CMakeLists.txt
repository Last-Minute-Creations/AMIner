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
SET(PALETTE_CONV "${TOOLS_PATH}/bin/palette_conv")
SET(BITMAP_CONV "${TOOLS_PATH}/bitmap_conv/bitmap_conv")
SET(BITMAP_EXTRACT "${TOOLS_PATH}/bin/bitmap_extract")
SET(TILESET_CONV "${TOOLS_PATH}/bin/tileset_conv")

file(GLOB palette_gimp _res/aminer.gpl)
set(palette_plt "${CMAKE_CURRENT_SOURCE_DIR}/data/aminer.plt")
add_custom_command(
	OUTPUT ${palette_plt}
	COMMAND ${PALETTE_CONV} ${palette_gimp} ${palette_plt}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${palette_gimp}
)
target_sources(${TARGET_NAME} PUBLIC ${palette_plt})

# Non-alpha bitmaps
file(GLOB bitmaps _res/shop_border.png)
foreach(bitmap_path ${bitmaps}) # /path/file.png
	get_filename_component(bitmap_file_png ${bitmap_path} NAME) # file.png
	STRING(REPLACE ".png" ".bm" bitmap_file_bm ${bitmap_file_png}) # file.bm
	SET(bitmap_out_name "${data_path}/${bitmap_file_bm}") # data/file.bm
	add_custom_command(
		OUTPUT ${bitmap_out_name}
		COMMAND ${BITMAP_CONV} ${palette_plt} ${bitmap_path}
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
)
file(GLOB bitmaps_planar
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

foreach(bitmap_path ${bitmaps_planar}) # /path/file.png
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
			-o ${bitmap_out_name} -mc ${color_transparent}
			-mo ${bitmap_out_mask}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS ${bitmap_path} ${palette_plt}
	)
	target_sources(${TARGET_NAME} PUBLIC ${bitmap_out_name} ${bitmap_out_mask})
endforeach()

set(TILE_0 0 0 "base1.png") # TILE_BASE_BG_FIRST
set(TILE_1 0 32 "base1.png")
set(TILE_2 0 64 "base1.png")
set(TILE_3 0 96 "base1.png")
set(TILE_4 0 128 "base1.png")
set(TILE_5 288 128 "base1.png")
set(TILE_6 0 160 "base1.png")
set(TILE_7 64 160 "base1.png")
set(TILE_8 160 160 "base1.png")
set(TILE_9 192 160 "base1.png")
set(TILE_10 224 160 "base1.png")
set(TILE_11 256 160 "base1.png")
set(TILE_12 288 160 "base1.png")

# TILE_BASE_BG_FIRST + 13 .. 22
set(TILE_IDX 13)
set(TILE_x 0)
while(TILE_IDX LESS "23")
	set(TILE_${TILE_IDX} ${TILE_x} 192 "base1.png")
	math(EXPR TILE_x "${TILE_x} + 32")
	math(EXPR TILE_IDX "${TILE_IDX} + 1")
endwhile()

# TILE_BASE_BG_FIRST + 23 .. 32
set(TILE_IDX 23)
set(TILE_x 0)
while(TILE_IDX LESS "33")
	set(TILE_${TILE_IDX} ${TILE_x} 224 "base1.png")
	math(EXPR TILE_IDX "${TILE_IDX} + 1")
	math(EXPR TILE_x "${TILE_x} + 32")
endwhile()

# Rest of base tiles
set(TILE_33 32 256 "base1.png") # TILE_BASE_SHAFT
set(TILE_34 0 256 "base1.png") # TILE_BASE_GROUND

set(TILE_35 66 0 "dirt.png") # TILE_CAVE_BG
# TILE_CAVE_BG with edges:
	set(TILE_36 66 66 "dirt.png")  # N
	set(TILE_37 0 99 "dirt.png")   # S
	set(TILE_38 33 66 "dirt.png")  # NS
	set(TILE_39 99 66 "dirt.png")  # E
	set(TILE_40 66 99 "dirt.png")  # NE
	set(TILE_41 99 99 "dirt.png")  # SE
	set(TILE_42 0 33 "dirt.png")   # NSE
	set(TILE_43 33 99 "dirt.png")  # W
	set(TILE_44 33 132 "dirt.png") # NW
	set(TILE_45 0 132 "dirt.png")  # SW
	set(TILE_46 99 33 "dirt.png")  # NSW
	set(TILE_47 0 66 "dirt.png")   # EW
	set(TILE_48 66 33 "dirt.png")  # NEW
	set(TILE_49 33 33 "dirt.png")  # SEW
	set(TILE_50 99 0 "dirt.png")   # NSEW
set(TILE_51 198 33 "dirt.png") # TILE_STONE_1
set(TILE_52 198 66 "dirt.png") # TILE_STONE_2
set(TILE_53 0 0 "dirt.png") # TILE_ROCK_1
set(TILE_54 33 0 "dirt.png") # TILE_ROCK_2
set(TILE_55 1 1 "minerals.png") # TILE_GOLD_1
set(TILE_56 1 34 "minerals.png") # TILE_GOLD_2
set(TILE_57 1 67 "minerals.png") # TILE_GOLD_3
set(TILE_58 34 1 "minerals.png") # TILE_SILVER_1
set(TILE_59 34 34 "minerals.png") # TILE_SILVER_2
set(TILE_60 34 67 "minerals.png") # TILE_SILVER_3
set(TILE_61 67 232 "minerals.png") # TILE_EMERALD_1
set(TILE_62 34 232 "minerals.png") # TILE_EMERALD_2
set(TILE_63 1 232 "minerals.png") # TILE_EMERALD_3
set(TILE_64 67 199 "minerals.png") # TILE_RUBY_1
set(TILE_65 34 199 "minerals.png") # TILE_RUBY_2
set(TILE_66 1 199 "minerals.png") # TILE_RUBY_3
set(TILE_67 67 265 "minerals.png") # TILE_MOONSTONE_1
set(TILE_68 34 265 "minerals.png") # TILE_MOONSTONE_2
set(TILE_69 1 265 "minerals.png") # TILE_MOONSTONE_3
set(TILE_70 1 298 "minerals.png") # TILE_COAL_1
set(TILE_71 34 298 "minerals.png") # TILE_COAL_2
set(TILE_72 67 298 "minerals.png") # TILE_COAL_3
set(TILE_73 67 298 "minerals.png") # TILE_COAL_3, TODO: Proper checkpoint
set(TILE_74 67 298 "minerals.png") # TILE_COAL_3, TODO: Proper finish

set(TILE_IDX 0)
set(TILESET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/_res/tiles")
set(TILESET_DEPENDS "")
while(TILE_IDX LESS "256")
	if(TILE_${TILE_IDX})
		LIST(GET TILE_${TILE_IDX} 0 TILE_x)
		LIST(GET TILE_${TILE_IDX} 1 TILE_y)
		LIST(GET TILE_${TILE_IDX} 2 TILE_src)
		SET(TILE_PNG_PATH "${TILESET_DIR}/${TILE_IDX}.png")
		SET(SRC_PNG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/_res/${TILE_src}")

		add_custom_command(
			OUTPUT ${TILE_PNG_PATH}
			COMMAND ${BITMAP_EXTRACT}
				${SRC_PNG_PATH} ${TILE_x} ${TILE_y} 32 32
				${TILE_PNG_PATH}
			DEPENDS ${SRC_PNG_PATH}
		)
		set(TILESET_DEPENDS "${TILESET_DEPENDS};${TILE_PNG_PATH}")
	endif()
	math(EXPR TILE_IDX "${TILE_IDX} + 1")
endwhile()

set(TILESET_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/data/tiles.bm")
add_custom_command(
	OUTPUT ${TILESET_OUTPUT}
	COMMAND ${TILESET_CONV}
		${TILESET_DIR} 32 ${TILESET_OUTPUT} -i -plt ${palette_plt}
	DEPENDS ${palette_plt} ${TILESET_DEPENDS}
)
target_sources(${TARGET_NAME} PUBLIC ${TILESET_OUTPUT})
