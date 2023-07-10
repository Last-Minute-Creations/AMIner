/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "menu.h"
#include <ace/managers/system.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include <ace/managers/bob.h>
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

static void menuOnStoryStart(void);
static void menuOnStoryLoad(void);
static void menuOnExit(void);
static void menuOnShowScores(void);
static void menuOnBackToMain();
static void menuOnFreeStart();
static void menuOnFreeLoad();
static void menuOnChallengeStart();
static void menuOnBackToMainFromSettings();
static void menuOnEnterStory();
static void menuOnEnterFree();
static void menuOnEnterChallenge();
static void menuOnEnterSettings();
static void menuOnEnterCredits();

//----------------------------------------------------------------- PRIVATE VARS

static tState s_sStateMenuScore;

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

//------------------------------------------------------------------ PUBLIC VARS

char **g_pMenuCaptions;
char **g_pMenuEnumP1;
char **g_pMenuEnumP2;
char **g_pMenuEnumOnOff;
char **g_pMenuEnumPlayerCount;
char **	g_pMenuEnumVolume;
tPtplayerMod *g_pMenuMod;

static void menuEnableAtari(void) {
	if(s_ubIndexAtari != INDEX_ATARI_INVALID && g_sSettings.isAtariHidden) {
		g_sSettings.isAtariHidden = 0;
		menuListSetPosHidden(s_ubIndexAtari, 0); // TODO: find option
		audioMixerPlaySfx(s_pSfxAtari, SFX_CHANNEL_ATARI, 1, 0);
	}
}

static void menuStartGame(UBYTE isChallenge) {
	commEraseAll();
	gameStart(
		isChallenge,
		g_sSettings.is1pKbd ? steerInitKey(STEER_KEYMAP_WSAD) : steerInitJoy(0),
		g_sSettings.is2pKbd ? steerInitKey(STEER_KEYMAP_ARROWS) : steerInitJoy(1)
	);
	commHide();
	// viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	// copProcessBlocks();
	s_eMenuState = MENU_STATE_ROLL_OUT;
}

static void menuLoadGame(const char *szSavePath) {
	commEraseAll();
	gameStart(
		1, // challenge loading is faster due to less terrain prep
		g_sSettings.is1pKbd ? steerInitKey(STEER_KEYMAP_WSAD) : steerInitJoy(0),
		g_sSettings.is2pKbd ? steerInitKey(STEER_KEYMAP_ARROWS) : steerInitJoy(1)
	);

	systemUse();
	tFile *pSave = fileOpen(szSavePath, "rb");
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

static void menuOnStoryStart(void) {
	menuStartGame(0);
}

static void menuOnStoryLoad(void) {
	menuLoadGame("save_story.dat");
}

static void menuOnShowScores(void) {
	commEraseAll();
	hiScoreSetup(0, 0);
	statePush(g_pGameStateManager, &s_sStateMenuScore);
}

static void menuOnExit(void) {
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
		isActive ? COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
	);
	*pUndrawWidth = fontMeasureText(g_pFont, szText).uwX;
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

static void menuOnEnterStory(void) {
	s_ubMenuOptionCount = 0;
	s_ubIndexAtari = INDEX_ATARI_INVALID;

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_START],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnStoryStart}
	};

	if(fileExists("save_story.dat")) {
		s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
			.szCaption = g_pMenuCaptions[MENU_CAPTION_CONTINUE],
			.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
			.isHidden = 0,
			.sOptCb = {.cbSelect = menuOnStoryLoad}
		};
	}

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

static void menuOnEnterFree(void) {
	s_ubMenuOptionCount = 0;
	s_ubIndexAtari = INDEX_ATARI_INVALID;

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_START],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnFreeStart}
	};

	if(fileExists("save_free.dat")) {
		s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
			.szCaption = g_pMenuCaptions[MENU_CAPTION_CONTINUE],
			.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
			.isHidden = 0,
			.sOptCb = {.cbSelect = menuOnFreeLoad}
		};
	}

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
		.szCaption = g_pMenuCaptions[MENU_CAPTION_BACK],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnBackToMain}
	};

	menuRedraw();
}

static void menuOnEnterChallenge(void) {
	s_ubMenuOptionCount = 0;
	s_ubIndexAtari = INDEX_ATARI_INVALID;

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_START],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnChallengeStart}
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
			.pEnumLabels = (const char **)g_pMenuEnumVolume
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
			.pEnumLabels = (const char **)g_pMenuEnumVolume
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
		.szCaption = g_pMenuCaptions[MENU_CAPTION_FREE],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnEnterFree}
	};

	s_pMenuOptions[s_ubMenuOptionCount++] = (tMenuListOption) {
		.szCaption = g_pMenuCaptions[MENU_CAPTION_CHALLENGE],
		.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK,
		.isHidden = 0,
		.sOptCb = {.cbSelect = menuOnEnterChallenge}
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
	s_pMenuSteers[2] = g_sSettings.is2pKbd ? steerInitIdle() : steerInitJoy(STEER_MODE_JOY_2);

	tFile *pFileSettings = fileOpen("settings.dat", "wb");
	if(pFileSettings) {
		settingsSave(pFileSettings);
		fileClose(pFileSettings);
		logWrite("Saved settings\n");
	}
	menuOnEnterMain();
}

static void menuOnFreeStart(void) {
	logWrite("TODO: implement menuOnFreeStart");
}

static void menuOnFreeLoad(void) {
	menuLoadGame("save_free.dat");
}

static void menuOnChallengeStart(void) {
	menuStartGame(1);
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
	hudUpdate();

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
	}

	if(ubNewKey) {
		memmove(s_pKeyHistory+1, s_pKeyHistory, 8-1);
		s_pKeyHistory[0] = ubNewKey;
		if(!memcmp(s_pKeyHistory, s_pKeyKonami, 8)) {
			menuEnableAtari();
		}
	}
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

		if(!commTryShow(s_pMenuSteers, MENU_STEER_COUNT)) {
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
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

static void menuScoreGsDestroy(void) {
	menuInitialDraw(); // g_pMainBuffer->pScroll->pFront
}

//------------------------------------------------------------------- PUBLIC FNS

void menuPreload(void) {
	s_pLogo = bitmapCreateFromFile("data/logo.bm", 0);
	s_pSfxAtari = ptplayerSfxCreateFromFile("data/sfx/atari.sfx", 1);

	tFile *pFileSettings = fileOpen("settings.dat", "rb");
	if(pFileSettings) {
		if(settingsLoad(pFileSettings)) {
			logWrite("Saved settings\n");
		}
		else {
			logWrite("ERR: Can't save settings\n");
		}

		fileClose(pFileSettings);
	}

	// Init all steers except joy2 if not explicitly selected, because mouse may be connected there
	s_pMenuSteers[0] = steerInitJoy(STEER_MODE_JOY_1);
	s_pMenuSteers[1] = steerInitKey(STEER_KEYMAP_WSAD);
	s_pMenuSteers[2] = g_sSettings.is2pKbd ? steerInitIdle() : steerInitJoy(STEER_MODE_JOY_2);
	s_pMenuSteers[3] = steerInitKey(STEER_KEYMAP_ARROWS);
}

void menuUnload(void) {
	bitmapDestroy(s_pLogo);
	ptplayerSfxDestroy(s_pSfxAtari);
}

void menuGsEnter(UBYTE isScoreShow) {
	// Switch to menu, after popping it will process gameGsLoop
	s_isScoreShowAfterRollIn = isScoreShow;
	if(!isScoreShow) {
		hudReset(0, 0);
	}
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
