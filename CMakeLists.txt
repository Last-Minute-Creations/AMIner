cmake_minimum_required(VERSION 2.8.5)
project(aminer)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT AMIGA)
	message(SEND_ERROR "This project only compiles for Amiga")
endif()

# ACE
add_subdirectory(deps/ace ace)

set(CMAKE_C_STANDARD 11)
file(GLOB_RECURSE SOURCES src/*.c src/*.h)
add_executable(aminer ${SOURCES})
target_include_directories(aminer PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_options(aminer PUBLIC -Wall -Wextra)
target_link_libraries(aminer ace)
if(GAME_DEBUG)
  target_compile_definitions(aminer PRIVATE GAME_DEBUG)
endif()

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
convertPalette(aminer ${palette_aminer_gpl} ${palette_aminer})
convertPalette(aminer ${palette_lmc_gpl} ${palette_lmc})
set(transparency_hex "#993399")

convertBitmaps(
	TARGET aminer PALETTE ${palette_aminer_unique}
	INTERLEAVED SOURCES
		_res/commrade/bg.png _res/commrade/buttons.png _res/commrade/faces.png
		_res/hud/hud.png _res/hud/mode_icons.png _res/logo.png
	DESTINATIONS
		data/comm_bg.bm data/comm_buttons.bm data/comm_faces.bm
		data/hud.bm data/mode_icons.bm data/logo.bm
)

convertBitmaps(
	TARGET aminer PALETTE ${palette_lmc}
	INTERLEAVED SOURCES _res/lmc.png DESTINATIONS data/lmc.bm
)

convertBitmaps(
	TARGET aminer PALETTE ${palette_aminer_unique} MASK_COLOR ${transparency_hex}
	INTERLEAVED SOURCES
		_res/drill.png _res/tool.png _res/wreck.png
		_res/drill_2.png _res/tool_2.png _res/wreck_2.png
		_res/track.png _res/jet.png _res/bones.png _res/smoke.png
		_res/explosion.png _res/teleport.png _res/hud/mode_cursor.png
		_res/bomb_marker.png
	DESTINATIONS
		data/drill.bm data/tool.bm data/wreck.bm
		data/drill_2.bm data/tool_2.bm data/wreck_2.bm
		data/track.bm data/jet.bm data/bones.bm data/smoke.bm
		data/explosion.bm data/teleport.bm data/mode_cursor.bm
		data/bomb_marker.bm
	MASKS
		data/drill_mask.bm data/tool_mask.bm data/wreck_mask.bm
		NONE NONE NONE
		data/track_mask.bm data/jet_mask.bm data/bones_mask.bm data/smoke_mask.bm
		data/explosion_mask.bm data/teleport_mask.bm data/mode_cursor_mask.bm
		data/bomb_marker_mask.bm
)

convertFont(aminer _res/uni54 data/uni54.fnt)

function(tileExtractFromPng aminer TILE_PREFIX TILESET_DIR TILESET_OUTPUT)
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
	target_sources(aminer PUBLIC ${TILESET_OUTPUT})
endfunction()

include("base0.cmake")
include("base1.cmake")
include("tiles.cmake")
include("checkpoint.cmake")
