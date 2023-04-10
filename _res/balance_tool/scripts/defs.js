class Defs {
	constructor() {
		this.timeInDay = 140;
		this.maxRebukes = parseInt(document.querySelector('[name=defs_rebukes]').value);
		this.maxSubAccolades = parseInt(document.querySelector('[name=defs_plans_per_accoldade]').value);
		this.maxAccolades = parseInt(document.querySelector('[name=defs_accolades]').value);
		this.hullPrice = 2;
		this.literPrice = 5;
		this.fuelInLiter = 100;
		this.planCostMultiplier = parseFloat(document.querySelector('[name=defs_plan_value_ratio]').value) - 1;
		this.seed1 = parseInt(document.querySelector('[name=defs_seed1]').value);
		this.seed2 = parseInt(document.querySelector('[name=defs_seed2]').value);
	}
}
