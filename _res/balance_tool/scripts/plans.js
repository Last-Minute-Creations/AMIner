class PlanInfo {
	constructor(targetSum, mineralsRequired) {
		this.targetSum = targetSum;
		this.mineralsRequired = mineralsRequired; // [mineralId] => count
	}
}

class Plans {
	constructor() {
		this.mineralsCollected = new Array(MineralType.all.length).fill(0); // [mineralId] => count
		this.mineralsUnlocked = [ MineralType.SILVER.id ]; // [mineralId]
		this.index = 0;
		this.isStarted = true;
		this.isExtendedByFavor = false;

		this.sequence = []; // [planIndex] => [planInfo]

		// Generate plan based on allowed minerals and target money - no more than dirt tiles in segment
		let targetSum = 15;
		let planCount = g_defs.maxAccolades * g_defs.maxSubAccolades;
		for(let planIndex = 0; planIndex < planCount; ++planIndex) {
			let mineralsRequired = new Array(MineralType.all.length).fill(0); // [mineralId] => count
			let costRemaining = targetSum;
			let mineralsAllowed = [];
			if(planIndex >= g_defs.planSilver) {
				mineralsAllowed.push(MineralType.SILVER.id);
			}
			if(planIndex >= g_defs.planGold) {
				mineralsAllowed.push(MineralType.GOLD.id);
			}
			if(planIndex >= g_defs.planEmerald) {
				mineralsAllowed.push(MineralType.EMERALD.id);
			}
			if(planIndex >= g_defs.planRuby) {
				mineralsAllowed.push(MineralType.RUBY.id);
			}
			if(planIndex >= g_defs.planMoonstone) {
				mineralsAllowed.push(MineralType.MOONSTONE.id);
			}

			while(mineralsAllowed.some((mineralId) => MineralType.all[mineralId].reward <= costRemaining)) {
				let mineralId = mineralsAllowed[g_rand.next16MinMax(0, mineralsAllowed.length - 1)];
				let reward = MineralType.all[mineralId].reward;
				let count = g_rand.next16Max(Math.floor((costRemaining + reward - 1) / reward));
				if(count > 0) {
					console.log(`adding ${count} of mineral ${MineralType.all[mineralId].name}`)
					mineralsRequired[mineralId] += count;
					costRemaining -= count * reward;
				}
			}
			this.sequence.push(new PlanInfo(targetSum, mineralsRequired));
			targetSum += Math.floor(targetSum * g_defs.planCostMultiplier);
		}

		this.start();
	}

	// todo: refactor for new procgen
	reroll() {
		this.mineralsRequired = new Array(MineralType.all.length).fill(0); // [mineralId] => count
		let costRemaining = this.targetSum;
		let i = 0;
		do {
			let collectibleIndex = g_rand.next16MinMax(0, MineralType.collectibles.length - 1);
			let mineralId = MineralType.collectibles[collectibleIndex].id;
			if(this.mineralsUnlocked.indexOf(mineralId) != -1) {
				let reward = MineralType.all[mineralId].reward;
				let count = g_rand.next16Max(Math.floor((costRemaining + reward - 1) / reward));

				this.mineralsRequired[mineralId] += count;
				costRemaining -= count * reward;
				console.log(`reward ${reward}, count ${count}, remaining: ${costRemaining}`);
			}
			if(++i > 100) {
				break;
			}
		} while(costRemaining > 0);
	}

	next() {
		++this.index;
		this.start();
	}

	start() {
		this.mineralsCollected = new Array(MineralType.all.length).fill(0); // [mineralId] => count
		this.timeRemaining = 2 * 2 * 1000; // two full fuel, per player
		this.timeRemaining += 200; // Add for nice division into 30 days
		this.wasWarning = false;
	}

	getCurrentPlanInfo() {
		return this.sequence[this.index];
	}

	isCompleted() {
		for(let i = 0; i < MineralType.all.length; ++i) {
			if(this.mineralsRequired[i] != this.mineralsCollected[i]) {
				return false;
			}
		}
		return true;
	}

	tryProceed() {
		if(this.isCompleted()) {
			this.next();
			return true;
		}
		return false;
	}

	elapseTime(timeDelta) {
		this.timeRemaining -= timeDelta;
		if(this.timeRemaining <= 0) {
			g_vehicle.addRebuke();
			this.timeRemaining = 14 * g_defs.timeInDay;
		}
		else if(this.timeRemaining <= 3 * g_defs.timeInDay && !this.wasWarning) {
			addMessage('Plan due soon', 'warning');
			this.wasWarning = true;
		}
	}
}
