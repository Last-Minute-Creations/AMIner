set(BASE1_0 96 0 "base1.png") # TILE_BASE_BG_FIRST
set(BASE1_1 256 0 "base1.png")
set(BASE1_2 288 64 "base1.png")
set(BASE1_3 160 96 "base1.png")
set(BASE1_4 128 128 "base1.png")
set(BASE1_5 64 160 "base1.png")
set(BASE1_6 96 160 "base1.png")
set(BASE1_7 128 160 "base1.png")
set(BASE1_8 160 160 "base1.png")
set(BASE1_9 192 160 "base1.png")
set(BASE1_10 224 160 "base1.png")
set(BASE1_11 256 160 "base1.png")
set(BASE1_12 288 160 "base1.png")

# TILE_BASE_BG_FIRST + 13 .. 22
set(TILE_IDX 13)
set(TILE_x 0)
while(TILE_IDX LESS "23")
	set(BASE1_${TILE_IDX} ${TILE_x} 192 "base1.png")
	math(EXPR TILE_x "${TILE_x} + 32")
	math(EXPR TILE_IDX "${TILE_IDX} + 1")
endwhile()

# TILE_BASE_BG_FIRST + 23 .. 32
set(TILE_IDX 23)
set(TILE_x 0)
while(TILE_IDX LESS "33")
	set(BASE1_${TILE_IDX} ${TILE_x} 224 "base1.png")
	math(EXPR TILE_IDX "${TILE_IDX} + 1")
	math(EXPR TILE_x "${TILE_x} + 32")
endwhile()

# Rest of base tiles
set(BASE1_34 0 256 "base1.png") # TILE_BASE_GROUND_1
set(BASE1_35 32 256 "base1.png") # TILE_BASE_SHAFT
set(BASE1_36 64 256 "base1.png") # TILE_BASE_GROUND_2
set(BASE1_37 96 256 "base1.png") # TILE_BASE_GROUND_3
set(BASE1_38 128 256 "base1.png") # TILE_BASE_GROUND_4
set(BASE1_33 160 256 "base1.png") # TILE_BASE_GROUND_5
set(BASE1_39 192 256 "base1.png") # TILE_BASE_GROUND_6
set(BASE1_40 224 256 "base1.png") # TILE_BASE_GROUND_7
set(BASE1_41 256 256 "base1.png") # TILE_BASE_GROUND_8
set(BASE1_42 288 256 "base1.png") # TILE_BASE_GROUND_9

tileExtractFromPng(
	${AMINER_EXECUTABLE} "BASE1" "${GEN_DIR}/base1" "${DATA_DIR}/base1.bm"
)
