cmake_minimum_required(VERSION 3.14.0)
project(ace_audio_mixer VERSION 3.1)

if(NOT VASM_PATH)
	message(FATAL_ERROR "VASM_PATH is not set!")
endif()

set(MIXER_OBJ_FILE ${CMAKE_CURRENT_BINARY_DIR}/audio_mixer.o)
file(GLOB MIXER_SOURCE src/mixer.asm)
file(GLOB MIXER_DEPENDS src/mixer.asm src/mixer.i src/mixer_config.i)

add_custom_command(OUTPUT ${MIXER_OBJ_FILE}
	COMMAND ${VASM_PATH} -Felf ${MIXER_SOURCE} -I ${TOOLCHAIN_PATH}/m68k-amiga-elf/sys-include -o ${MIXER_OBJ_FILE}
	DEPENDS ${MIXER_DEPENDS}
	COMMENT "Building ${MIXER_OBJ_FILE}"
)
add_custom_target(mixer_obj DEPENDS ${MIXER_OBJ_FILE})

# TODO: configure mixer_config.i
# - MIXER_SINGLE
# - MIXER_WORDSIZED
# - mixer_OBJ_FILE_channels - needed for installing handler
# - mixer_sw_channels
# - mixer_period
# TODO: propagate those switches as defines for mixer_control.h

file(GLOB_RECURSE ACE_MIXER_SOURCES src/ace/*.c inc/ace/*.h)
add_library(ace_audio_mixer OBJECT ${ACE_MIXER_SOURCES})
target_include_directories(ace_audio_mixer PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(ace_audio_mixer PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(ace_audio_mixer PUBLIC ${MIXER_OBJ_FILE} ace)
