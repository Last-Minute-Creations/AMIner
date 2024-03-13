class StrikeState {
	static NONE = {description: 'ok', className: 'strike_good', level: 0, isHullRepair: true};
	static WARNING = {description: 'warning', className: 'strike_bad', level: 1, isHullRepair: true};
	static ACTIVE = {description: 'active', className: 'strike_bad', level: 2, isHullRepair: false};
	static UPRISING = {description: 'uprising', className: 'strike_bad', level: 3,isHullRepair: false};
}

