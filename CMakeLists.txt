cmake_minimum_required(VERSION 3.0.0)
project(aminer)

if(NOT AMIGA)
	message(SEND_ERROR "This project only compiles for Amiga")
endif()

set(CMAKE_C_STANDARD 11)

# ACE
add_subdirectory(deps/ace ace)
file(GLOB_RECURSE AMINER_src src/*.c src/*.h)

if(ELF2HUNK)
set(AMINER_EXECUTABLE Aminer.elf)
set(AMINER_OUTPUT_EXECUTABLE Aminer.exe)
add_executable(${AMINER_EXECUTABLE} ${AMINER_src})
target_link_libraries(${AMINER_EXECUTABLE} -Wl,-Map=Aminer.map)
add_custom_command(
	TARGET ${AMINER_EXECUTABLE} POST_BUILD
	COMMAND ${ELF2HUNK} ${AMINER_EXECUTABLE} ${AMINER_OUTPUT_EXECUTABLE}
)
add_custom_command(
	TARGET ${AMINER_EXECUTABLE} POST_BUILD
	COMMAND ${OBJDUMP} --disassemble -S ${AMINER_EXECUTABLE} > Aminer.s
)
else()
	SET(AMINER_EXECUTABLE Aminer)
	SET(AMINER_OUTPUT_EXECUTABLE Aminer)
	add_executable(${AMINER_EXECUTABLE} ${AMINER_src})
endif()

target_include_directories(${AMINER_EXECUTABLE} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_options(${AMINER_EXECUTABLE} PUBLIC -Wall -Wextra -Wimplicit-fallthrough=2)
target_link_libraries(${AMINER_EXECUTABLE} ace)
if(GAME_DEBUG)
  target_compile_definitions(${AMINER_EXECUTABLE} PRIVATE GAME_DEBUG)
	target_compile_definitions(ace PUBLIC ACE_DEBUG_UAE)
endif()

set(RES_DIR ${CMAKE_CURRENT_LIST_DIR}/_res)
set(DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)
set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${DATA_DIR})
file(MAKE_DIRECTORY ${GEN_DIR})

# Copy already processed files
file(GLOB COPY_FILES ${RES_DIR}/copied/*)
file(COPY ${COPY_FILES} DESTINATION ${DATA_DIR})
file(COPY ${RES_DIR}/aminer.info DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Palette
set(palette_aminer_unique "${RES_DIR}/palettes/aminer_editing.gpl")
file(GLOB palette_aminer_gpl ${RES_DIR}/palettes/aminer.gpl)
file(GLOB palette_lmc_gpl ${RES_DIR}/palettes/lmc.gpl)
set(palette_aminer "${DATA_DIR}/aminer.plt")
set(palette_lmc "${DATA_DIR}/lmc.plt")
convertPalette(${AMINER_EXECUTABLE} ${palette_aminer_gpl} ${palette_aminer})
convertPalette(${AMINER_EXECUTABLE} ${palette_lmc_gpl} ${palette_lmc})
set(transparency_hex "#993399")

convertBitmaps(
	TARGET ${AMINER_EXECUTABLE} PALETTE ${palette_aminer_unique}
	INTERLEAVED SOURCES
		${RES_DIR}/commrade/bg.png ${RES_DIR}/commrade/buttons.png ${RES_DIR}/commrade/faces.png
		${RES_DIR}/hud/hud.png ${RES_DIR}/hud/mode_icons.png ${RES_DIR}/logo.png ${RES_DIR}/lang_select.png
		${RES_DIR}/commrade/faces_office.png ${RES_DIR}/commrade/office_selection.png
	DESTINATIONS
		${DATA_DIR}/comm_bg.bm ${DATA_DIR}/comm_buttons.bm ${DATA_DIR}/comm_faces.bm
		${DATA_DIR}/hud.bm ${DATA_DIR}/mode_icons.bm ${DATA_DIR}/logo.bm ${DATA_DIR}/lang_select.bm
		${DATA_DIR}/comm_faces_office.bm ${DATA_DIR}/comm_office_selection.bm
)

convertBitmaps(
	TARGET ${AMINER_EXECUTABLE} PALETTE ${palette_lmc}
	INTERLEAVED SOURCES ${RES_DIR}/lmc.png DESTINATIONS ${DATA_DIR}/lmc.bm
)

convertBitmaps(
	TARGET ${AMINER_EXECUTABLE} PALETTE ${palette_aminer_unique} MASK_COLOR ${transparency_hex}
	INTERLEAVED SOURCES
		${RES_DIR}/drill.png ${RES_DIR}/tool.png ${RES_DIR}/wreck.png
		${RES_DIR}/drill_2.png ${RES_DIR}/tool_2.png ${RES_DIR}/wreck_2.png
		${RES_DIR}/track.png ${RES_DIR}/jet.png ${RES_DIR}/bones.png ${RES_DIR}/smoke.png
		${RES_DIR}/explosion.png ${RES_DIR}/teleport.png ${RES_DIR}/hud/mode_cursor.png
		${RES_DIR}/bomb_marker.png
	DESTINATIONS
		${DATA_DIR}/drill.bm ${DATA_DIR}/tool.bm ${DATA_DIR}/wreck.bm
		${DATA_DIR}/drill_2.bm ${DATA_DIR}/tool_2.bm ${DATA_DIR}/wreck_2.bm
		${DATA_DIR}/track.bm ${DATA_DIR}/jet.bm ${DATA_DIR}/bones.bm ${DATA_DIR}/smoke.bm
		${DATA_DIR}/explosion.bm ${DATA_DIR}/teleport.bm ${DATA_DIR}/mode_cursor.bm
		${DATA_DIR}/bomb_marker.bm
	MASKS
		${DATA_DIR}/drill_mask.bm ${DATA_DIR}/tool_mask.bm ${DATA_DIR}/wreck_mask.bm
		NONE NONE NONE
		${DATA_DIR}/track_mask.bm ${DATA_DIR}/jet_mask.bm ${DATA_DIR}/bones_mask.bm ${DATA_DIR}/smoke_mask.bm
		${DATA_DIR}/explosion_mask.bm ${DATA_DIR}/teleport_mask.bm ${DATA_DIR}/mode_cursor_mask.bm
		${DATA_DIR}/bomb_marker_mask.bm
)

convertFont(
	TARGET ${AMINER_EXECUTABLE} SOURCE ${RES_DIR}/uni54 DESTINATION ${DATA_DIR}/uni54.fnt
)

function(tileExtractFromPng TARGET_EXECUTABLE TILE_PREFIX TILESET_DIR TILESET_OUTPUT)
	set(TILE_IDX 0)
	set(TILESET_DEPENDS "")
	file(MAKE_DIRECTORY ${TILESET_DIR})
	set(GENERATED_FILES "")
	while(TILE_IDX LESS "256")
		set(TILE_LIST ${TILE_PREFIX}_${TILE_IDX}) # var name to read next
		if(${TILE_LIST})
			LIST(GET ${TILE_LIST} 0 TILE_x)
			LIST(GET ${TILE_LIST} 1 TILE_y)
			LIST(GET ${TILE_LIST} 2 TILE_src)
			SET(TILE_PNG_PATH "${TILESET_DIR}/${TILE_IDX}.png")
			SET(SRC_PNG_PATH "${RES_DIR}/${TILE_src}")
			list(APPEND GENERATED_FILES "${TILE_IDX}.png")

			extractBitmaps(
				TARGET ${TARGET_EXECUTABLE} SOURCE ${SRC_PNG_PATH} DESTINATIONS
				${TILE_PNG_PATH} ${TILE_x} ${TILE_y} 32 32
			)
		endif()
		math(EXPR TILE_IDX "${TILE_IDX} + 1")
	endwhile()

	convertTileset(
		TARGET ${TARGET_EXECUTABLE} SIZE 32 PALETTE ${palette_aminer_unique}
		INTERLEAVED SOURCE ${TILESET_DIR} DESTINATION ${TILESET_OUTPUT}
		TILES ${GENERATED_FILES}
	)
endfunction()

include("base0.cmake")
include("base1.cmake")
include("tiles.cmake")
include("checkpoint.cmake")
