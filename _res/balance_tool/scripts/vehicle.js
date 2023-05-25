class Vehicle {
	constructor() {
		this.hullMk = 0;
		this.cargoMk = 0;
		this.drillMk = 0;
		this.money = 0;
		this.moneySpentOnRestock = 0;
		this.subAccolades = 0;
		this.accolades = 0;
		this.rebukes = 0;
		this.heat = 5;
		this.bribeCount = 0;
		this.ending = Ending.NONE;
		this.stock = new Array(MineralType.all.length).fill(0); // [mineralId] => count
		this.sold = new Array(MineralType.all.length).fill(0); // [mineralId] => count

		this.hullMax = g_defs.hullBase;
		this.cargoMax = g_defs.cargoBase;
		this.drillMax = g_defs.drillBase;

		this.foundBones = 0;
		this.foundGates = 0;

		this.respawn();
	}

	respawn() {
		this.cargoMinerals = new Array(MineralType.all.length).fill(0); // [mineralId] => count
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
		g_plans.elapseTime(drillCost);
		let mineralId = tile.mineralType.id;

		// Add to cargo
		let cargoDelta = Math.min(tile.mineralAmount, this.cargoMax - this.cargoCurr);
		this.cargoMinerals[mineralId] += cargoDelta;
		this.cargoCurr += cargoDelta;
		if(this.cargoCurr == this.cargoMax) {
			addMessage('Cargo full', 'warning');
		}
		if(tile.mineralType.isCollectible && g_plans.mineralsUnlocked.indexOf(mineralId) == -1) {
			g_plans.mineralsUnlocked.push(mineralId);
		}

		// Update tile data
		g_tileMap.tiles[posX][posY] = new Tile(TileIndex.CAVE_BG_16,  MineralType.AIR, 0);
		if(tile.mineralType.isDino) {
			--g_tileMap.currentMineralCounts[tile.mineralType.id];
			++this.foundBones;
		}
		else if(tile.mineralType.isGate) {
			--g_tileMap.currentMineralCounts[tile.mineralType.id];
			++this.foundGates;
		}
		else {
			g_tileMap.currentMineralCounts[tile.mineralType.id] -= tile.mineralAmount;
		}
		g_tileMap.currentMoney -= tile.mineralAmount * tile.mineralType.reward;

		// Damage vehicle
		if(tile.mineralType.isDamaging) {
			this.damage(5 + (g_rand.next16() & 0x7));
		}

		return true;
	}

	restock() {
		for(let mineralId in this.cargoMinerals) {
			this.stock[mineralId] += this.cargoMinerals[mineralId];
			this.cargoMinerals[mineralId] = 0;
		}

		let drillUnits = Math.floor((this.drillMax - this.drillCurr + 0.5 * g_defs.dillInUnit) / g_defs.dillInUnit);
		let restockCost = 0;
		restockCost += (this.hullMax - this.hullCurr) * g_defs.hullPrice;
		restockCost += drillUnits * g_defs.drillUnitPrice;
		this.moneySpentOnRestock += restockCost;
		this.money -= restockCost;

		this.hullCurr = this.hullMax;
		this.cargoCurr = 0;
		this.drillCurr = Math.min(this.drillCurr + drillUnits * g_defs.dillInUnit, this.drillMax);

		// Simulated damage
		this.damage(g_defs.damageAfterRestock)
	}

	trySell(mineralType, amount) {
		let sellAmount = Math.min(amount, this.stock[mineralType.id]);
		this.money += mineralType.reward * sellAmount;
		this.stock[mineralType.id] -= sellAmount;
		this.sold[mineralType.id] += sellAmount;
		return sellAmount;
	}

	tryBlackMarketExchange(mineralSellType, sellAmount, mineralBuyType, buyAmount){
		if(this.stock[mineralSellType.id] < sellAmount) {
			return false;
		}

		this.stock[mineralSellType.id] -= sellAmount;
		this.stock[mineralBuyType.id] += buyAmount;
		return true;
	}

	tryFillPlan(mineralType, amount) {
		if(g_vehicle.stock[mineralType.id] == undefined) {
			return 0;
		}

		let amountRemaining = g_plans.getCurrentPlanInfo().mineralsRequired[mineralType.id] - g_plans.mineralsCollected[mineralType.id];
		let fillAmount = Math.min(Math.min(amount, g_vehicle.stock[mineralType.id]), amountRemaining);
		g_plans.mineralsCollected[mineralType.id] +=  fillAmount;
		g_vehicle.stock[mineralType.id] -= fillAmount;

		if(g_plans.tryProceed()) {
			this.heat = Math.max(0, this.heat - g_defs.heatRemovedByPlan);
			this.advanceAccolade();
			return true;
		}

		return false;
	}

	advanceAccolade() {
		if(++this.subAccolades == g_defs.maxSubAccolades) {
			this.subAccolades = 0;
			if(++this.accolades >= g_defs.maxAccolades) {
				this.ending = Ending.ACCOLADES_WIN;
				addMessage('Game won', 'success');
			}
			else {
				addMessage('New accolade', 'success');
			}
		}
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

	doAccounting() {
		let pick = g_rand.next16MinMax(1, 100);
		if(pick > this.heat) {
			// Bring back stuff already spent on plan
			for(let mineralType of MineralType.collectibles) {
				this.stock[mineralType.id] += g_plans.mineralsCollected[mineralType.id];
				g_plans.mineralsCollected[mineralType.id] = 0;
			}

			// Force next plan
			g_plans.next();
			this.advanceAccolade();
		}
		else {
			this.addRebuke('Accounting failed');
		}
		this.heat = Math.min(this.heat + g_defs.heatAddPerAccounting, 99);
	}

	getBribeCost() {
		let bribeCost = g_defs.bribeBaseCost;
		for(let i = 0; i < this.bribeCount; ++i) {
			bribeCost += Math.floor(bribeCost * g_defs.bribeCostMultiplier);
		}
		return bribeCost;
	}

	doBribe() {
		let bribeCost = this.getBribeCost();
		if(this.money < bribeCost) {
			return;
		}

		this.money -= bribeCost;
		let pick = g_rand.next16MinMax(1, 100);
		if(pick > this.heat) {
			g_plans.extendTime(14 * g_defs.timeInDay);
		}
		else {
			this.addRebuke('Bribe failed');
		}

		this.heat = Math.min(this.heat + g_defs.heatAddPerAccounting, 99);
		++this.bribeCount;
	}

	damage(amount) {
		this.hullCurr -= amount;
		if(this.hullCurr <= 0) {
				this.respawn();
				this.addRebuke('Vehicle destroyed');
		}
	}

	addRebuke(reason = null) {
		if(++this.rebukes >= g_defs.maxRebukes) {
			this.ending = Ending.REBUKE_LOST;
			addMessage('Game lost', 'error');
		}
		else {
			addMessage(`New rebuke${reason == null ? '' : " - " + reason}`, 'error');
		}
	}

	isGameOver() {
		return this.ending != Ending.NONE;
	}
}
