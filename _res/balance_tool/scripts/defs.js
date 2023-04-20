class Defs {
	constructor() {
		this.timeInDay = 140;
		this.maxRebukes = parseInt(document.querySelector('[name=defs_rebukes]').value);
		this.maxSubAccolades = parseInt(document.querySelector('[name=defs_plans_per_accoldade]').value);
		this.maxAccolades = parseInt(document.querySelector('[name=defs_accolades]').value);
		this.extraPlanMoney = parseInt(document.querySelector('[name=extra_plan_money]').value);
		this.hullPrice = 2;
		this.literPrice = 5;
		this.fuelInLiter = 100;
		this.seed1 = parseInt(document.querySelector('[name=defs_seed1]').value);
		this.seed2 = parseInt(document.querySelector('[name=defs_seed2]').value);
		this.upgradeCosts = document.querySelector('[name=defs_upgrade_costs]').value.split(',').map((x) => parseInt(x));
		this.hullBase = 100;
		this.hullAddPerLevel = 20;
		this.drillBase = 1000;
		this.drillAddPerLevel = 250;
		this.cargoBase = 50;
		this.cargoAddPerLevel = 20;
		this.damageAfterRestock = 10;

		this.planSilver = 0;
		this.planGold = 10;
		this.planEmerald = 30;
		this.planRuby = 40;
		this.planMoonstone = 20;

		// TODO: remove? leave for free resource pool?
		this.depthSilver = 0;
		this.depthGold = 60;
		this.depthEmerald = 200;
		this.depthRuby = 400;
		this.depthMoonstone = 175;
	}
}
