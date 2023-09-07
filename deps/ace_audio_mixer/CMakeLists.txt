cmake_minimum_required(VERSION 3.14.0)
project(ace_audio_mixer VERSION 3.1 LANGUAGES C ASM_VASM)

include(mixer_config.cmake)
file(GLOB_RECURSE ACE_MIXER_SOURCES src/ace/*.c inc/ace/*.h src/mixer.asm)
add_library(ace_audio_mixer OBJECT ${ACE_MIXER_SOURCES})
target_include_directories(ace_audio_mixer PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(ace_audio_mixer PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ace_audio_mixer PUBLIC ace)

target_sources(ace_audio_mixer INTERFACE ${MIXER_OBJ_FILE})
