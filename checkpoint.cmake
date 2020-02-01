# Checkpoint line
set(CHECKPOINT_0 0 0 "checkpoint.png") # TILE_CHECKPOINT_1, TODO: Proper checkpoint
set(CHECKPOINT_1 32 0 "checkpoint.png") # TILE_CHECKPOINT_2
set(CHECKPOINT_2 64 0 "checkpoint.png") # TILE_CHECKPOINT_3
set(CHECKPOINT_3 96 0 "checkpoint.png") # TILE_CHECKPOINT_4
set(CHECKPOINT_4 128 0 "checkpoint.png") # TILE_CHECKPOINT_5
set(CHECKPOINT_5 160 0 "checkpoint.png") # TILE_CHECKPOINT_6
set(CHECKPOINT_6 192 0 "checkpoint.png") # TILE_CHECKPOINT_7
set(CHECKPOINT_7 224 0 "checkpoint.png") # TILE_CHECKPOINT_8
set(CHECKPOINT_8 256 0 "checkpoint.png") # TILE_CHECKPOINT_9
set(CHECKPOINT_9 288 0 "checkpoint.png") # TILE_CHECKPOINT_10

# Finish line
set(CHECKPOINT_10 0 64 "checkpoint.png") # TILE_CHECKPOINT_1,
set(CHECKPOINT_11 32 64 "checkpoint.png") # TILE_CHECKPOINT_2
set(CHECKPOINT_12 64 64 "checkpoint.png") # TILE_CHECKPOINT_3
set(CHECKPOINT_13 96 64 "checkpoint.png") # TILE_CHECKPOINT_4
set(CHECKPOINT_14 128 64 "checkpoint.png") # TILE_CHECKPOINT_5
set(CHECKPOINT_15 160 64 "checkpoint.png") # TILE_CHECKPOINT_6
set(CHECKPOINT_16 192 64 "checkpoint.png") # TILE_CHECKPOINT_7
set(CHECKPOINT_17 224 64 "checkpoint.png") # TILE_CHECKPOINT_8
set(CHECKPOINT_18 256 64 "checkpoint.png") # TILE_CHECKPOINT_9
set(CHECKPOINT_19 288 64 "checkpoint.png") # TILE_CHECKPOINT_10

tileExtractFromPng(
	aminer "CHECKPOINT"
	"${CMAKE_CURRENT_SOURCE_DIR}/_res/checkpoint"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/checkpoint.bm"
)
