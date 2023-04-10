let g_rand = null;
let g_tileMap = null;
let g_vehicle = null;
let g_plan = null;
let g_defs = null;

function onCellTryExcavate(evt) {
	evt.preventDefault();
	if(!(evt.buttons & 1) || g_vehicle.isGameOver()) {
		return;
	}

	let cell = evt.target;
	let posX = parseInt(cell.dataset.posX);
	let posY = parseInt(cell.dataset.posY);

	if(g_vehicle.tryExcavate(posX, posY)) {
		console.log(`excavated at ${posX},${posY}`);

		// Update view
		cell.setAttribute('class', '');
		cell.classList.add('tile_bg');
		cell.textContent = '';

		updateVehicleStats();
		updateMineralStats();
		updateOfficeStats();
	}
}

function updateVehicleStats() {
	document.querySelector('#vehicle_hull_mk').textContent = g_vehicle.hullMk + 1;
	document.querySelector('#vehicle_hull_curr').textContent = g_vehicle.hullCurr;
	document.querySelector('#vehicle_hull_max').textContent = g_vehicle.hullMax;

	document.querySelector('#vehicle_cargo_mk').textContent = g_vehicle.cargoMk + 1;
	document.querySelector('#vehicle_cargo_curr').textContent = g_vehicle.cargoCurr;
	document.querySelector('#vehicle_cargo_max').textContent = g_vehicle.cargoMax;

	document.querySelector('#vehicle_drill_mk').textContent = g_vehicle.drillMk + 1;
	document.querySelector('#vehicle_drill_curr').textContent = g_vehicle.drillCurr;
	document.querySelector('#vehicle_drill_max').textContent = g_vehicle.drillMax;

	document.querySelector('#vehicle_money').textContent = g_vehicle.money;
}

function setMineCellEvents(cell) {
	cell.addEventListener('mousedown', onCellTryExcavate);
	cell.addEventListener('mousemove', onCellTryExcavate);
}

function updateWarehouse() {
	for(let mineralType of MineralType.collectibles) {
		let name = mineralType.name;
		document.querySelector(`#stock_${name}`).textContent = g_vehicle.stock[mineralType.id];
		document.querySelector(`#plan_${name}_spent`).textContent = g_plan.mineralsCollected[mineralType.id];
		document.querySelector(`#plan_${name}_required`).textContent = g_plan.mineralsRequired[mineralType.id];
	}
}

function drawTiles(tileMap) {
	let table = document.querySelector('#mine_preview');
	table.innerHTML = '';

	let sizeX = tileMap.tiles.length - 1; // skip dummy column on the left
	let sizeY = tileMap.tiles[0].length;

	for(let y = 0; y < sizeY; ++y) {
		let tr = document.createElement('tr');
		for(let x = 1; x < sizeX; ++x) {
			let td = document.createElement('td');
			if(tileMap.tiles[x][y].mineralType == MineralType.DIRT) {
				td.classList.add('tile_dirt');
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.SILVER) {
				td.classList.add('tile_silver');
				td.textContent = 'S' + tileMap.tiles[x][y].mineralAmount;
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.GOLD) {
				td.classList.add('tile_gold');
				td.textContent = 'G' + tileMap.tiles[x][y].mineralAmount;
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.EMERALD) {
				td.classList.add('tile_emerald');
				td.textContent = 'E' + tileMap.tiles[x][y].mineralAmount;
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.RUBY) {
				td.classList.add('tile_ruby');
				td.textContent = 'R' + tileMap.tiles[x][y].mineralAmount;
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.MOONSTONE) {
				td.classList.add('tile_moonstone');
				td.textContent = 'M' + tileMap.tiles[x][y].mineralAmount;
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.AIR) {
				td.classList.add('tile_bg');
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.MAGMA) {
				td.classList.add('tile_magma');
				td.textContent = '🔥';
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.ROCK) {
				td.classList.add('tile_stone');
				td.textContent = 'R';
			}
			else {
				td.textContent = tileMap.tiles[x][y].index;
			}
			if(tileMap.isSolid(x, y)) {
				var colorHex = GroundLayer.getLayerAt(y).colorHex;
				td.style = `background-color: ${colorHex};`;
			}

			td.dataset.posX = x;
			td.dataset.posY = y;
			setMineCellEvents(td);

			tr.appendChild(td);
		}

		let tdRowIndex = document.createElement('td');
		tdRowIndex.textContent = y;
		tdRowIndex.classList.add('row_index');
		tr.appendChild(tdRowIndex);

		let tdLayerName = document.createElement('td');
		tdLayerName.textContent = GroundLayer.getLayerAt(y).name;
		tdLayerName.classList.add('row_index');
		tr.appendChild(tdLayerName);

		table.appendChild(tr);
	}
}

function onSellClicked(mineralType, amount) {
	if(g_vehicle.isGameOver()) {
		return;
	}

	g_vehicle.trySell(mineralType, amount);
	updateVehicleStats();
	updateWarehouse();
}

function onSellAllClicked() {
	if(g_vehicle.isGameOver()) {
		return;
	}

	for(let mineralType of MineralType.all) {
		g_vehicle.trySell(mineralType, 1000000);
	}
	updateVehicleStats();
	updateWarehouse();
}

function onPlanFillClicked(mineralType, amount) {
	if(g_vehicle.isGameOver()) {
		return;
	}

	g_vehicle.tryFillPlan(mineralType, amount);
	updateVehicleStats();
	updateWarehouse();
	updateOfficeStats();
}

function onFillAllPlanClicked() {
	if(g_vehicle.isGameOver()) {
		return;
	}

	for(let mineralType of MineralType.all) {
		g_vehicle.tryFillPlan(mineralType, 1000000);
	}
	updateVehicleStats();
	updateWarehouse();
	updateOfficeStats();
}

function onRestockClicked(evt) {
	if(g_vehicle.isGameOver()) {
		return;
	}

	g_vehicle.restock();
	updateVehicleStats();
	updateWarehouse();
}

function updateMineralStats() {
	for(let mineralType of MineralType.collectibles) {
		let name = mineralType.name;
		document.querySelector(`#minerals_${name}_total`).textContent = g_tileMap.totalMineralCounts[mineralType.id];
		document.querySelector(`#minerals_${name}`).textContent = g_tileMap.currentMineralCounts[mineralType.id];
	}
}

function updateOfficeStats() {
	document.querySelector('#office_ending').textContent = g_vehicle.ending.description;
	document.querySelector('#office_ending').className = g_vehicle.ending.className;

	document.querySelector('#plan_index').textContent = g_plan.index + 1;
	document.querySelector('#plan_started').textContent = g_plan.isStarted ? '✓' : '✗';
	document.querySelector('#plan_extended').textContent = g_plan.isExtendedByFavor ? '✓' : '✗';
	document.querySelector('#plan_time_remaining').textContent = g_plan.timeRemaining;
	document.querySelector('#plan_time_remaining_days').textContent = (g_plan.timeRemaining / g_defs.timeInDay).toFixed(2);
	document.querySelector('#plan_unlocked_minerals').textContent = g_plan.mineralsUnlocked.map((x) => MineralType.all[x].name).join(', ');
	document.querySelector('#office_rebukes_curr').textContent = g_vehicle.rebukes;
	document.querySelector('#office_rebukes_max').textContent = g_defs.maxRebukes;
	document.querySelector('#office_accolades_curr').textContent = g_vehicle.accolades;
	document.querySelector('#office_accolades_max').textContent = g_defs.maxAccolades;
	document.querySelector('#office_accolades_progress_curr').textContent = g_vehicle.subAccolades;
	document.querySelector('#office_accolades_progress_max').textContent = g_defs.maxSubAccolades;
}

window.addEventListener('load', function() {
	document.querySelector('#btn_vehicle_restock').addEventListener('click', onRestockClicked);

	for(let mineralType of MineralType.collectibles) {
		let name = mineralType.name;
		document.querySelector(`#${name}_price`).textContent = mineralType.reward;
		document.querySelector(`#btn_${name}_sell_1`).addEventListener('click', function() {onSellClicked(mineralType, 1); });
		document.querySelector(`#btn_${name}_sell_10`).addEventListener('click', function() {onSellClicked(mineralType, 10); });
		document.querySelector(`#btn_${name}_plan_1`).addEventListener('click', function() {onPlanFillClicked(mineralType, 1); });
		document.querySelector(`#btn_${name}_plan_fill`).addEventListener('click', function() {onPlanFillClicked(mineralType, 1000000); });
	}
	document.querySelector('#btn_sell_all').addEventListener('click', function() {onSellAllClicked(); });
	document.querySelector('#btn_fill_plan').addEventListener('click', function() {onFillAllPlanClicked(); });

	g_defs = new Defs();
	g_rand = new rand(g_defs.seed1, g_defs.seed2);
	g_tileMap = new TileMap(11, 1024);
	g_vehicle = new Vehicle();
	g_plan = new Plan();
	drawTiles(g_tileMap);
	updateVehicleStats();
	updateMineralStats();
	updateOfficeStats();
	updateWarehouse();
});
