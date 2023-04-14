class Plan {
	constructor(roll = true) {
		this.mineralsRequired = new Array(MineralType.all.length).fill(0); // [mineralId] => count
		this.mineralsCollected = new Array(MineralType.all.length).fill(0); // [mineralId] => count
		this.mineralsUnlocked = [ MineralType.SILVER.id ]; // [mineralId]
		this.index = 0;
		this.isStarted = true;
		this.isExtendedByFavor = false;
		this.targetSum = 15;

		if(roll) {
			this.reroll();
		}
	}

	reroll() {
		this.mineralsRequired = new Array(MineralType.all.length).fill(0); // [mineralId] => count
		this.mineralsCollected = new Array(MineralType.all.length).fill(0); // [mineralId] => count
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

		this.timeRemaining = 2 * 2 * 1000; // two full fuel, per player
		this.timeRemaining += 200; // Add for nice division into 30 days
		this.wasWarning = false;
	}

	next(roll = true) {
		++this.index;
		this.targetSum += Math.floor(this.targetSum * g_defs.planCostMultiplier);
		if(roll) {
			this.reroll();
		}
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
			this.reroll();
		}
		else if(this.timeRemaining <= 3 * g_defs.timeInDay && !this.wasWarning) {
			addMessage('Plan due soon', 'warning');
			this.wasWarning = true;
		}
	}
}
