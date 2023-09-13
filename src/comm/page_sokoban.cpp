/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_sokoban.h"
#include <ace/types.h>
#include <comm/comm.h>

namespace {

//---------------------------------------------------------------- PRIVATE TYPES

struct tLevelData {
	enum class tTile: char {
		FLOOR = ' ',
		WALL = '#',
		SLOT = '.',
		BOX_ON_FLOOR = '$',
		BOX_ON_SLOT = '*',
		PLAYER_ON_FLOOR = '@',
		PLAYER_ON_SLOT = '+',
	};

	static constexpr UBYTE s_ubWidth = 8;
	static constexpr UBYTE s_ubHeight = 8;

	tTile m_pTiles[s_ubWidth][s_ubHeight]; ///< [x, y]

	constexpr tLevelData() = default;

	constexpr tLevelData(
		const char *pRow1, const char *pRow2, const char *pRow3, const char *pRow4,
		const char *pRow5, const char *pRow6, const char *pRow7, const char *pRow8
	);
};

class tLevelState {
public:
	constexpr void load(const tLevelData &rLevelData);

	constexpr const tLevelData &getData() const;

	constexpr bool isSolved() const;

	constexpr bool tryMovePlayer(tDirection eDirection);

	void drawAll(void);

	void drawDirty(void);

private:
	static constexpr UBYTE s_ubMaxBoxes = 5;
	static constexpr UBYTE s_ubMaxDirty = 4;

	tLevelData m_LevelData;
	tUbCoordYX m_PlayerPos;
	UBYTE m_ubSlotCount;
	UBYTE m_ubDirtyCount;
	tUbCoordYX m_pSlotPositions[s_ubMaxBoxes];
	tUbCoordYX m_pDirtyPositions[s_ubMaxDirty];

	void drawTileAt(UBYTE ubX, UBYTE ubY);
	constexpr bool tryMoveBox(tUbCoordYX BoxPos, tDirection eDirection);
};

//----------------------------------------------------------------- PRIVATE VARS

// http://sneezingtiger.com/sokoban/levels/yoshioText.html
const tLevelData s_pLevels[] = {
	tLevelData(
		" ###### ",
		"##  . # ",
		"# * # # ",
		"# .$  # ",
		"#  #$## ",
		"## @ #  ",
		" #####  ",
		"        "
	),
	tLevelData(
		"####### ",
		"#  .@ # ",
		"# #.# # ",
		"#   $ # ",
		"#.$$ ## ",
		"#  ###  ",
		"####    ",
		"        "
	),
	tLevelData(
		"   #### ",
		"#### @# ",
		"#  *$ # ",
		"#     # ",
		"## .### ",
		" #$ #   ",
		" # .#   ",
		" ####   "
	),
	tLevelData(
		"### ### ",
		"#.###.# ",
		"# #  .# ",
		"# $$ @# ",
		"#  $  # ",
		"#  #  # ",
		"#  #### ",
		"####    "
	),
	tLevelData(
		"   #### ",
		"   # @##",
		"####   #",
		"#. #$$ #",
		"#     ##",
		"#.  $## ",
		"##.  #  ",
		" #####  "
	),
	tLevelData(
		"        ",
		"#####   ",
		"# ..####",
		"# $    #",
		"#  #$# #",
		"# @ .$ #",
		"########",
		"        "
	),
	tLevelData(
		"  ##### ",
		"###  .# ",
		"# $ # # ",
		"# *$  # ",
		"# .#@ # ",
		"#    ## ",
		"#   ##  ",
		"#####   "
	),
	tLevelData(
		"####### ",
		"#.  @.# ",
		"#  $# ##",
		"# # $. #",
		"#   $# #",
		"####   #",
		"   #####",
		"        "
	),
	tLevelData(
		"#####   ",
		"#. .### ",
		"#.#$$ # ",
		"#   @ # ",
		"# $#  # ",
		"##   ## ",
		" #####  ",
		"        "
	),
	tLevelData(
		"#####   ",
		"#.  ### ",
		"# #   # ",
		"# . # # ",
		"# $*$ # ",
		"##@ ### ",
		" #  #   ",
		" ####   "
	),
	tLevelData(
		"########",
		"#.   . #",
		"# # #  #",
		"#@$  $.#",
		"##### $#",
		"    #  #",
		"    ####",
		"        "
	),
	tLevelData(
		"####    ",
		"#  #    ",
		"#  #####",
		"# .*   #",
		"##$    #",
		" # #$###",
		" #. @#  ",
		" #####  "
	),
	tLevelData(
		" #####  ",
		" # @ ###",
		"## .   #",
		"#. $.$ #",
		"##$# ###",
		" #   #  ",
		" #####  ",
		"        "
	),
	tLevelData(
		" #####  ",
		"##   #  ",
		"# $# #  ",
		"# . @## ",
		"# *   # ",
		"## #$ # ",
		" #.  ## ",
		" #####  "
	),
	tLevelData(
		" ####   ",
		"##  ####",
		"#..$  .#",
		"# #$ $ #",
		"#@  #  #",
		"#####  #",
		"    ####",
		"        "
	),
	tLevelData(
		" ###### ",
		" #  .@##",
		" #   $.#",
		" ###*# #",
		"##     #",
		"#  $  ##",
		"#   ### ",
		"#####   "
	),
	tLevelData(
		" ####   ",
		" #@ #   ",
		" #  #   ",
		"##. ####",
		"# $$. .#",
		"#  $ ###",
		"###  #  ",
		"  ####  "
	),
	tLevelData(
		"#####   ",
		"#.  #   ",
		"# # ### ",
		"# *$  # ",
		"#  $. # ",
		"#  @### ",
		"#####   ",
		"        "
	),
	tLevelData(
		"  ##### ",
		"  #   # ",
		"  # #.# ",
		"###  .# ",
		"#@ $$ # ",
		"#  .$ # ",
		"####### ",
		"        "
	),
	tLevelData(
		"######  ",
		"#   @#  ",
		"# $# ###",
		"# * $  #",
		"#   ## #",
		"##.  . #",
		" ##   ##",
		"  ##### "
	),
	tLevelData(
		"######  ",
		"#   @## ",
		"#  #  # ",
		"#.  $ # ",
		"# $$#.# ",
		"###  .# ",
		"  ##### ",
		"        "
	),
	tLevelData(
		"        ",
		"  ####  ",
		"###. #  ",
		"# .  ###",
		"#   $$ #",
		"## . $@#",
		" #######",
		"        "
	),
	tLevelData(
		" ###### ",
		"##@.  # ",
		"# $$* # ",
		"#  #  ##",
		"#  #  .#",
		"#### # #",
		"   #   #",
		"   #####"
	),
	tLevelData(
		"    ####",
		"    #  #",
		"  ###$.#",
		"  #  . #",
		"###  #.#",
		"# $  $ #",
		"#   #@ #",
		"########"
	),
	tLevelData(
		"#####   ",
		"#  .### ",
		"# $.. # ",
		"#  ##$##",
		"##  #  #",
		" #$   @#",
		" #  ####",
		" ####   "
	),
	tLevelData(
		"  ####  ",
		"  #  #  ",
		"  #  ###",
		"### .. #",
		"#  $#  #",
		"#  .$$ #",
		"#### @ #",
		"   #####"
	),
	tLevelData(
		"#####   ",
		"#   ### ",
		"# # *@##",
		"#  *   #",
		"###$   #",
		"  #   .#",
		"  ######",
		"        "
	),
	tLevelData(
		"  ######",
		"### .  #",
		"# $@#. #",
		"#  $# ##",
		"#  *  # ",
		"##  # # ",
		" ##   # ",
		"  ##### "
	),
	tLevelData(
		"        ",
		" ####   ",
		"##  ### ",
		"#     ##",
		"#  #$$@#",
		"#  . *.#",
		"########",
		"        "
	),
	tLevelData(
		" #######",
		"##@    #",
		"#. #   #",
		"# $$$.##",
		"# .#  # ",
		"#  #### ",
		"####    ",
		"        "
	),
	tLevelData(
		"########",
		"#      #",
		"# # ##*#",
		"# #@ $ #",
		"#.$ .  #",
		"#####  #",
		"    #  #",
		"    ####"
	),
	tLevelData(
		" ###### ",
		" #@   ##",
		" ##$   #",
		"### .  #",
		"# $ #$##",
		"# .  .# ",
		"####  # ",
		"   #### "
	),
	tLevelData(
		"#####   ",
		"#   ### ",
		"#  $  # ",
		"##$$ .# ",
		" #@ . # ",
		" ## # # ",
		"  #  .# ",
		"  ##### "
	),
	tLevelData(
		"#####   ",
		"#   ####",
		"# $$   #",
		"# .#.  #",
		"#  ## ##",
		"#  ##$# ",
		"# @  .# ",
		"####### "
	),
	tLevelData(
		"        ",
		"######  ",
		"# .  #  ",
		"# .# ###",
		"# @$$  #",
		"# $.   #",
		"########",
		"        "
	),
	tLevelData(
		"########",
		"# @.#  #",
		"# .$ . #",
		"#  #$  #",
		"#  $  ##",
		"###  ## ",
		"  #  #  ",
		"  ####  "
	),
	tLevelData(
		" #######",
		"##   . #",
		"# $  $@#",
		"#.$.####",
		"#  ##   ",
		"#  #    ",
		"#  #    ",
		"####    "
	),
	tLevelData(
		"######  ",
		"# .  #  ",
		"#  #@#  ",
		"#  $ ## ",
		"##$#  # ",
		"#   # # ",
		"#. *  # ",
		"####### "
	),
	tLevelData(
		"   #####",
		"#### . #",
		"# *@ . #",
		"# $ #  #",
		"# #  $ #",
		"#   ####",
		"#####   ",
		"        "
	),
	tLevelData(
		"        ",
		"  ####  ",
		"###  ###",
		"# .. $.#",
		"#  $$ @#",
		"####   #",
		"   #####",
		"        "
	),
	tLevelData(
		"    ####",
		"    #@ #",
		"##### .#",
		"# $ $ $#",
		"#   .  #",
		"### .  #",
		"  ######",
		"        "
	),
	tLevelData(
		"########",
		"#   #  #",
		"# #.$ $#",
		"#   $  #",
		"#####. #",
		"  #   @#",
		"  #   .#",
		"  ######"
	),
	tLevelData(
		"   #### ",
		"  ##@ ##",
		" ##  ..#",
		"## $#$##",
		"#   $. #",
		"#  #   #",
		"#    ###",
		"######  "
	),
	tLevelData(
		"######  ",
		"#   @#  ",
		"# $$####",
		"# $ .  #",
		"## #.# #",
		"#.   # #",
		"#      #",
		"########"
	),
	tLevelData(
		"   #### ",
		"   #  # ",
		"#### $##",
		"# @$.  #",
		"# ##   #",
		"#   ## #",
		"#   * .#",
		"########"
	),
	tLevelData(
		"   #####",
		"   # @ #",
		" ###   #",
		" # $ $##",
		"## $  # ",
		"#.  # # ",
		"#..   # ",
		"####### "
	),
	tLevelData(
		"   #####",
		"####. @#",
		"#  .$  #",
		"# #  ###",
		"# $ $ .#",
		"####   #",
		"   #####",
		"        "
	),
	tLevelData(
		"########",
		"#  .# @#",
		"# # $  #",
		"# $.#$ #",
		"## .   #",
		" #  ####",
		" ####   ",
		"        "
	),
	tLevelData(
		"#######",
		"#     #",
		"#.## .#",
		"#*  $@#",
		"#  #$ #",
		"#  #  #",
		"#######",
		"        "
	),
	tLevelData(
		"####    ",
		"#. #    ",
		"# $#    ",
		"#  #####",
		"# .$ @ #",
		"# .$ # #",
		"###    #",
		"  ######"
	),
	tLevelData(
		"########",
		"#      #",
		"# #$   #",
		"# $ @#.#",
		"##$#.  #",
		" #    .#",
		" #######",
		"        "
	),
	tLevelData(
		"######  ",
		"#  . #  ",
		"#    ###",
		"# #$$. #",
		"#.  ## #",
		"#@$ ## #",
		"###    #",
		"  ######"
	)
};

constexpr UBYTE s_ubTileSize = 16;
tLevelState s_CurrentLevel;
tBitMap *s_pBmTiles;

//------------------------------------------------------------------ PRIVATE FNS

tUbCoordYX positionMoveInDirection(tUbCoordYX Position, tDirection eDirection) {
	switch(eDirection) {
		case DIRECTION_UP:
			--Position.ubY;
			break;
		case DIRECTION_DOWN:
			++Position.ubY;
			break;
		case DIRECTION_LEFT:
			--Position.ubX;
			break;
		case DIRECTION_RIGHT:
			++Position.ubX;
			break;
		case DIRECTION_FIRE:
		case DIRECTION_COUNT:
			break;
	}
	return Position;
}

void loadLevel(UBYTE ubIndex) {
	s_CurrentLevel.load(s_pLevels[ubIndex]);
	s_CurrentLevel.drawAll();
}

void pageSokobanProcess(void) {
	tDirection eDirection = DIRECTION_COUNT;
	if(commNavUse(DIRECTION_UP)) {
		eDirection = DIRECTION_UP;
	}
	else if(commNavUse(DIRECTION_DOWN)) {
		eDirection = DIRECTION_DOWN;
	}
	else if(commNavUse(DIRECTION_LEFT)) {
		eDirection = DIRECTION_LEFT;
	}
	else if(commNavUse(DIRECTION_RIGHT)) {
		eDirection = DIRECTION_RIGHT;
	}

	if(eDirection != DIRECTION_COUNT) {
		bool isMoved = s_CurrentLevel.tryMovePlayer(eDirection);
		if(isMoved) {
			s_CurrentLevel.drawDirty();
		}
	}
}

void pageSokobanDestroy() {
	bitmapDestroy(s_pBmTiles);
}

constexpr tLevelData::tLevelData(
	const char *pRow1, const char *pRow2, const char *pRow3, const char *pRow4,
	const char *pRow5, const char *pRow6, const char *pRow7, const char *pRow8
) {
	for(UBYTE ubX = 0; ubX < s_ubWidth; ++ubX) {
		m_pTiles[ubX][0] = static_cast<tTile>(pRow1[ubX]);
		m_pTiles[ubX][1] = static_cast<tTile>(pRow2[ubX]);
		m_pTiles[ubX][2] = static_cast<tTile>(pRow3[ubX]);
		m_pTiles[ubX][3] = static_cast<tTile>(pRow4[ubX]);
		m_pTiles[ubX][4] = static_cast<tTile>(pRow5[ubX]);
		m_pTiles[ubX][5] = static_cast<tTile>(pRow6[ubX]);
		m_pTiles[ubX][6] = static_cast<tTile>(pRow7[ubX]);
		m_pTiles[ubX][7] = static_cast<tTile>(pRow8[ubX]);
	}
}

constexpr void tLevelState::load(const tLevelData &rLevelData) {
	m_LevelData  = rLevelData;

	// Initialize metadata
	m_ubSlotCount = 0;
	m_ubDirtyCount = 0;
	for(UBYTE ubY = 0; ubY < tLevelData::s_ubHeight; ++ubY) {
		for(UBYTE ubX = 0; ubX < tLevelData::s_ubWidth; ++ubX) {
			switch(rLevelData.m_pTiles[ubX][ubY]) {
				case tLevelData::tTile::PLAYER_ON_FLOOR:
				case tLevelData::tTile::PLAYER_ON_SLOT:
					m_PlayerPos = {.ubY = ubY, .ubX = ubX};
					break;
				case tLevelData::tTile::BOX_ON_SLOT:
					m_pSlotPositions[m_ubSlotCount++] = {.ubY = ubY, .ubX = ubX};
					break;
				case tLevelData::tTile::SLOT:
					m_pSlotPositions[m_ubSlotCount++] = {.ubY = ubY, .ubX = ubX};
					break;
				case tLevelData::tTile::BOX_ON_FLOOR:
				case tLevelData::tTile::FLOOR:
				case tLevelData::tTile::WALL:
					break;
			}
		}
	}
}

constexpr const tLevelData &tLevelState::getData() const {
	return m_LevelData;
}

constexpr bool tLevelState::isSolved() const {
	for(UBYTE i = 0; i < m_ubSlotCount; ++i) {
		const tUbCoordYX &rBoxPos = m_pSlotPositions[i];
		if(m_LevelData.m_pTiles[rBoxPos.ubX][rBoxPos.ubY] != tLevelData::tTile::BOX_ON_SLOT) {
			return false;
		}
	}

	return true;
}

constexpr bool tLevelState::tryMoveBox(tUbCoordYX BoxPos, tDirection eDirection) {
	using tTile = tLevelData::tTile;

	auto NewBoxPosition = positionMoveInDirection(BoxPos, eDirection);
	auto &rCurrentTileOnBoxPos = m_LevelData.m_pTiles[BoxPos.ubX][BoxPos.ubY];
	auto &rCurrentTileOnNewBoxPos = m_LevelData.m_pTiles[NewBoxPosition.ubX][NewBoxPosition.ubY];

	if(rCurrentTileOnNewBoxPos == tTile::FLOOR) {
		// TODO: move in box array
		rCurrentTileOnNewBoxPos = tTile::BOX_ON_FLOOR;
	}
	else if(rCurrentTileOnNewBoxPos == tTile::SLOT) {
		rCurrentTileOnNewBoxPos = tTile::BOX_ON_SLOT;
	}
	else {
		return false;
	}

	m_pDirtyPositions[m_ubDirtyCount++] = BoxPos;
	m_pDirtyPositions[m_ubDirtyCount++] = NewBoxPosition;

	if(rCurrentTileOnBoxPos == tTile::BOX_ON_SLOT) {
		rCurrentTileOnBoxPos = tTile::SLOT;
	}
	else {
		rCurrentTileOnBoxPos = tTile::FLOOR;
	}
	return true;
}

constexpr bool tLevelState::tryMovePlayer(tDirection eDirection) {
	using tTile = tLevelData::tTile;
	auto NewPlayerPos = positionMoveInDirection(m_PlayerPos, eDirection);

	auto &rCurrentTileOnNewPlayerPos = m_LevelData.m_pTiles[NewPlayerPos.ubX][NewPlayerPos.ubY];
	switch(rCurrentTileOnNewPlayerPos) {
		case tTile::WALL:
			return false;
		case tTile::FLOOR:
			rCurrentTileOnNewPlayerPos = tTile::PLAYER_ON_FLOOR;
			break;
		case tTile::SLOT:
			rCurrentTileOnNewPlayerPos = tTile::PLAYER_ON_SLOT;
			break;
		case tTile::BOX_ON_SLOT:
		case tTile::BOX_ON_FLOOR: {
			bool isBoxMoved = tryMoveBox(NewPlayerPos, eDirection);
			if(!isBoxMoved) {
				return false;
			}
			if(rCurrentTileOnNewPlayerPos == tTile::FLOOR) {
				rCurrentTileOnNewPlayerPos = tTile::PLAYER_ON_FLOOR;
			}
			else { // if(rCurrentTileOnNewPlayerPos == tTile::SLOT)
				rCurrentTileOnNewPlayerPos = tTile::PLAYER_ON_SLOT;
			}
		} break;
		case tTile::PLAYER_ON_FLOOR:
		case tTile::PLAYER_ON_SLOT:
			break;
	}

	m_pDirtyPositions[m_ubDirtyCount++] = m_PlayerPos;
	m_pDirtyPositions[m_ubDirtyCount++] = NewPlayerPos;

	tTile &rOldPlayerTile = m_LevelData.m_pTiles[m_PlayerPos.ubX][m_PlayerPos.ubY];
	if(rOldPlayerTile == tTile::PLAYER_ON_FLOOR) {
		rOldPlayerTile = tTile::FLOOR;
	}
	else if(rOldPlayerTile == tTile::PLAYER_ON_SLOT) {
		rOldPlayerTile = tTile::SLOT;
	}
	m_PlayerPos = NewPlayerPos;
	return true;
}

void tLevelState::drawTileAt(UBYTE ubX, UBYTE ubY) {
	tBitMap *pBuffer = commGetDisplayBuffer();
	tUwCoordYX sOrigin = commGetOriginDisplay();
	const auto Tile = s_CurrentLevel.getData().m_pTiles[ubX][ubY];

	UBYTE ubTileOffsetY;
	switch(Tile) {
		case tLevelData::tTile::WALL:
			ubTileOffsetY = 0;
			break;
		case tLevelData::tTile::FLOOR:
			ubTileOffsetY = 1 * 16;
			break;
		case tLevelData::tTile::PLAYER_ON_FLOOR:
		case tLevelData::tTile::PLAYER_ON_SLOT:
			ubTileOffsetY = 2 * 16;
			break;
		case tLevelData::tTile::BOX_ON_FLOOR:
		case tLevelData::tTile::BOX_ON_SLOT:
			ubTileOffsetY = 3 * 16;
			break;
		case tLevelData::tTile::SLOT:
			ubTileOffsetY = 4 * 16;
			break;
	}

	blitCopy(
		s_pBmTiles, 0, ubTileOffsetY,
		pBuffer,
		sOrigin.uwX + ubX * s_ubTileSize,
		sOrigin.uwY + ubY * s_ubTileSize,
		s_ubTileSize, s_ubTileSize, MINTERM_COOKIE
	);
}

void tLevelState::drawAll(void) {
	commEraseAll();
	for(UBYTE ubY = 0; ubY < tLevelData::s_ubHeight; ++ubY) {
		for(UBYTE ubX = 0; ubX < tLevelData::s_ubWidth; ++ubX) {
			drawTileAt(ubX, ubY);
		}
	}

	m_ubDirtyCount = 0;
}

void tLevelState::drawDirty(void) {
	for(UBYTE ubIdx = 0; ubIdx < m_ubDirtyCount; ++ubIdx) {
		drawTileAt(m_pDirtyPositions[ubIdx].ubX, m_pDirtyPositions[ubIdx].ubY);
	}

	m_ubDirtyCount = 0;
}

} // namespace

//------------------------------------------------------------------- PUBLIC FNS


void pageSokobanCreate(void) {
	s_pBmTiles = bitmapCreateFromFile("data/soko.bm", 0);
	commRegisterPage(pageSokobanProcess, pageSokobanDestroy);
	loadLevel(0);
}
