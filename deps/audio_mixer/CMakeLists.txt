cmake_minimum_required(VERSION 3.14.0)
project(ace_audio_mixer VERSION 3.1)

if(NOT VASM_PATH)
	message(FATAL_ERROR "VASM_PATH is not set!")
endif()

set(MIXER_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/audio_mixer.o)
file(GLOB MIXER_SOURCE src/mixer.asm)
file(GLOB MIXER_DEPENDS src/mixer.asm src/*.i)

add_custom_command(OUTPUT ${MIXER_OUTPUT}
	COMMAND ${VASM_PATH} -Felf ${MIXER_SOURCE} -I ${TOOLCHAIN_PATH}/m68k-amiga-elf/sys-include -o ${MIXER_OUTPUT}
	DEPENDS ${MIXER_DEPENDS}
	COMMENT "Building ${MIXER_OUTPUT}"
)
add_custom_target(mixer_obj DEPENDS ${MIXER_OUTPUT})

# TODO: configure mixer_config.i
# - MIXER_SINGLE
# - MIXER_WORDSIZED
# - mixer_output_channels - needed for installing handler
# - mixer_sw_channels
# - mixer_period
# TODO: propagate those switches as defines for mixer_control.h

add_library(ace_audio_mixer OBJECT IMPORTED GLOBAL)
set_property(TARGET ace_audio_mixer PROPERTY IMPORTED_OBJECTS ${MIXER_OUTPUT})
target_include_directories(ace_audio_mixer INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_definitions(ace_audio_mixer INTERFACE MIXER_SINGLE)
add_dependencies(ace_audio_mixer mixer_obj)
