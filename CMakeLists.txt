cmake_minimum_required(VERSION 3.14.0)
project(aminer)

if(NOT AMIGA)
	message(SEND_ERROR "This project only compiles for Amiga")
endif()

set(CMAKE_C_STANDARD 11)

# ACE
add_subdirectory(deps/ace ace)
add_subdirectory(deps/ace_audio_mixer ace_audio_mixer)
file(GLOB_RECURSE GAME_src src/*.c src/*.h)

set(GAME_EXECUTABLE_STEM Aminer)
if(ELF2HUNK)
	set(GAME_EXECUTABLE ${GAME_EXECUTABLE_STEM}.elf)
	set(GAME_OUTPUT_EXECUTABLE ${GAME_EXECUTABLE_STEM}.exe)
	add_executable(${GAME_EXECUTABLE} ${GAME_src})
	target_link_options(${GAME_EXECUTABLE} PUBLIC -Wno-attributes)
	target_link_libraries(${GAME_EXECUTABLE} -Wl,-Map=${GAME_EXECUTABLE_STEM}.map)
	add_custom_command(
		TARGET ${GAME_EXECUTABLE} POST_BUILD
		COMMAND ${ELF2HUNK} ${GAME_EXECUTABLE} ${GAME_OUTPUT_EXECUTABLE}
	)
	add_custom_command(
		TARGET ${GAME_EXECUTABLE} POST_BUILD
		COMMAND ${OBJDUMP} --disassemble -S ${GAME_EXECUTABLE} > ${GAME_EXECUTABLE_STEM}.s
	)
else()
	SET(GAME_EXECUTABLE ${GAME_EXECUTABLE_STEM})
	SET(GAME_OUTPUT_EXECUTABLE ${GAME_EXECUTABLE_STEM})
	add_executable(${GAME_EXECUTABLE} ${GAME_src})
endif()

target_include_directories(${GAME_EXECUTABLE} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_options(${GAME_EXECUTABLE} PUBLIC -Wall -Wextra -Wimplicit-fallthrough=2)
target_compile_options(${GAME_EXECUTABLE} PRIVATE -Werror -Wno-error=unused-parameter -Wimplicit-fallthrough=2)
target_link_libraries(${GAME_EXECUTABLE} ace_audio_mixer ace)
if(GAME_DEBUG)
  target_compile_definitions(${GAME_EXECUTABLE} PRIVATE GAME_DEBUG)
endif()

set(RES_DIR ${CMAKE_CURRENT_LIST_DIR}/_res)
set(DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)
set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${DATA_DIR})
file(MAKE_DIRECTORY ${GEN_DIR})

# Copy already processed files
file(GLOB COPY_FILES ${RES_DIR}/copied/*)
file(COPY ${COPY_FILES} DESTINATION ${DATA_DIR})
set(GAME_INFO_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${GAME_OUTPUT_EXECUTABLE}.info")
add_custom_command(
	TARGET ${GAME_EXECUTABLE} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${RES_DIR}/aminer.info ${GAME_INFO_DESTINATION}
)

# Palette
set(palette_aminer_unique "${RES_DIR}/palettes/aminer_editing.gpl")
file(GLOB palette_aminer_gpl ${RES_DIR}/palettes/aminer.gpl)
file(GLOB palette_lmc_gpl ${RES_DIR}/palettes/lmc.gpl)
set(palette_aminer "${DATA_DIR}/aminer.plt")
set(palette_lmc "${DATA_DIR}/lmc.plt")
convertPalette(${GAME_EXECUTABLE} ${palette_aminer_gpl} ${palette_aminer})
convertPalette(${GAME_EXECUTABLE} ${palette_lmc_gpl} ${palette_lmc})
set(transparency_hex "#993399")

#-------------------------------------------------------------------------- BODY
file(MAKE_DIRECTORY ${GEN_DIR}/drill)
file(MAKE_DIRECTORY ${GEN_DIR}/drill_2)

extractBitmaps(TARGET ${GAME_EXECUTABLE} SOURCE ${RES_DIR}/krtk.png
	GENERATED_FILE_LIST "FRAMES_DRILL_P1"
	DESTINATIONS
	${GEN_DIR}/drill/0.png 0 76 32 20
	${GEN_DIR}/drill/1.png 0 44 32 20
	${GEN_DIR}/drill/2.png 0 12 32 20
	${GEN_DIR}/drill/3.png 159 12 32 20
	${GEN_DIR}/drill/4.png 96 76 32 20
	${GEN_DIR}/drill/5.png 96 44 32 20
	${GEN_DIR}/drill/6.png 96 12 32 20
	${GEN_DIR}/drill/7.png 192 12 32 20
)

convertTileset(
	TARGET ${GAME_EXECUTABLE} SIZE 32 HEIGHT 20
	SOURCE ${GEN_DIR}/drill DESTINATION ${GEN_DIR}/drill.png
	TILE_PATHS ${FRAMES_DRILL_P1}
)

extractBitmaps(TARGET ${GAME_EXECUTABLE} SOURCE ${RES_DIR}/krtk.png
	GENERATED_FILE_LIST "FRAMES_DRILL_P2"
	DESTINATIONS
	${GEN_DIR}/drill_2/0.png 0 236 32 20
	${GEN_DIR}/drill_2/1.png 0 204 32 20
	${GEN_DIR}/drill_2/2.png 0 172 32 20
	${GEN_DIR}/drill_2/3.png 255 12 32 20
	${GEN_DIR}/drill_2/4.png 96 236 32 20
	${GEN_DIR}/drill_2/5.png 96 204 32 20
	${GEN_DIR}/drill_2/6.png 96 172 32 20
	${GEN_DIR}/drill_2/7.png 288 12 32 20
)

convertTileset(
	TARGET ${GAME_EXECUTABLE} SIZE 32 HEIGHT 20
	SOURCE ${GEN_DIR}/drill_2 DESTINATION ${GEN_DIR}/drill_2.png
	TILE_PATHS ${FRAMES_DRILL_P2}
)

#------------------------------------------------------------------------- WRECK

file(MAKE_DIRECTORY ${GEN_DIR}/wreck)
file(MAKE_DIRECTORY ${GEN_DIR}/wreck_2)

extractBitmaps(TARGET ${GAME_EXECUTABLE} SOURCE ${RES_DIR}/krtk.png
	GENERATED_FILE_LIST "FRAMES_WRECK_P1"
	DESTINATIONS
		${GEN_DIR}/wreck/0.png 0 109 48 21
		${GEN_DIR}/wreck/1.png 86 109 48 21
)

convertTileset(
	TARGET ${GAME_EXECUTABLE} SIZE 48 HEIGHT 21
	SOURCE ${GEN_DIR}/wreck DESTINATION ${GEN_DIR}/wreck.png
	TILE_PATHS ${FRAMES_WRECK_P1}
)

extractBitmaps(TARGET ${GAME_EXECUTABLE} SOURCE ${RES_DIR}/krtk.png
	GENERATED_FILE_LIST "FRAMES_WRECK_P2"
	DESTINATIONS
		${GEN_DIR}/wreck_2/0.png 0 269 48 21
		${GEN_DIR}/wreck_2/1.png 86 269 48 21
)

convertTileset(
	TARGET ${GAME_EXECUTABLE} SIZE 48 HEIGHT 21
	SOURCE ${GEN_DIR}/wreck_2 DESTINATION ${GEN_DIR}/wreck_2.png
	TILE_PATHS ${FRAMES_WRECK_P2}
)

#-------------------------------------------------------------------------- TOOL
file(MAKE_DIRECTORY ${GEN_DIR}/tool)
file(MAKE_DIRECTORY ${GEN_DIR}/tool_2)

extractBitmaps(TARGET ${GAME_EXECUTABLE} SOURCE ${RES_DIR}/krtk.png
	GENERATED_FILE_LIST "FRAMES_TOOL_P1"
	DESTINATIONS
		# right
		${GEN_DIR}/tool/0.png 33 64 16 16
		${GEN_DIR}/tool/1.png 33 80 16 16
		${GEN_DIR}/tool/2.png 33 32 16 16
		${GEN_DIR}/tool/3.png 33 48 16 16
		${GEN_DIR}/tool/4.png 33 0 16 16
		${GEN_DIR}/tool/5.png 33 16 16 16
		${GEN_DIR}/tool/6.png 160 96 16 16
		${GEN_DIR}/tool/7.png 160 112 16 16
		# left
		${GEN_DIR}/tool/8.png  79 64 16 16
		${GEN_DIR}/tool/9.png  79 80 16 16
		${GEN_DIR}/tool/10.png 79 32 16 16
		${GEN_DIR}/tool/11.png 79 48 16 16
		${GEN_DIR}/tool/12.png 79 0 16 16
		${GEN_DIR}/tool/13.png 79 16 16 16
		${GEN_DIR}/tool/14.png 208 96 16 16
		${GEN_DIR}/tool/15.png 208 112 16 16
)

convertTileset(
	TARGET ${GAME_EXECUTABLE} SIZE 16
	SOURCE ${GEN_DIR}/tool DESTINATION ${GEN_DIR}/tool.png
	TILE_PATHS ${FRAMES_TOOL_P1}
)

extractBitmaps(TARGET ${GAME_EXECUTABLE} SOURCE ${RES_DIR}/krtk.png
	GENERATED_FILE_LIST "FRAMES_TOOL_P2"
	DESTINATIONS
		# right
		${GEN_DIR}/tool_2/0.png 33 224 16 16
		${GEN_DIR}/tool_2/1.png 33 240 16 16
		${GEN_DIR}/tool_2/2.png 33 192 16 16
		${GEN_DIR}/tool_2/3.png 33 208 16 16
		${GEN_DIR}/tool_2/4.png 33 160 16 16
		${GEN_DIR}/tool_2/5.png 33 176 16 16
		${GEN_DIR}/tool_2/6.png 256 96 16 16
		${GEN_DIR}/tool_2/7.png 256 112 16 16
		# left
		${GEN_DIR}/tool_2/8.png  79 224 16 16
		${GEN_DIR}/tool_2/9.png  79 240 16 16
		${GEN_DIR}/tool_2/10.png 79 192 16 16
		${GEN_DIR}/tool_2/11.png 79 208 16 16
		${GEN_DIR}/tool_2/12.png 79 160 16 16
		${GEN_DIR}/tool_2/13.png 79 176 16 16
		${GEN_DIR}/tool_2/14.png 304 96 16 16
		${GEN_DIR}/tool_2/15.png 304 112 16 16
)

convertTileset(
	TARGET ${GAME_EXECUTABLE} SIZE 16
	SOURCE ${GEN_DIR}/tool_2 DESTINATION ${GEN_DIR}/tool_2.png
	TILE_PATHS ${FRAMES_TOOL_P2}
)

#------------------------------------------------------------------------- TRACK
file(MAKE_DIRECTORY ${GEN_DIR}/track)

extractBitmaps(TARGET ${GAME_EXECUTABLE} SOURCE ${RES_DIR}/krtk.png
	GENERATED_FILE_LIST "FRAMES_TRACK"
	DESTINATIONS
		# right
		${GEN_DIR}/track/0.png 159 34 32 7
		${GEN_DIR}/track/1.png 159 41 32 7
		${GEN_DIR}/track/2.png 159 48 32 7
		${GEN_DIR}/track/3.png 159 55 32 7
		${GEN_DIR}/track/4.png 159 79 32 7
		${GEN_DIR}/track/5.png 159 61 32 7
		${GEN_DIR}/track/6.png 159 69 32 8
)

convertTileset(
	TARGET ${GAME_EXECUTABLE} VARIABLE_HEIGHT SIZE 32
	SOURCE ${GEN_DIR}/track DESTINATION ${GEN_DIR}/track.png
	TILE_PATHS ${FRAMES_TRACK}
)

#------------------------------------------------------------------------- BONES
file(MAKE_DIRECTORY ${GEN_DIR}/bones)

extractBitmaps(TARGET ${GAME_EXECUTABLE} SOURCE ${RES_DIR}/minerals.png
	GENERATED_FILE_LIST "DINO_FRAGS"
	DESTINATIONS
		${GEN_DIR}/bones/0.png 0 384 80 22 # true width: 32
		${GEN_DIR}/bones/1.png 0 408 80 10 # true width: 32
		${GEN_DIR}/bones/2.png 0 419 80 15 # true width: 32
		${GEN_DIR}/bones/3.png 0 435 80 24 # true width: 32
		${GEN_DIR}/bones/4.png 0 460 80 44 # true width: 48
		${GEN_DIR}/bones/5.png 0 505 80 29 # true width: 48
		${GEN_DIR}/bones/6.png 0 535 80 45 # true width: 16
		${GEN_DIR}/bones/7.png 0 581 80 45 # true width: 32
		${GEN_DIR}/bones/8.png 0 627 80 22 # true width: 80
)

convertTileset(
	TARGET ${GAME_EXECUTABLE} SIZE 80 VARIABLE_HEIGHT PALETTE ${palette_aminer_unique}
	INTERLEAVED SOURCE ${GEN_DIR}/bones DESTINATION ${GEN_DIR}/bones.png
	TILE_PATHS ${DINO_FRAGS}
)

#-------------------------------------------------------------------------- GATE
file(MAKE_DIRECTORY ${GEN_DIR}/gate)

extractBitmaps(TARGET ${GAME_EXECUTABLE} SOURCE ${RES_DIR}/minerals.png
	GENERATED_FILE_LIST "GATE_FRAGS"
	DESTINATIONS
		${GEN_DIR}/gate/0.png  0  649 32 10
		${GEN_DIR}/gate/1.png  0  659 32 17
		${GEN_DIR}/gate/2.png  0  676 32 20
		${GEN_DIR}/gate/3.png  0  696 32 18
		${GEN_DIR}/gate/4.png  0  714 32 18
		${GEN_DIR}/gate/5.png  0  732 32 19
		${GEN_DIR}/gate/6.png  0  751 32 17
		${GEN_DIR}/gate/7.png  0  768 32 14
		${GEN_DIR}/gate/8.png  0  782 32 13
		${GEN_DIR}/gate/9.png  0  795 32 17
		${GEN_DIR}/gate/10.png 0  812 32 19
		${GEN_DIR}/gate/11.png 0  831 32 18
		${GEN_DIR}/gate/12.png 0  849 32 18
		${GEN_DIR}/gate/13.png 0  867 32 19
		${GEN_DIR}/gate/14.png 0  886 32 17
		${GEN_DIR}/gate/15.png 0  903 32 11
)

convertTileset(
	TARGET ${GAME_EXECUTABLE} SIZE 32 VARIABLE_HEIGHT PALETTE ${palette_aminer_unique}
	INTERLEAVED SOURCE ${GEN_DIR}/gate DESTINATION ${GEN_DIR}/gate.png
	TILE_PATHS ${GATE_FRAGS}
)

#---------------------------------------------------------------------- TILESETS

include("base0.cmake")
include("base1.cmake")
include("base2.cmake")
include("tiles.cmake")
include("checkpoint.cmake")

#-------------------------------------------------------- CONVERT TO ACE FORMATS

convertBitmaps(
	TARGET ${GAME_EXECUTABLE} PALETTE ${palette_aminer_unique}
	INTERLEAVED SOURCES
		${RES_DIR}/commrade/bg.png ${RES_DIR}/commrade/buttons.png ${RES_DIR}/commrade/faces_hud.png
		${RES_DIR}/hud/hud.png ${RES_DIR}/hud/mode_icons.png ${RES_DIR}/logo.png ${RES_DIR}/lang_select.png
		${RES_DIR}/commrade/faces_office.png ${RES_DIR}/commrade/office_selection.png ${RES_DIR}/commrade/news.png
	DESTINATIONS
		${DATA_DIR}/comm_bg.bm ${DATA_DIR}/comm_buttons.bm ${DATA_DIR}/comm_faces.bm
		${DATA_DIR}/hud.bm ${DATA_DIR}/mode_icons.bm ${DATA_DIR}/logo.bm ${DATA_DIR}/lang_select.bm
		${DATA_DIR}/comm_faces_office.bm ${DATA_DIR}/comm_office_selection.bm ${DATA_DIR}/comm_news.bm
)

convertBitmaps(
	TARGET ${GAME_EXECUTABLE} PALETTE ${palette_lmc}
	INTERLEAVED SOURCES ${RES_DIR}/lmc.png DESTINATIONS ${DATA_DIR}/lmc.bm
)

convertBitmaps(
	TARGET ${GAME_EXECUTABLE} PALETTE ${palette_aminer_unique} MASK_COLOR ${transparency_hex}
	INTERLEAVED SOURCES
		${GEN_DIR}/drill.png ${GEN_DIR}/tool.png ${GEN_DIR}/wreck.png
		${GEN_DIR}/drill_2.png ${GEN_DIR}/tool_2.png ${GEN_DIR}/wreck_2.png
		${GEN_DIR}/track.png ${RES_DIR}/jet.png ${GEN_DIR}/bones.png
		${GEN_DIR}/gate.png ${RES_DIR}/smoke.png
		${RES_DIR}/explosion.png ${RES_DIR}/teleport.png ${RES_DIR}/hud/mode_cursor.png
		${RES_DIR}/bomb_marker.png ${RES_DIR}/commrade/edges.png
	DESTINATIONS
		${DATA_DIR}/drill.bm ${DATA_DIR}/tool.bm ${DATA_DIR}/wreck.bm
		${DATA_DIR}/drill_2.bm ${DATA_DIR}/tool_2.bm ${DATA_DIR}/wreck_2.bm
		${DATA_DIR}/track.bm ${DATA_DIR}/jet.bm ${DATA_DIR}/bones.bm
		${DATA_DIR}/gate.bm ${DATA_DIR}/smoke.bm
		${DATA_DIR}/explosion.bm ${DATA_DIR}/teleport.bm ${DATA_DIR}/mode_cursor.bm
		${DATA_DIR}/bomb_marker.bm NONE
	MASKS
		${DATA_DIR}/drill_mask.bm ${DATA_DIR}/tool_mask.bm ${DATA_DIR}/wreck_mask.bm
		NONE NONE NONE
		${DATA_DIR}/track_mask.bm ${DATA_DIR}/jet_mask.bm ${DATA_DIR}/bones_mask.bm
		${DATA_DIR}/gate_mask.bm ${DATA_DIR}/smoke_mask.bm
		${DATA_DIR}/explosion_mask.bm ${DATA_DIR}/teleport_mask.bm ${DATA_DIR}/mode_cursor_mask.bm
		${DATA_DIR}/bomb_marker_mask.bm ${DATA_DIR}/comm_edges_mask.bm
)

convertFont(
	TARGET ${GAME_EXECUTABLE}
	SOURCE ${RES_DIR}/uni54 DESTINATION ${DATA_DIR}/uni54.fnt
)

#--------------------------------------------------------------------------- SFX
file(MAKE_DIRECTORY ${DATA_DIR}/sfx)

convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/atari.wav
	DESTINATION ${DATA_DIR}/sfx/atari.sfx
)

convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/drill1.wav
	DESTINATION ${DATA_DIR}/sfx/drill1.sfx
)

convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/ore2.wav
	DESTINATION ${DATA_DIR}/sfx/ore2.sfx
)

convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/penalty.wav
	DESTINATION ${DATA_DIR}/sfx/penalty.sfx
)

convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/explosion.wav
	DESTINATION ${DATA_DIR}/sfx/explosion.sfx
)

convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/teleport.wav
	DESTINATION ${DATA_DIR}/sfx/teleport.sfx
)

convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/key_press_0.wav
	DESTINATION ${DATA_DIR}/sfx/key_press_0.sfx
)
convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/key_press_1.wav
	DESTINATION ${DATA_DIR}/sfx/key_press_1.sfx
)
convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/key_press_2.wav
	DESTINATION ${DATA_DIR}/sfx/key_press_2.sfx
)
convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/key_press_3.wav
	DESTINATION ${DATA_DIR}/sfx/key_press_3.sfx
)

convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/key_release_0.wav
	DESTINATION ${DATA_DIR}/sfx/key_release_0.sfx
)
convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/key_release_1.wav
	DESTINATION ${DATA_DIR}/sfx/key_release_1.sfx
)
convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/key_release_2.wav
	DESTINATION ${DATA_DIR}/sfx/key_release_2.sfx
)
convertAudio(
	TARGET ${GAME_EXECUTABLE}
	STRICT SOURCE ${RES_DIR}/audio/key_release_3.wav
	DESTINATION ${DATA_DIR}/sfx/key_release_3.sfx
)

#------------------------------------------------------------------------- MUSIC
file(MAKE_DIRECTORY ${DATA_DIR}/music)

mergeMods(
	TARGET ${GAME_EXECUTABLE}
	SOURCES
		${RES_DIR}/music/czolowka.mod
		${RES_DIR}/music/coal-prelude.mod
		${RES_DIR}/music/dumka.mod
		${RES_DIR}/music/inzynier.mod
		${RES_DIR}/music/piesnoatari.mod
		${RES_DIR}/music/popescu.mod
		${RES_DIR}/music/roboczy.mod
		${RES_DIR}/music/tancjor1.mod
		${RES_DIR}/music/tancjordrugoj.mod
		${RES_DIR}/music/wintowka.mod
		${RES_DIR}/music/zima.mod
	DESTINATIONS
		${DATA_DIR}/music/menu.mod
		${DATA_DIR}/music/game0.mod
		${DATA_DIR}/music/game1.mod
		${DATA_DIR}/music/game2.mod
		${DATA_DIR}/music/game3.mod
		${DATA_DIR}/music/game4.mod
		${DATA_DIR}/music/game5.mod
		${DATA_DIR}/music/game6.mod
		${DATA_DIR}/music/game7.mod
		${DATA_DIR}/music/game8.mod
		${DATA_DIR}/music/game9.mod
	SAMPLE_PACK ${DATA_DIR}/music/samples.samplepack
)

# Version stuff
string(TIMESTAMP YEAR "%y")
string(TIMESTAMP DAY "%d")
string(TIMESTAMP MONTH "%m")
MATH(EXPR VER_MAJOR "0 + ${YEAR}")
MATH(EXPR VER_MINOR "0 + ${MONTH}")
MATH(EXPR VER_FIX "0 + ${DAY}")
set(VERSION "${VER_MAJOR}.${VER_MINOR}.${VER_FIX}")
target_compile_definitions(${GAME_EXECUTABLE} PRIVATE GAME_VERSION="${VERSION}")

# Generating ZIP
set(GAME_PACKAGE_NAME "${CMAKE_PROJECT_NAME}_${VER_MAJOR}_${VER_MINOR}_${VER_FIX}")
add_custom_target(generateZip COMMAND
	${CMAKE_COMMAND} -E tar "cf" "${GAME_PACKAGE_NAME}.zip" --format=zip
	"${CMAKE_CURRENT_BINARY_DIR}/${GAME_OUTPUT_EXECUTABLE}"
	"${CMAKE_CURRENT_BINARY_DIR}/${GAME_OUTPUT_EXECUTABLE}.info"
	"${DATA_DIR}"
	COMMENT "Generating ZIP file ${GAME_PACKAGE_NAME}.zip"
)

# Generating ADF
set(ADF_DIR "${CMAKE_CURRENT_BINARY_DIR}/adf")
add_custom_target(generateAdf
	COMMAND ${CMAKE_COMMAND} -E make_directory "${ADF_DIR}/s"
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${GAME_OUTPUT_EXECUTABLE}" "${ADF_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${GAME_OUTPUT_EXECUTABLE}.info" "${ADF_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${DATA_DIR}" "${ADF_DIR}/data"
	# COMMAND ${CMAKE_COMMAND} -E echo "c:add21k" > "${ADF_DIR}/s/startup-sequence"
	COMMAND ${CMAKE_COMMAND} -E echo "${GAME_OUTPUT_EXECUTABLE}" > "${ADF_DIR}/s/startup-sequence"
	COMMAND exe2adf -l ${CMAKE_PROJECT_NAME} -a "${GAME_PACKAGE_NAME}.adf" -d ${ADF_DIR}
	COMMAND ${CMAKE_COMMAND} -E rm -rf "${ADF_DIR}"
	COMMENT "Generating ADF file ${GAME_PACKAGE_NAME}.adf"
)
