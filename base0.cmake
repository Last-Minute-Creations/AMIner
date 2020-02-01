set(BASE0_0 0 0 "base0.png") # TILE_BASE_BG_FIRST
set(BASE0_1 0 32 "base0.png")
set(BASE0_2 0 64 "base0.png")
set(BASE0_3 0 96 "base0.png")
set(BASE0_4 0 128 "base0.png")
set(BASE0_5 288 128 "base0.png")
set(BASE0_6 0 160 "base0.png")
set(BASE0_7 64 160 "base0.png")
set(BASE0_8 160 160 "base0.png")
set(BASE0_9 192 160 "base0.png")
set(BASE0_10 224 160 "base0.png")
set(BASE0_11 256 160 "base0.png")
set(BASE0_12 288 160 "base0.png")

# TILE_BASE_BG_FIRST + 13 .. 22
set(TILE_IDX 13)
set(TILE_x 0)
while(TILE_IDX LESS "23")
	set(BASE0_${TILE_IDX} ${TILE_x} 192 "base0.png")
	math(EXPR TILE_x "${TILE_x} + 32")
	math(EXPR TILE_IDX "${TILE_IDX} + 1")
endwhile()

# TILE_BASE_BG_FIRST + 23 .. 32
set(TILE_IDX 23)
set(TILE_x 0)
while(TILE_IDX LESS "33")
	set(BASE0_${TILE_IDX} ${TILE_x} 224 "base0.png")
	math(EXPR TILE_IDX "${TILE_IDX} + 1")
	math(EXPR TILE_x "${TILE_x} + 32")
endwhile()

# Rest of base tiles
set(BASE0_33 32 256 "base0.png") # TILE_BASE_SHAFT
set(BASE0_34 0 256 "base0.png") # TILE_BASE_GROUND_1
set(BASE0_35 64 256 "base0.png") # TILE_BASE_GROUND_2
set(BASE0_36 96 256 "base0.png") # TILE_BASE_GROUND_3
set(BASE0_37 128 256 "base0.png") # TILE_BASE_GROUND_4
set(BASE0_38 160 256 "base0.png") # TILE_BASE_GROUND_5
set(BASE0_39 192 256 "base0.png") # TILE_BASE_GROUND_6
set(BASE0_40 224 256 "base0.png") # TILE_BASE_GROUND_7
set(BASE0_41 256 256 "base0.png") # TILE_BASE_GROUND_8
set(BASE0_42 288 256 "base0.png") # TILE_BASE_GROUND_9

tileExtractFromPng(
	aminer "BASE0"
	"${CMAKE_CURRENT_SOURCE_DIR}/_res/base0"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/base0.bm"
)
