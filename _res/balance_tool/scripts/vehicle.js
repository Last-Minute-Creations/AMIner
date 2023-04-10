class Vehicle {
	constructor() {
		this.hullMk = 0;
		this.cargoMk = 0;
		this.drillMk = 0;
		this.money = 0;
		this.subAccolades = 0;
		this.accolades = 0;
		this.rebukes = 0;
		this.ending = Ending.NONE;
		this.cargoMinerals = {}; // [mineralId] => count
		this.stock = {}; // [mineralId] => count
		for(let mineral of MineralType.all) {
			this.stock[mineral.id] = 0;
		}

		this.hullMax = g_defs.hullBase;
		this.cargoMax = g_defs.cargoBase;
		this.drillMax = g_defs.drillBase;

		this.hullCurr = this.hullMax;
		this.cargoCurr = 0;
		this.drillCurr = this.drillMax;
	}

	tryExcavate(posX, posY) {
		let tile = g_tileMap.tiles[posX][posY];

		if(!g_tileMap.isSolid(posX, posY)) {
			return;
		}

		let isExcavatableFromSide = (
			// from left
			(posX && !g_tileMap.isSolid(posX - 1, posY) && g_tileMap.isSolid(posX - 1, posY  + 1)) ||
			// from right
			(posX < g_tileMap.width - 1 && !g_tileMap.isSolid(posX + 1, posY) && g_tileMap.isSolid(posX + 1, posY  + 1)) ||
			// from up
			(!g_tileMap.isSolid(posX, posY - 1))
		);
		if(!isExcavatableFromSide) {
			return;
		}

		let drillUnitCost = 15;
		let drillLevel = this.drillMk;
		let difficulty = tile.isHardToDrill() ? 10 : GroundLayer.getLayerAt(posY).drillDifficulty;
		let drillDuration = Math.max(1, difficulty - drillLevel);
		let drillCost = drillUnitCost * drillDuration;

		if(this.drillCurr < drillCost) {
			return false;
		}

		this.drillCurr -= drillCost;
		g_plan.elapseTime(drillCost);
		let mineralId = tile.mineralType.id;

		if(this.cargoMinerals[mineralId] == undefined) {
			this.cargoMinerals[mineralId] = 0;
		}
		this.cargoMinerals[mineralId] += tile.mineralAmount;

		this.cargoCurr = Math.min(this.cargoCurr + tile.mineralAmount, this.cargoMax);
		if(tile.mineralType.isCollectible && g_plan.mineralsUnlocked.indexOf(mineralId) == -1) {
			g_plan.mineralsUnlocked.push(mineralId);
		}

		// Update tile data
		g_tileMap.tiles[posX][posY] = new Tile(TileIndex.CAVE_BG_16,  MineralType.AIR, 0);
		g_tileMap.currentMineralCounts[tile.mineralType.id] -= tile.mineralAmount;

		return true;
	}

	restock() {
		for(let mineralId in this.cargoMinerals) {
			this.stock[mineralId] += this.cargoMinerals[mineralId];
			this.cargoMinerals[mineralId] = 0;
		}

		let liters = Math.floor((this.drillMax - this.drillCurr + 0.5 * g_defs.fuelInLiter) / g_defs.fuelInLiter);
		this.money -= (this.hullMax - this.hullCurr) * g_defs.hullPrice;
		this.money -= liters * g_defs.literPrice;

		this.hullCurr = this.hullMax;
		this.cargoCurr = 0;
		this.drillCurr = Math.min(this.drillCurr + liters * g_defs.fuelInLiter, this.drillMax);
	}

	trySell(mineralType, amount) {
		if(g_vehicle.stock[mineralType.id] == undefined) {
			return 0;
		}

		let sellAmount = Math.min(amount, g_vehicle.stock[mineralType.id]);
		g_vehicle.money += mineralType.reward * sellAmount;
		g_vehicle.stock[mineralType.id] -= sellAmount;
		return sellAmount;
	}

	tryFillPlan(mineralType, amount) {
		if(g_vehicle.stock[mineralType.id] == undefined) {
			return 0;
		}

		let requiredAmount = g_plan.mineralsRequired[mineralType.id] - g_plan.mineralsCollected[mineralType.id];
		let fillAmount = Math.min(Math.min(amount, g_vehicle.stock[mineralType.id]), requiredAmount);
		g_plan.mineralsCollected[mineralType.id] +=  fillAmount;
		g_vehicle.stock[mineralType.id] -= fillAmount;

		if(g_plan.tryProceed()) {
			if(++this.subAccolades == g_defs.maxSubAccolades) {
				this.subAccolades = 0;
				if(++this.accolades >= g_defs.maxAccolades) {
					this.ending = Ending.ACCOLADES_WIN;
				}
			}
		}

		return fillAmount;
	}

	tryUpgradePart(part) {
		if(part == 'hull') {
			if(this.hullMk < g_defs.upgradeCosts.length) {
				if(this.money >= g_defs.upgradeCosts[this.hullMk]) {
					this.money -= g_defs.upgradeCosts[this.hullMk];
					++this.hullMk;
					this.hullMax += g_defs.hullAddPerLevel;
				}
			}
		}
		if(part == 'drill') {
			if(this.drillMk < g_defs.upgradeCosts.length) {
				if(this.money >= g_defs.upgradeCosts[this.drillMk]) {
					this.money -= g_defs.upgradeCosts[this.drillMk];
					++this.drillMk;
					this.drillMax += g_defs.drillAddPerLevel;
				}
			}
		}
		if(part == 'cargo') {
			if(this.cargoMk < g_defs.upgradeCosts.length) {
				if(this.money >= g_defs.upgradeCosts[this.cargoMk]) {
					this.money -= g_defs.upgradeCosts[this.cargoMk];
					++this.cargoMk;
					this.cargoMax += g_defs.cargoAddPerLevel;
				}
			}
		}
	}

	addRebuke() {
		if(++this.rebukes >= g_defs.maxRebukes) {
			this.ending = Ending.REBUKE_LOST;
		}
	}

	isGameOver() {
		return this.ending != Ending.NONE;
	}
}
