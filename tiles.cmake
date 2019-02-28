set(TILE_43 66 0 "dirt.png") # TILE_CAVE_BG
# TILE_CAVE_BG with edges:
	set(TILE_44 66 66 "dirt.png")  # N
	set(TILE_45 0 99 "dirt.png")   # S
	set(TILE_46 33 66 "dirt.png")  # NS
	set(TILE_47 99 66 "dirt.png")  # E
	set(TILE_48 66 99 "dirt.png")  # NE
	set(TILE_49 99 99 "dirt.png")  # SE
	set(TILE_50 0 33 "dirt.png")   # NSE
	set(TILE_51 33 99 "dirt.png")  # W
	set(TILE_52 33 132 "dirt.png") # NW
	set(TILE_53 0 132 "dirt.png")  # SW
	set(TILE_54 99 33 "dirt.png")  # NSW
	set(TILE_55 0 66 "dirt.png")   # EW
	set(TILE_56 66 33 "dirt.png")  # NEW
	set(TILE_57 33 33 "dirt.png")  # SEW
	set(TILE_58 99 0 "dirt.png")   # NSEW
set(TILE_59 198 33 "dirt.png") # TILE_STONE_1
set(TILE_60 198 66 "dirt.png") # TILE_STONE_2
set(TILE_61 0 0 "dirt.png") # TILE_ROCK_1
set(TILE_62 33 0 "dirt.png") # TILE_ROCK_2
set(TILE_63 1 1 "minerals.png") # TILE_GOLD_1
set(TILE_64 1 34 "minerals.png") # TILE_GOLD_2
set(TILE_65 1 67 "minerals.png") # TILE_GOLD_3
set(TILE_66 34 1 "minerals.png") # TILE_SILVER_1
set(TILE_67 34 34 "minerals.png") # TILE_SILVER_2
set(TILE_68 34 67 "minerals.png") # TILE_SILVER_3
set(TILE_69 67 232 "minerals.png") # TILE_EMERALD_1
set(TILE_70 34 232 "minerals.png") # TILE_EMERALD_2
set(TILE_71 1 232 "minerals.png") # TILE_EMERALD_3
set(TILE_72 67 199 "minerals.png") # TILE_RUBY_1
set(TILE_73 34 199 "minerals.png") # TILE_RUBY_2
set(TILE_74 1 199 "minerals.png") # TILE_RUBY_3
set(TILE_75 67 265 "minerals.png") # TILE_MOONSTONE_1
set(TILE_76 34 265 "minerals.png") # TILE_MOONSTONE_2
set(TILE_77 1 265 "minerals.png") # TILE_MOONSTONE_3
set(TILE_78 1 298 "minerals.png") # TILE_COAL_1
set(TILE_79 34 298 "minerals.png") # TILE_COAL_2
set(TILE_80 67 298 "minerals.png") # TILE_COAL_3
set(TILE_81 67 298 "minerals.png") # TILE_COAL_3, TODO: Proper checkpoint
set(TILE_82 67 298 "minerals.png") # TILE_COAL_3, TODO: Proper finish

tileExtractFromPng(
	${TARGET_NAME} "TILE"
	"${CMAKE_CURRENT_SOURCE_DIR}/_res/tiles"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/tiles.bm"
)
