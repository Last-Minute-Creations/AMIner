/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "menu.h"
#include <ace/managers/system.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include <ace/managers/bob.h>
#include <ace/utils/disk_file.h>
#include <ace/contrib/managers/audio_mixer.h>
#include <comm/comm.h>
#include "game.h"
#include "text_bob.h"
#include "ground_layer.h"
#include "base.h"
#include "hi_score.h"
#include "hud.h"
#include "core.h"
#include "settings.h"
#include "menu_list.h"
#include "steer.h"
#include "assets.h"
#include "achievement.h"

#define SFX_CHANNEL_ATARI 1
#define MENU_OPTIONS_MAX 10
#define INDEX_ATARI_INVALID 0xFF
#define MENU_STEER_COUNT 4

//------------------------------------------------------------------------ TYPES

typedef enum _tMenuState {
	MENU_STATE_ROLL_IN = 0,
	MENU_STATE_SELECTING,
	MENU_STATE_ROLL_OUT
} tMenuState;

//------------------------------------------------------------------- PROTOTYPES

static void menuOnGameStart(void);
static void menuOnStoryLoad(void);
static void menuOnExit(void);
static void menuOnShowScores(void);
static void menuOnBackToMain();
static void menuOnDeadlineLoad();
static void menuOnBackToMainFromSettings();
static void menuOnEnterStory();
static void menuOnEnterDeadline();
static void menuOnEnterChallenge();
static void menuOnEnterAchievements();
static void menuOnEnterSettings();
static void menuOnEnterCredits();

//----------------------------------------------------------------- PRIVATE VARS

static tState s_sStateMenuScore;
static tState s_sStateMenuAchievements;

static tMenuListOption s_pMenuOptions[MENU_OPTIONS_MAX];
static UBYTE s_ubMenuOptionCount;

static tPtplayerSfx *s_pSfxAtari;

static UBYTE s_pKeyHistory[8] = {0};
static UBYTE s_pKeyKonami[8] = {
	KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_DOWN, KEY_UP, KEY_UP
};

static tMenuState s_eMenuState;
static tBitMap *s_pLogo;
static UWORD s_uwOffsY;
static UBYTE s_isScoreShowAfterRollIn;
static UBYTE s_ubIndexAtari;
static tSteer s_pMenuSteers[MENU_STEER_COUNT];
static tGameSummary s_sSummary;
static tGameMode s_eModeMenu;

//------------------------------------------------------------------ PUBLIC VARS

char **g_pMenuCaptions;
char **g_pMenuEnumP1;
char **g_pMenuEnumP2;
char **g_pMenuEnumOnOff;
char **g_pMenuEnumPlayerCount;
char **	g_pMenuEnumVolume;

static void menuEnableAtari(void) {
	if(s_ubIndexAtari != INDEX_ATARI_INVALID && g_sSettings.isAtariHidden) {
		g_sSettings.isAtariHidden = 0;
		menuListSetPosHidden(s_ubIndexAtari, 0); // TODO: find option
		audioMixerPlaySfx(s_pSfxAtari, SFX_CHANNEL_ATARI, 1, 0);
	}
}

static void menuStartGame(tGameMode eMode) {
	commEraseAll();
	commProgressInit();
	gameStart(
		eMode,
		g_sSettings.is1pKbd ? steerInitFromMode(STEER_MODE_KEY_WSAD) : steerInitFromMode(STEER_MODE_JOY_1),
		g_sSettings.is2pKbd ? steerInitFromMode(STEER_MODE_KEY_ARROWS) : steerInitFromMode(STEER_MODE_JOY_2)
	);
	commHide();
	// viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	// copProcessBlocks();
	s_eMenuState = MENU_STATE_ROLL_OUT;
}

static void menuLoadGame(const char *szSavePath) {
	commEraseAll();
	commProgressInit();
	gameStart(
		GAME_MODE_CHALLENGE, // challenge loading is faster due to less terrain prep
		g_sSettings.is1pKbd ? steerInitFromMode(STEER_MODE_KEY_WSAD) : steerInitFromMode(STEER_MODE_JOY_1),
		g_sSettings.is2pKbd ? steerInitFromMode(STEER_MODE_KEY_ARROWS) : steerInitFromMode(STEER_MODE_JOY_2)
	);

	systemUse();
	tFile *pSave = diskFileOpen(szSavePath, "rb");
	if(pSave) {
		if(!gameLoad(pSave)) {
			logWrite("ERR: Failed to load game\n");
		}
		fileClose(pSave);
	}
	else {
		logWrite("ERR: Save file not found\n");
	}
	systemUnuse();

	commHide();
	// viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	// copProcessBlocks();
	s_eMenuState = MENU_STATE_ROLL_OUT;
}

static void menuOnStoryLoad(void) {
	menuLoadGame("save_story.dat");
}

static void menuOnShowScores(void) {
	commEraseAll();
	hiScoreSetup(0, 0, (s_eModeMenu == GAME_MODE_CHALLENGE) ? SCORE_MODE_CHALLENGE : SCORE_MODE_DEADLINE);
	statePush(g_pGameStateManager, &s_sStateMenuScore);
}

static void menuOnExit(void) {
	settingsFileSave();
	statePopAll(g_pGameStateManager);
}

static void onMenuPosUndraw(
	UWORD uwX, UWORD uwY, UWORD uwWidth, UWORD uwHeight
) {
	// Accomodate for text shadow, clear one more line
	++uwHeight;

	commErase(uwX + (COMM_DISPLAY_WIDTH - uwWidth) / 2, uwY, uwWidth, uwHeight);
}

static void onMenuPosDraw(
	UWORD uwX, UWORD uwY, UNUSED_ARG const char *szCaption, const char *szText,
	UBYTE isActive, UWORD *pUndrawWidth
) {
	commDrawText(
		uwX  + COMM_DISPLAY_WIDTH / 2, uwY,
		szText, FONT_SHADOW | FONT_COOKIE | FONT_HCENTER,
		isActive ? COMM_DISPLAY_COLOR_TEXT_HOVER : COMM_DISPLAY_COLOR_TEXT_DARK
	);
	*pUndrawWidth = fontMeasureText(g_pFont, szText).uwX;

	if(szCaption == g_pMenuCaptions[MENU_CAPTION_CONTINUE] && s_eModeMenu == GAME_MODE_STORY) {
		UBYTE ubLineHeight = commGetLineHeight();
		UWORD uwY = COMM_DISPLAY_HEIGHT - 4 * ubLineHeight;
		if(isActive) {
			char szBfr[50];
			const UWORD uwColumnWidth = COMM_DISPLAY_WIDTH / 2;

			ULONG ulGameDays = (s_sSummary.ulGameTime + GAME_TIME_PER_DAY - 1) / GAME_TIME_PER_DAY;
			sprintf(szBfr, "%s %lu", g_pMsgs[MSG_COMM_DAYS], ulGameDays);
			commDrawText(0, uwY, szBfr, FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT_DARK);

			sprintf(szBfr, "%s %ld\x1F", g_pMsgs[MSG_HUD_CASH], s_sSummary.lCash);
			commDrawText(uwColumnWidth, uwY, szBfr, FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT_DARK);

			uwY += ubLineHeight;

			sprintf(szBfr, "%s %hhu", g_pMsgs[MSG_COMM_ACCOLADES], s_sSummary.ubAccolades);
			commDrawText(0, uwY, szBfr, FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT_DARK);

			sprintf(szBfr, "%s %hhu", g_pMsgs[MSG_COMM_REBUKES], s_sSummary.ubRebukes);
			commDrawText(uwColumnWidth, uwY, szBfr, FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT_DARK);

			uwY += ubLineHeight;

			sprintf(szBfr, "%s %hhu%%", g_pMsgs[MSG_COMM_PLANS], (100 * s_sSummary.ubPlanIndex) / 50);
			commDrawText(0, uwY, szBfr, FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT_DARK);

			uwY += ubLineHeight;

			sprintf(szBfr, "%s %hu.%hhum", g_pMsgs[MSG_HUD_DEPTH], s_sSummary.uwMaxDepth / 10, s_sSummary.uwMaxDepth % 10);
			commDrawText(0, uwY, szBfr, FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT_DARK);

			sprintf(szBfr, "%s %hu%%", g_pMsgs[MSG_COMM_HEAT], s_sSummary.ubHeatPercent);
			commDrawText(uwColumnWidth, uwY, szBfr, FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT_DARK);
		}
		else {
			commErase(0, uwY, COMM_DISPLAY_WIDTH, 4 * ubLineHeight);
		}
	}
}

static void menuRedraw(void) {
	tBitMap *pDisplayBuffer = commGetDisplayBuffer();
	commEraseAll();
	UWORD uwLogoWidth = bitmapGetByteWidth(s_pLogo)*8;
	const tUwCoordYX sOrigin = commGetOriginDisplay();
	blitCopy(
		s_pLogo, 0, 0, pDisplayBuffer,
		sOrigin.uwX + (COMM_DISPLAY_WIDTH - uwLogoWidth) / 2, sOrigin.uwY,
		uwLogoWidth, s_pLogo->Rows, MINTERM_COOKIE
	);

	menuListInit(
		s_pMenuOptions, s_ubMenuOptionCount, g_pFont,
		0, s_pLogo->Rows + 10, // commrade-relative
		onMenuPosUndraw, onMenuPosDraw
	);
}

static UBYTE menuLoadSummaryFromSave(const char *szPath, tGameSummary *pSummary) {
	systemUse();

	if(!diskFileExists(szPath)) {
		logWrite("No save file to load summary file from\n");
		systemUnuse();
		return 0;
	}

	tFile *pFileSave = diskFileOpen(szPath, "rb");
	if(!pFileSave) {
		logWrite("ERR: Save file not found\n");
		systemUnuse();
		return 0;
	}

	UBYTE isLoaded = gameLoadSummary(pFileSave, pSummary);
	if(!isLoaded) {
		logWrite("ERR: Failed to load game summary\n");
	}
	fileClose(pFileSave);
	systemUnuse();
	return isLoaded;
}

static void menuOnEnterStory(void) {
	s_eModeMenu = GAME_MODE_STORY;
	s_ubMenuOptionCount = 0;
	s_ubIndexAtari = INDEX_ATARI_INVALID;

	if(menuLoadSummaryFromSave("save_story.dat", &s_sSummary)) {
		s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
			.szCaption = g_pMenuCaptions[MENU_CAPTION_CONTINUE],
			.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
			.isHidden = 0,
			.sOptCb = {.cbSelect = menuOnStoryLoad}
		};
	}

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_START],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnGameStart}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_PLAYER_COUNT],
		.eOptionType = MENU_LIST_OPTION_TYPE_UINT8,
		.isHidden = 0,
		.sOptUb = {
			.pVar = &g_is2pPlaying,
			.ubMax = 1,
			.isCyclic = 0,
			.pEnumLabels = (const char **)g_pMenuEnumPlayerCount
		}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_BACK],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnBackToMain}
	};

	menuRedraw();
}

static void menuOnEnterDeadline(void) {
	s_eModeMenu = GAME_MODE_DEADLINE;
	s_ubMenuOptionCount = 0;
	s_ubIndexAtari = INDEX_ATARI_INVALID;

	if(diskFileExists("save_deadline.dat")) {
		s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
			.szCaption = g_pMenuCaptions[MENU_CAPTION_CONTINUE],
			.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
			.isHidden = 0,
			.sOptCb = {.cbSelect = menuOnDeadlineLoad}
		};
	}

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_START],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnGameStart}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_PLAYER_COUNT],
		.eOptionType = MENU_LIST_OPTION_TYPE_UINT8,
		.isHidden = 0,
		.sOptUb = {
			.pVar = &g_is2pPlaying,
			.ubMax = 1,
			.isCyclic = 0,
			.pEnumLabels = (const char **)g_pMenuEnumPlayerCount
		}
	};
	s_ubIndexAtari = s_ubMenuOptionCount;

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_ATARI_MODE],
		.eOptionType = MENU_LIST_OPTION_TYPE_UINT8,
		.isHidden = g_sSettings.isAtariHidden,
		.sOptUb = {
			.pVar = &g_isAtari,
			.ubMax = 1,
			.isCyclic = 0,
			.pEnumLabels = (const char **)g_pMenuEnumOnOff
		}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_HI_SCORES],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnShowScores}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_BACK],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnBackToMain}
	};

	menuRedraw();
}

static void menuOnEnterChallenge(void) {
	s_eModeMenu = GAME_MODE_CHALLENGE;
	s_ubMenuOptionCount = 0;
	s_ubIndexAtari = INDEX_ATARI_INVALID;

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_START],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnGameStart}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_PLAYER_COUNT],
		.eOptionType = MENU_LIST_OPTION_TYPE_UINT8,
		.isHidden = 0,
		.sOptUb = {
			.pVar = &g_is2pPlaying,
			.ubMax = 1,
			.isCyclic = 0,
			.pEnumLabels = (const char **)g_pMenuEnumPlayerCount
		}
	};
	s_ubIndexAtari = s_ubMenuOptionCount;

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_ATARI_MODE],
		.eOptionType = MENU_LIST_OPTION_TYPE_UINT8,
		.isHidden = g_sSettings.isAtariHidden,
		.sOptUb = {
			.pVar = &g_isAtari,
			.ubMax = 1,
			.isCyclic = 0,
			.pEnumLabels = (const char **)g_pMenuEnumOnOff
		}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_HI_SCORES],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnShowScores}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_BACK],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnBackToMain}
	};

	menuRedraw();
}

static void menuOnEnterAchievements(void) {
	statePush(g_pGameStateManager, &s_sStateMenuAchievements);
}

static void menuUpdateVolume(void) {
	ptplayerSetMasterVolume((g_sSettings.ubMusicVolume * 16) / 10);
	audioMixerSetVolume((g_sSettings.ubSoundVolume * 64) / 10);
}

static void menuOnEnterSettings(void) {
	s_ubMenuOptionCount = 0;
	s_ubIndexAtari = INDEX_ATARI_INVALID;

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_VOLUME_SOUND],
		.eOptionType = MENU_LIST_OPTION_TYPE_UINT8,
		.isHidden = 0,
		.sOptUb = {
			.pVar = &g_sSettings.ubSoundVolume,
			.ubMax = 10,
			.isCyclic = 0,
			.pEnumLabels = (const char **)g_pMenuEnumVolume,
			.cbOnValChange = menuUpdateVolume,
		}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_VOLUME_MUSIC],
		.eOptionType = MENU_LIST_OPTION_TYPE_UINT8,
		.isHidden = 0,
		.sOptUb = {
			.pVar = &g_sSettings.ubMusicVolume,
			.ubMax = 10,
			.isCyclic = 0,
			.pEnumLabels = (const char **)g_pMenuEnumVolume,
			.cbOnValChange = menuUpdateVolume,
		}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_STEER_P1],
		.eOptionType = MENU_LIST_OPTION_TYPE_UINT8,
		.isHidden = 0,
		.sOptUb = {
			.pVar = &g_sSettings.is1pKbd,
			.ubMax = 1,
			.isCyclic = 1,
			.pEnumLabels = (const char **)g_pMenuEnumP1
		}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_STEER_P2],
		.eOptionType = MENU_LIST_OPTION_TYPE_UINT8,
		.isHidden = 0,
		.sOptUb = {
			.pVar = &g_sSettings.is2pKbd,
			.ubMax = 1,
			.isCyclic = 1,
			.pEnumLabels = (const char **)g_pMenuEnumP2
		}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_BACK],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnBackToMainFromSettings}
	};

	menuRedraw();
}

static void menuOnEnterMain(void) {
	s_ubMenuOptionCount = 0;
	s_ubIndexAtari = INDEX_ATARI_INVALID;

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_STORY],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnEnterStory}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_DEADLINE],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnEnterDeadline}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_CHALLENGE],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnEnterChallenge}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_ACHIEVEMENTS],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnEnterAchievements}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_SETTINGS],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnEnterSettings}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_CREDITS],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnEnterCredits}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_EXIT],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnExit}
	};

	menuRedraw();

	static const char szVersion[15] = "v." GAME_VERSION;
	commDrawText(
		COMM_DISPLAY_WIDTH / 2, COMM_DISPLAY_HEIGHT, szVersion,
		FONT_LAZY | FONT_HCENTER | FONT_COOKIE | FONT_SHADOW | FONT_BOTTOM,
		COMM_DISPLAY_COLOR_TEXT
	);
}

static void menuOnBackToMain(void) {
	menuOnEnterMain();
}

static void menuOnBackToMainFromSettings(void) {
	// Update the 2nd port steer to prevent interfering with mouse
	s_pMenuSteers[2] = g_sSettings.is2pKbd ? steerInitFromMode(STEER_MODE_IDLE) : steerInitFromMode(STEER_MODE_JOY_2);

	settingsFileSave();
	menuOnEnterMain();
}

static void menuOnGameStart(void) {
	menuStartGame(s_eModeMenu);
}

static void menuOnDeadlineLoad(void) {
	menuLoadGame("save_deadline.dat");
}

static void menuOnEnterCredits(void) {
	logWrite("TODO: implement menuOnEnterCredits");
}

static void menuInitialDraw(void) {
	memset(s_pKeyHistory, 0, 8);
	menuOnEnterMain();
}

static void menuGsCreate(void) {
	ptplayerLoadMod(g_pMenuMod, g_pModSampleData, 0);
	ptplayerEnableMusic(1);
	ptplayerConfigureSongRepeat(0, 0);
	s_eMenuState = MENU_STATE_ROLL_IN;
	hudShowMain();
}

static void menuProcessSelecting(void) {
	commProcess();

	menuListDraw();

	UBYTE ubNewKey = 0;
	if(commNavUse(DIRECTION_UP)) {
		ubNewKey = KEY_UP;
		menuListNavigate(-1);
	}
	else if(commNavUse(DIRECTION_DOWN)) {
		ubNewKey = KEY_DOWN;
		menuListNavigate(+1);
	}
	else if(commNavUse(DIRECTION_LEFT)) {
		ubNewKey = KEY_LEFT;
		menuListToggle(-1);
	}
	else if(commNavUse(DIRECTION_RIGHT)) {
		ubNewKey = KEY_RIGHT;
		menuListToggle(+1);
	}
	else if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		menuListEnter();
		return;
	}

	if(ubNewKey) {
		memmove(s_pKeyHistory+1, s_pKeyHistory, 8-1);
		s_pKeyHistory[0] = ubNewKey;
		if(!memcmp(s_pKeyHistory, s_pKeyKonami, 8)) {
			menuEnableAtari();
		}
	}

	hudProcess();
	// Process only managers of HUD because we want single buffering on main one
	vPortProcessManagers(g_pMainBuffer->sCommon.pVPort->pView->pFirstVPort);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

static void menuProcessRollIn(void) {
	bobDiscardUndraw();
	coreProcessBeforeBobs();

	UWORD *pCamY = &g_pMainBuffer->pCamera->uPos.uwY;
	UWORD uwAvailHeight = g_pMainBuffer->pScroll->uwBmAvailHeight;
	if(*pCamY < uwAvailHeight) {
		*pCamY += 4;
	}
	else if(*pCamY > uwAvailHeight) {
		*pCamY = uwAvailHeight + (*pCamY % uwAvailHeight);
		if(*pCamY - uwAvailHeight > 4) {
			*pCamY -= 4;
		}
		else {
			*pCamY = uwAvailHeight;
		}
	}
	else {
		s_eMenuState = MENU_STATE_SELECTING;
		s_uwOffsY = 16 + s_pLogo->Rows + 30;

		if(!commTryShow(s_pMenuSteers, MENU_STEER_COUNT, 0)) {
			// TODO do something
		}
		if(s_isScoreShowAfterRollIn) {
			statePush(g_pGameStateManager, &s_sStateMenuScore);
		}
		else {
			menuInitialDraw(); // g_pMainBuffer->pScroll->pBack
		}
	}

	coreProcessAfterBobs();
}

static void menuProcessRollOut(void) {
	coreProcessBeforeBobs();

	UWORD *pCamY = &g_pMainBuffer->pCamera->uPos.uwY;
	if(*pCamY >= 64) {
		*pCamY -= 4;
	}
	else {
		stateChange(g_pGameStateManager, &g_sStateGame);
	}
	coreProcessAfterBobs();
}

static void menuGsLoop(void) {
  if(keyUse(KEY_ESCAPE)) {
    statePopAll(g_pGameStateManager);
		return;
  }

	switch(s_eMenuState) {
		case MENU_STATE_ROLL_IN:
			menuProcessRollIn();
			break;
		case MENU_STATE_SELECTING:
			menuProcessSelecting();
			break;
		case MENU_STATE_ROLL_OUT:
			menuProcessRollOut();
	}
}

static void menuGsDestroy(void) {

}

static void menuScoreGsCreate(void) {
	hiScoreDrawAll();
}

static void menuScoreGsLoop(void) {
	commProcess();
	if(hiScoreIsEnteringNew()) {
		hiScoreEnteringProcess();
	}
	else {
		if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
			statePop(g_pGameStateManager);
			return;
		}
	}

	hudProcess();
	// Process only managers of HUD because we want single buffering on main one
	vPortProcessManagers(g_pMainBuffer->sCommon.pVPort->pView->pFirstVPort);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

static void menuScoreGsDestroy(void) {
	menuInitialDraw(); // g_pMainBuffer->pScroll->pFront
}

#define ACHIEVEMENT_ICON_SIZE 24
#define ACHIEVEMENT_SELECTION_BORDER_WIDTH 2
#define ACHIEVEMENT_BORDERED_ICON_SIZE (ACHIEVEMENT_ICON_SIZE + 2 * ACHIEVEMENT_SELECTION_BORDER_WIDTH)
#define ACHIEVEMENTS_PER_ROW (COMM_DISPLAY_WIDTH / ACHIEVEMENT_BORDERED_ICON_SIZE)

static tBitMap *s_pAchievementIcons;
static UBYTE s_ubSelectedAchievement;

static void menuDrawAchievementIcon(UBYTE ubIndex) {
	UWORD uwX = (ubIndex % ACHIEVEMENTS_PER_ROW) * ACHIEVEMENT_BORDERED_ICON_SIZE;
	UWORD uwY = (ubIndex / ACHIEVEMENTS_PER_ROW) * ACHIEVEMENT_BORDERED_ICON_SIZE;
	tUwCoordYX sOrigin = commGetOriginDisplay();

	if(ubIndex == s_ubSelectedAchievement) {
		blitCopy(
			g_pCommBmSelection, 9, 0,
			commGetDisplayBuffer(), sOrigin.uwX + uwX,
			sOrigin.uwY + uwY,
			7, 7, MINTERM_COOKIE
		);
		blitCopy(
			g_pCommBmSelection, 9, 7,
			commGetDisplayBuffer(), sOrigin.uwX + uwX + ACHIEVEMENT_BORDERED_ICON_SIZE - 7,
			sOrigin.uwY + uwY,
			7, 7, MINTERM_COOKIE
		);
		blitCopy(
			g_pCommBmSelection, 9, 14,
			commGetDisplayBuffer(), sOrigin.uwX + uwX,
			sOrigin.uwY + uwY + ACHIEVEMENT_BORDERED_ICON_SIZE - 7,
			7, 7, MINTERM_COOKIE
		);
		blitCopy(
			g_pCommBmSelection, 9, 21,
			commGetDisplayBuffer(), sOrigin.uwX + uwX + ACHIEVEMENT_BORDERED_ICON_SIZE - 7,
			sOrigin.uwY + uwY + ACHIEVEMENT_BORDERED_ICON_SIZE - 7,
			7, 7, MINTERM_COOKIE
		);
	}
	else {
		commErase(uwX, uwY, ACHIEVEMENT_BORDERED_ICON_SIZE, ACHIEVEMENT_BORDERED_ICON_SIZE);
	}

	blitCopy(
		s_pAchievementIcons, achievementIsUnlocked(ubIndex) ? ACHIEVEMENT_ICON_SIZE : 0, 0,
		commGetDisplayBuffer(), sOrigin.uwX + uwX + 2, sOrigin.uwY + uwY + 2,
		ACHIEVEMENT_ICON_SIZE, ACHIEVEMENT_ICON_SIZE, MINTERM_COOKIE
	);
}

static void menuDrawCurrentAchievementDescription(void) {
	UWORD uwDescriptionY = 3 * ACHIEVEMENT_BORDERED_ICON_SIZE;
	commErase(0, uwDescriptionY, COMM_DISPLAY_WIDTH, commGetLineHeight() * 3 - 2);
	commDrawTitle(0, uwDescriptionY, g_pMsgs[MSG_ACHIEVEMENT_TITLE_LAST_RIGHTEOUS + s_ubSelectedAchievement]);
	uwDescriptionY += commGetLineHeight() - 2;
	commDrawMultilineText(g_pMsgs[MSG_ACHIEVEMENT_DESC_LAST_RIGHTEOUS + s_ubSelectedAchievement], 0, uwDescriptionY);
}

static void menuAchievementsGsCreate(void) {
	commEraseAll();
	s_pAchievementIcons = bitmapCreateFromPath("data/comm_achievements.bm", 0);
	s_ubSelectedAchievement = 0;

	for(UBYTE i = 0; i < 18; ++i) {
		menuDrawAchievementIcon(i);
	}

	menuDrawCurrentAchievementDescription();

	commDrawText(
		COMM_DISPLAY_WIDTH / 2, COMM_DISPLAY_HEIGHT - 1,
		g_pMsgs[MSG_HI_SCORE_PRESS],
		FONT_BOTTOM | FONT_HCENTER | FONT_SHADOW | FONT_COOKIE,
		COMM_DISPLAY_COLOR_TEXT_HOVER
	);
}

static void menuAchievementsGsLoop(void) {
	commProcess();
	if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		statePop(g_pGameStateManager);
		return;
	}

	BYTE bNewSelection = s_ubSelectedAchievement;
	if(commNavUse(DIRECTION_LEFT)) {
		if(bNewSelection % ACHIEVEMENTS_PER_ROW >= 1) {
			--bNewSelection;
		}
		else {
			bNewSelection = (bNewSelection / ACHIEVEMENTS_PER_ROW) * ACHIEVEMENTS_PER_ROW + (ACHIEVEMENTS_PER_ROW - 1);
		}
	}
	if(commNavUse(DIRECTION_RIGHT)) {
		if(bNewSelection % ACHIEVEMENTS_PER_ROW < (ACHIEVEMENTS_PER_ROW - 1)) {
			++bNewSelection;
		}
		else {
			bNewSelection = (bNewSelection / ACHIEVEMENTS_PER_ROW) * ACHIEVEMENTS_PER_ROW;
		}
	}
	if(commNavUse(DIRECTION_UP)) {
		bNewSelection -= ACHIEVEMENTS_PER_ROW;
		if(bNewSelection < 0) {
			bNewSelection += 18;
		}
	}
	else if(commNavUse(DIRECTION_DOWN)) {
		bNewSelection += ACHIEVEMENTS_PER_ROW;
		if(bNewSelection >= 18) {
			bNewSelection -= 18;
		}
	}

	if(bNewSelection != s_ubSelectedAchievement) {
		UBYTE ubPrevSelection = s_ubSelectedAchievement;
		s_ubSelectedAchievement = bNewSelection;
		menuDrawAchievementIcon(ubPrevSelection);
		menuDrawAchievementIcon(s_ubSelectedAchievement);
		menuDrawCurrentAchievementDescription();
	}

	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

static void menuAchievementsGsDestroy(void) {
	bitmapDestroy(s_pAchievementIcons);

	menuInitialDraw();
}

//------------------------------------------------------------------- PUBLIC FNS

void menuPreload(void) {
	s_pLogo = bitmapCreateFromPath("data/logo.bm", 0);
	s_pSfxAtari = ptplayerSfxCreateFromPath("data/sfx/atari.sfx", 1);

	settingsFileLoad();
	// menuUpdateVolume();

	// Init all steers except joy2 if not explicitly selected, because mouse may be connected there
	s_pMenuSteers[0] = steerInitFromMode(STEER_MODE_JOY_1);
	s_pMenuSteers[1] = steerInitFromMode(STEER_MODE_KEY_WSAD);
	s_pMenuSteers[2] = g_sSettings.is2pKbd ? steerInitFromMode(STEER_MODE_IDLE) : steerInitFromMode(STEER_MODE_JOY_2);
	s_pMenuSteers[3] = steerInitFromMode(STEER_MODE_KEY_ARROWS);
}

void menuUnload(void) {
	bitmapDestroy(s_pLogo);
	ptplayerSfxDestroy(s_pSfxAtari);
}

void menuGsEnter(UBYTE isScoreShow) {
	// Save pending achievements in case user restarts amiga while being in menu
	settingsFileSave();

	// Switch to menu, after popping it will process gameGsLoop
	s_isScoreShowAfterRollIn = isScoreShow;
	// if(!isScoreShow) {
	// 	hudReset(0, 0);
	// }
	stateChange(g_pGameStateManager, &g_sStateMenu);
}

tState g_sStateMenu = {
	.cbCreate = menuGsCreate,
	.cbLoop = menuGsLoop,
	.cbDestroy = menuGsDestroy
};

static tState s_sStateMenuScore = {
	.cbCreate = menuScoreGsCreate,
	.cbLoop = menuScoreGsLoop,
	.cbDestroy = menuScoreGsDestroy
};

static tState s_sStateMenuAchievements = {
	.cbCreate = menuAchievementsGsCreate,
	.cbLoop = menuAchievementsGsLoop,
	.cbDestroy = menuAchievementsGsDestroy
};
