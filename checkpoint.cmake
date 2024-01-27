file(MAKE_DIRECTORY ${GEN_DIR}/checkpoint)

extractBitmaps(TARGET ${GAME_EXECUTABLE} SOURCE ${RES_DIR}/checkpoint.png
	GENERATED_FILE_LIST "TILES_CHECKPOINT_LIST"
	DESTINATIONS
	# Checkpoint line
	${GEN_DIR}/checkpoint/0.png   0 0 32 32 # TILE_CHECKPOINT_1, TODO: Proper checkpoint
	${GEN_DIR}/checkpoint/1.png  32 0 32 32 # TILE_CHECKPOINT_2
	${GEN_DIR}/checkpoint/2.png  64 0 32 32 # TILE_CHECKPOINT_3
	${GEN_DIR}/checkpoint/3.png  96 0 32 32 # TILE_CHECKPOINT_4
	${GEN_DIR}/checkpoint/4.png 128 0 32 32 # TILE_CHECKPOINT_5
	${GEN_DIR}/checkpoint/5.png 160 0 32 32 # TILE_CHECKPOINT_6
	${GEN_DIR}/checkpoint/6.png 192 0 32 32 # TILE_CHECKPOINT_7
	${GEN_DIR}/checkpoint/7.png 224 0 32 32 # TILE_CHECKPOINT_8
	${GEN_DIR}/checkpoint/8.png 256 0 32 32 # TILE_CHECKPOINT_9
	${GEN_DIR}/checkpoint/9.png 288 0 32 32 # TILE_CHECKPOINT_10
	# Finish line
	${GEN_DIR}/checkpoint/10.png   0 64 32 32 # TILE_CHECKPOINT_1,
	${GEN_DIR}/checkpoint/11.png  32 64 32 32 # TILE_CHECKPOINT_2
	${GEN_DIR}/checkpoint/12.png  64 64 32 32 # TILE_CHECKPOINT_3
	${GEN_DIR}/checkpoint/13.png  96 64 32 32 # TILE_CHECKPOINT_4
	${GEN_DIR}/checkpoint/14.png 128 64 32 32 # TILE_CHECKPOINT_5
	${GEN_DIR}/checkpoint/15.png 160 64 32 32 # TILE_CHECKPOINT_6
	${GEN_DIR}/checkpoint/16.png 192 64 32 32 # TILE_CHECKPOINT_7
	${GEN_DIR}/checkpoint/17.png 224 64 32 32 # TILE_CHECKPOINT_8
	${GEN_DIR}/checkpoint/18.png 256 64 32 32 # TILE_CHECKPOINT_9
	${GEN_DIR}/checkpoint/19.png 288 64 32 32 # TILE_CHECKPOINT_10
)

convertTileset(
	TARGET ${GAME_EXECUTABLE} SIZE 32 PALETTE ${palette_aminer_unique}
	INTERLEAVED SOURCE ${GEN_DIR}/checkpoint DESTINATION ${DATA_DIR}/checkpoint.bm
	TILE_PATHS ${TILES_CHECKPOINT_LIST}
)

file(MAKE_DIRECTORY ${GEN_DIR}/prisoner)

extractBitmaps(TARGET ${GAME_EXECUTABLE} SOURCE ${RES_DIR}/prisoner.png
	GENERATED_FILE_LIST "TILES_CHECKPOINT_LIST"
	DESTINATIONS
	${GEN_DIR}/prisoner/0.png 0   0 32 32 # TILE_PRISONER_1,
	${GEN_DIR}/prisoner/1.png 0  32 32 32 # TILE_PRISONER_2
	${GEN_DIR}/prisoner/2.png 0  64 32 32 # TILE_PRISONER_3
	${GEN_DIR}/prisoner/3.png 0  96 32 32 # TILE_PRISONER_4
	${GEN_DIR}/prisoner/4.png 0 128 32 32 # TILE_PRISONER_5
	${GEN_DIR}/prisoner/5.png 0 160 32 32 # TILE_PRISONER_6
	${GEN_DIR}/prisoner/6.png 0 192 32 32 # TILE_PRISONER_7
	${GEN_DIR}/prisoner/7.png 0 224 32 32 # TILE_PRISONER_8
)

convertTileset(
	TARGET ${GAME_EXECUTABLE} SIZE 32 PALETTE ${palette_aminer_unique}
	INTERLEAVED SOURCE ${GEN_DIR}/prisoner DESTINATION ${DATA_DIR}/prisoner.bm
	TILE_PATHS ${TILES_CHECKPOINT_LIST}
)
