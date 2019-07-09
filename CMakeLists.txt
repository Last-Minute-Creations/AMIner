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
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data")
SET(TOOLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/deps/ace/tools")
SET(BITMAP_EXTRACT "${TOOLS_PATH}/bin/bitmap_extract")
SET(TILESET_CONV "${TOOLS_PATH}/bin/tileset_conv")

set(palette_aminer_unique "${CMAKE_CURRENT_SOURCE_DIR}/_res/palettes/aminer_editing.gpl")
file(GLOB palette_aminer_gpl _res/palettes/aminer.gpl)
file(GLOB palette_lmc_gpl _res/palettes/lmc.gpl)
set(palette_aminer "${CMAKE_CURRENT_SOURCE_DIR}/data/aminer.plt")
set(palette_lmc "${CMAKE_CURRENT_SOURCE_DIR}/data/lmc.plt")
convertPalette(${TARGET_NAME} ${palette_aminer_gpl} ${palette_aminer})
convertPalette(${TARGET_NAME} ${palette_lmc_gpl} ${palette_lmc})
set(transparency_hex "#993399")

convertBitmaps(
	TARGET ${TARGET_NAME} PALETTE ${palette_aminer_unique}
	INTERLEAVED SOURCES
		_res/commrade/bg.png _res/commrade/buttons.png _res/hud.png
	DESTINATIONS
		data/comm_bg.bm data/comm_buttons.bm data/hud.bm
)

convertBitmaps(
	TARGET ${TARGET_NAME} PALETTE ${palette_lmc}
	INTERLEAVED SOURCES _res/lmc.png DESTINATIONS data/lmc.bm
)

convertBitmaps(
	TARGET ${TARGET_NAME} PALETTE ${palette_aminer_unique} MASK_COLOR ${transparency_hex}
	SOURCES _res/logo.png DESTINATIONS data/logo.bm MASKS data/logo_mask.bm
)

convertBitmaps(
	TARGET ${TARGET_NAME} PALETTE ${palette_aminer_unique} MASK_COLOR ${transparency_hex}
	INTERLEAVED SOURCES
		_res/drill.png _res/tool.png _res/drill_2.png _res/tool_2.png
		_res/track.png _res/jet.png _res/bones.png
	DESTINATIONS
		data/drill.bm data/tool.bm data/drill_2.bm data/tool_2.bm
		data/track.bm data/jet.bm data/bones.bm
	MASKS
		data/drill_mask.bm data/tool_mask.bm data/drill_2_mask.bm data/tool_2_mask.bm
		data/track_mask.bm data/jet_mask.bm data/bones_mask.bm
)

convertFont(${TARGET_NAME} _res/uni54 data/uni54.fnt)

function(tileExtractFromPng TARGET_NAME TILE_PREFIX TILESET_DIR TILESET_OUTPUT)
	set(TILE_IDX 0)
	set(TILESET_DEPENDS "")
	while(TILE_IDX LESS "256")
		set(TILE_LIST ${TILE_PREFIX}_${TILE_IDX})
		if(${TILE_LIST})
			LIST(GET ${TILE_LIST} 0 TILE_x)
			LIST(GET ${TILE_LIST} 1 TILE_y)
			LIST(GET ${TILE_LIST} 2 TILE_src)
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

	add_custom_command(
		OUTPUT ${TILESET_OUTPUT}
		COMMAND ${TILESET_CONV}
			${TILESET_DIR} 32 ${TILESET_OUTPUT} -i -plt ${palette_aminer_unique}
		DEPENDS ${palette_aminer_unique} ${TILESET_DEPENDS}
	)
	target_sources(${TARGET_NAME} PUBLIC ${TILESET_OUTPUT})
endfunction()

include("base0.cmake")
include("base1.cmake")
include("tiles.cmake")
include("checkpoint.cmake")
