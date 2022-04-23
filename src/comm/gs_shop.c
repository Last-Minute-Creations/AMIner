/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gs_shop.h"
#include <ace/managers/system.h>
#include <ace/managers/rand.h>
#include <comm/base.h>
#include <comm/page_warehouse.h>
#include <comm/page_workshop.h>
#include <comm/page_office.h>
#include "core.h"
#include "hud.h"
#include "tutorial.h"

static tCommLed s_eTab;
static UBYTE s_isShown;

static void commShopShowTab(tCommLed eTab) {
	s_eTab = eTab;
	commSetActiveLed(eTab);
	commEraseAll();
	switch(eTab) {
		case COMM_LED_OFFICE:
			pageOfficeCreate();
			break;
		case COMM_LED_WORKSHOP:
			pageWorkshopCreate();
			break;
		case COMM_LED_WAREHOUSE:
			pageWarehouseCreate();
			break;
		default:
		break;
	}
}

static void commGsShopCreate(void) {
	s_isShown = commShow();
	if(!s_isShown) {
		// Camera not placed properly
		statePop(g_pGameStateManager);
		return;
	}

	s_eTab = COMM_LED_OFFICE;
	commShopShowTab(s_eTab);

	// Process managers once so that backbuffer becomes front buffer
	// Single buffering from now!
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

static void commGsShopLoop(void) {
	commProcess();
	tutorialProcess();

	tCommLed eOldTab = s_eTab;
	// Tab nav using shift+left / shift+right
	if(commNavExUse(COMM_NAV_EX_SHIFT_LEFT)) {
		if(s_eTab) {
			--s_eTab;
		}
		else {
			s_eTab = COMM_LED_COUNT - 1;
		}
	}
	else if(commNavExUse(COMM_NAV_EX_SHIFT_RIGHT)) {
		if(s_eTab < COMM_LED_COUNT - 1) {
			++s_eTab;
		}
		else {
			s_eTab = 0;
		}
	}

	hudUpdate();
	// Process only managers of HUD because we want single buffering on main one
	vPortProcessManagers(g_pMainBuffer->sCommon.pVPort->pView->pFirstVPort);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);

	if(s_eTab != eOldTab) {
		commShopShowTab(s_eTab);
	}
	else {
		if(!commProcessPage()) {
			logWrite("shop quit\n");
			statePop(g_pGameStateManager);
			return;
		}
	}
}

static void commGsShopDestroy(void) {
	if(!s_isShown) {
		return;
	}

	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
	commHide();
}

UBYTE commShopIsActive(void) {
	return s_isShown;
}

tState g_sStateShop = {
	.cbCreate = commGsShopCreate, .cbLoop = commGsShopLoop,
	.cbDestroy = commGsShopDestroy
};
