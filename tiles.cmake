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
set(TILE_61 198 99 "dirt.png") # TILE_STONE_2
set(TILE_62 198 132 "dirt.png") # TILE_STONE_2
set(TILE_63 0 0 "dirt.png") # TILE_ROCK_1
set(TILE_64 33 0 "dirt.png") # TILE_ROCK_2
set(TILE_65 1 1 "minerals.png") # TILE_GOLD_1
set(TILE_66 1 34 "minerals.png") # TILE_GOLD_2
set(TILE_67 1 67 "minerals.png") # TILE_GOLD_3
set(TILE_68 34 1 "minerals.png") # TILE_SILVER_1
set(TILE_69 34 34 "minerals.png") # TILE_SILVER_2
set(TILE_70 34 67 "minerals.png") # TILE_SILVER_3
set(TILE_71 67 232 "minerals.png") # TILE_EMERALD_1
set(TILE_72 34 232 "minerals.png") # TILE_EMERALD_2
set(TILE_73 1 232 "minerals.png") # TILE_EMERALD_3
set(TILE_74 67 199 "minerals.png") # TILE_RUBY_1
set(TILE_75 34 199 "minerals.png") # TILE_RUBY_2
set(TILE_76 1 199 "minerals.png") # TILE_RUBY_3
set(TILE_77 67 265 "minerals.png") # TILE_MOONSTONE_1
set(TILE_78 34 265 "minerals.png") # TILE_MOONSTONE_2
set(TILE_79 1 265 "minerals.png") # TILE_MOONSTONE_3
set(TILE_80 1 298 "minerals.png") # TILE_COAL_1
set(TILE_81 34 298 "minerals.png") # TILE_COAL_2
set(TILE_82 67 298 "minerals.png") # TILE_COAL_3
set(TILE_83 0 0 "checkpoint.png") # TILE_CHECKPOINT_1
set(TILE_84 32 0 "checkpoint.png") # TILE_CHECKPOINT_2
set(TILE_85 64 0 "checkpoint.png") # TILE_CHECKPOINT_3
set(TILE_86 96 0 "checkpoint.png") # TILE_CHECKPOINT_4
set(TILE_87 128 0 "checkpoint.png") # TILE_CHECKPOINT_5
set(TILE_88 160 0 "checkpoint.png") # TILE_CHECKPOINT_6
set(TILE_89 192 0 "checkpoint.png") # TILE_CHECKPOINT_7
set(TILE_90 224 0 "checkpoint.png") # TILE_CHECKPOINT_8
set(TILE_91 256 0 "checkpoint.png") # TILE_CHECKPOINT_9
set(TILE_92 288 0 "checkpoint.png") # TILE_CHECKPOINT_10
set(TILE_93 33 331 "minerals.png") # TILE_BONE_HEAD
set(TILE_94 1 331 "minerals.png") # TILE_BONE_1

tileExtractFromPng(
	${TARGET_NAME} "TILE"
	"${CMAKE_CURRENT_SOURCE_DIR}/_res/tiles"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/tiles.bm"
)
