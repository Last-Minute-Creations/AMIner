let g_rand = new rand(2184, 1911);
let g_tileMap = null;
let g_vehicle = null;

class TileIndex {
	static CAVE_BG_1 = 43
	static CAVE_BG_16 = 58
	static DIRT_1 = 59;
	static DIRT_2 = 60;
	static SILVER_1 = 61;
	static SILVER_2 = 62;
	static SILVER_3 = 63;
	static GOLD_1 = 64;
	static GOLD_2 = 65;
	static GOLD_3 = 66;
	static COAL_1 = 67;
	static COAL_2 = 68;
	static COAL_3 = 69;
	static MAGMA_1 = 70;
	static MAGMA_2 = 71;
	static STONE_1 = 72;
	static STONE_2 = 73;
	static STONE_3 = 74;
	static STONE_4 = 75;
	static EMERALD_1 = 76;
	static EMERALD_2 = 77;
	static EMERALD_3 = 78;
	static RUBY_1 = 79;
	static RUBY_2 = 80;
	static RUBY_3 = 81;
	static MOONSTONE_1 = 82;
	static MOONSTONE_2 = 83;
	static MOONSTONE_3 = 84;
	static BONE_HEAD = 85;
	static BONE_1 = 86;
}

class MineralType {
	static SILVER = 0;
	static GOLD = 1;
	static EMERALD = 2;
	static RUBY = 3;
	static MOONSTONE = 4;
	static DIRT = 5;
	static AIR = 6;
	static MAGMA = 7;
	static ROCK = 8;
	static COUNT = 9;

	static defs = [
		{reward: 5}, // SILVER
		{reward: 10}, // GOLD
		{reward: 15}, // EMERALD
		{reward: 20}, // RUBY
		{reward: 25}, // MOONSTONE
		{reward: 0}, // DIRT
		{reward: 0}, // AIR
		{reward: 0}, // MAGMA
		{reward: 0}, // ROCK
		{reward: 0}, // COUNT
	]
}

function rgbToHex(r, g, b) {
	return '#' +
		('0' + r.toString(16)).slice(-2) +
		('0' + g.toString(16)).slice(-2) +
		('0' + b.toString(16)).slice(-2);
}

class GroundLayer {
	constructor(pixelStartY, drillDifficulty, colorHex) {
		this.pixelStartY = pixelStartY;
		this.drillDifficulty = drillDifficulty;
		this.colorHex = colorHex;
	}

	static getLayerAt(posY) {
		for(let i = 0; i < this.layers.length - 1; ++i) {
			if(this.layers[i + 1].pixelStartY / 32 > posY) {
				return this.layers[i];
			}
		}
		this.layers[this.layers.length - 1];
	}

	static layers = [
		new GroundLayer(0, 2, rgbToHex(153, 68, 17)),
		new GroundLayer(2048 + 32, 3, rgbToHex(153, 102, 17)),
		new GroundLayer(4096 + 32, 4, rgbToHex(153, 102, 51)),
		new GroundLayer(6144 + 32, 5, rgbToHex(119, 102, 51)),
		new GroundLayer(8192 + 32, 6, rgbToHex(119, 102, 68)),
		new GroundLayer(65535, 0, rgbToHex(119, 102, 68)) // unreachable
	]
}

class Tile {
	constructor(index, mineralType, mineralAmount) {
		this.index = index;
		this.mineralType = mineralType;
		this.mineralAmount = mineralAmount;
	}

	isHardToDrill() {
		return this.index >= TileIndex.STONE_1;
	}
}

class TileMap {
	constructor(width, height) {
		this.tiles = new Array(width).fill(0).map(() => new Array(height).fill(0));
		this.width = width;
		this.height = height;
	}

	isSolid(x, y) {
		return this.tiles[x][y].index >= TileIndex.DIRT_1;
	}
}

class Vehicle {
	constructor() {
		this.hullMk = 0;
		this.cargoMk = 0;
		this.drillMk = 0;
		this.money = 0;
		this.cargoMoney = 0;

		this.hullMax = 100;
		this.cargoMax = 50;
		this.drillMax = 1000;

		this.hullCurr = this.hullMax;
		this.cargoCurr = 0;
		this.drillCurr = this.drillMax;

		this.restock();
	}

	tryExcavate(tile, posY) {
		let drillUnitCost = 15;
		let drillLevel = this.drillMk;
		let difficulty = tile.isHardToDrill() ? 10 : GroundLayer.getLayerAt(posY).drillDifficulty;
		let drillDuration = Math.max(1, difficulty - drillLevel);
		let drillCost = drillUnitCost * drillDuration;

		if(this.drillCurr < drillCost) {
			return false;
		}

		this.drillCurr -= drillCost;
		this.cargoMoney += tile.mineralAmount * MineralType.defs[tile.mineralType].reward;
		this.cargoCurr = Math.min(this.cargoCurr + tile.mineralAmount, this.cargoMax);
		return true;
	}

	restock() {
		let hullPrice = 2;
		let literPrice = 5;
		let fuelInLiter = 100;

		this.money += this.cargoMoney;
		this.cargoMoney = 0;

		let liters = Math.floor((this.drillMax - this.drillCurr + 0.5 * fuelInLiter) / fuelInLiter);
		this.money -= (this.hullMax - this.hullCurr) * hullPrice;
		this.money -= liters * literPrice;

		this.hullCurr = this.hullMax;
		this.cargoCurr = 0;
		this.drillCurr = Math.min(this.drillCurr + liters * fuelInLiter, this.drillMax);
	}
}

function clamp(x, min, max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

function chanceTrapezoid(curr, start, peakStart, peakEnd, end, min, max) {
	if(start < curr && curr < peakStart) {
		// Ascending ramp
		return min + ((max - min) * (curr - start)) / (peakStart - start);
	}
	if(peakStart <= curr && curr <= peakEnd) {
		// Peak
		return max;
	}
	if(peakEnd < curr && curr < end) {
		// Descending ramp
		return max - ((max - min) * (curr - peakEnd)) / (end - peakEnd);
	}
	// Out of range
	return min;
}

function tileGenerate(rand) {
	let endX = 11;
	let endY = 1024;

	let mineralCounts = new Array(MineralType.COUNT).fill(0);
	let tileMap = new TileMap(endX, endY);
	let tileRowBaseDirt = 8;

	for(let x = 1; x < endX; ++x) {
		for(let y = tileRowBaseDirt + 2; y < endY; ++y) {
			// 2000 is max
			let what = Math.floor((g_rand.next16() * 1000) / 65535);
			let chanceAir = 50;
			let chanceRock, chanceSilver, chanceGold, chanceEmerald, chanceRuby, chanceMoonstone, chanceMagma;
			chanceRock = clamp(y * 500 / 2000, 0, 500);
			chanceSilver = 75;
			chanceGold = y > 60 ? 75 : 0;
			chanceEmerald = y > 200 ? 75 : 0;
			chanceRuby = y  > 400 ? 75 : 0;
			chanceMoonstone = y > 175 ? 75 : 0;
			chanceMagma = chanceTrapezoid(y, 50, 900, 1000, 1100, 0, 75);

			let chance;
			if(what < (chance = chanceRock)) {
				tileMap.tiles[x][y] = new Tile(g_rand.next16MinMax(TileIndex.STONE_1, TileIndex.STONE_4), MineralType.ROCK, 0);
			}
			else if(what < (chance += chanceMagma)) {
				tileMap.tiles[x][y] = new Tile(g_rand.next16MinMax(TileIndex.MAGMA_1, TileIndex.MAGMA_2), MineralType.MAGMA, 0);
			}
			else if(
				what < (chance += chanceAir) &&
				tileMap.isSolid(x - 1, y) && tileMap.isSolid(x, y - 1)
			) {
				tileMap.tiles[x][y] = new Tile(TileIndex.CAVE_BG_1+15, MineralType.AIR, 0);
			}
			else if(what < (chance += chanceSilver)) {
				let add = g_rand.next16MinMax(0, 2);
				mineralCounts[MineralType.SILVER] += 1 + add;
				tileMap.tiles[x][y] = new Tile(TileIndex.SILVER_1 + add, MineralType.SILVER, 1 + add);
			}
			else if(what < (chance += chanceGold)) {
				let add = g_rand.next16MinMax(0, 2);
				mineralCounts[MineralType.GOLD] += 1 + add;
				tileMap.tiles[x][y] = new Tile(TileIndex.GOLD_1 + add, MineralType.GOLD, 1 + add);
			}
			else if(what < (chance += chanceEmerald)) {
				let add = g_rand.next16MinMax(0, 2);
				mineralCounts[MineralType.EMERALD] += 1 + add;
				tileMap.tiles[x][y] = new Tile(TileIndex.EMERALD_1 + add, MineralType.EMERALD, 1 + add);
			}
			else if(what < (chance += chanceRuby)) {
				let add = g_rand.next16MinMax(0, 2);
				mineralCounts[MineralType.RUBY] += 1 + add;
				tileMap.tiles[x][y] = new Tile(TileIndex.RUBY_1 + add, MineralType.RUBY, 1 + add);
			}
			else if(what < (chance += chanceMoonstone)) {
				let add = g_rand.next16MinMax(0, 2);
				mineralCounts[MineralType.MOONSTONE] += 1 + add;
				tileMap.tiles[x][y] = new Tile(TileIndex.MOONSTONE_1 + add, MineralType.MOONSTONE, 1 + add);
			}
			else {
				tileMap.tiles[x][y] = new Tile(TileIndex.DIRT_1 + ((x & 1) ^ (y & 1)), MineralType.DIRT, 0);
			}
		}
	}

	// Draw bases
	// for(let ubBase = 0; ubBase < BASE_ID_COUNT; ++ubBase) {
	// 	let base = s_bases[ubBase];
	// 	if(base.level != BASE_LEVEL_letIANT) {
	// 		tileDrawBase(base, base.level);
	// 	}
	// }

	// Fill left invisible col with rocks
	for(let y = 0; y < endY; ++y) {
		tileMap.tiles[0][y] = new Tile(TileIndex.STONE_1, MineralType.ROCK, 0);
	}

	// Rock bottom
	for(let x = 1; x < endX; ++x) {
		tileMap.tiles[x][endY - 1] = new Tile(TileIndex.STONE_1 + (x & 3), MineralType.ROCK, 0);
	}

	// Dino bones
	// tileMap.tiles[5][g_dinoDepths[0]] = new Tile(TileIndex.BONE_HEAD, MineralType.COUNT, 0);
	// tileMap.tiles[3][g_dinoDepths[1]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
	// tileMap.tiles[7][g_dinoDepths[2]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
	// tileMap.tiles[1][g_dinoDepths[3]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
	// tileMap.tiles[4][g_dinoDepths[4]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
	// tileMap.tiles[6][g_dinoDepths[5]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
	// tileMap.tiles[8][g_dinoDepths[6]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
	// tileMap.tiles[2][g_dinoDepths[7]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
	// tileMap.tiles[9][g_dinoDepths[8]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);

	return tileMap;
}

function onCellTryExcavate(evt) {
	evt.preventDefault();
	if(!(evt.buttons & 1)) {
		return;
	}

	var cell = evt.target;
	var posX = parseInt(cell.dataset.posX);
	var posY = parseInt(cell.dataset.posY);
	if(!g_tileMap.isSolid(posX, posY)) {
		return;
	}

	let isExcavatable = (
		// from left
		(posX && !g_tileMap.isSolid(posX - 1, posY) && g_tileMap.isSolid(posX - 1, posY  + 1)) ||
		// from right
		(posX < g_tileMap.width - 1 && !g_tileMap.isSolid(posX + 1, posY) && g_tileMap.isSolid(posX + 1, posY  + 1)) ||
		// from up
		(!g_tileMap.isSolid(posX, posY - 1))
	);
	if(!isExcavatable) {
		return;
	}

	var tile = g_tileMap.tiles[posX][posY];
	if(g_vehicle.tryExcavate(tile, posY)) {
		console.log(`excavated at ${posX},${posY}`);

		// Update data
		g_tileMap.tiles[posX][posY] = new Tile(TileIndex.CAVE_BG_16,  MineralType.AIR, 0);
		// Update view
		cell.setAttribute('class', '');
		cell.classList.add('tile_bg');
		cell.textContent = "";

		updateVehicleStats();
	}
}

function updateVehicleStats() {
	document.querySelector("#vehicle_hull_mk").textContent = g_vehicle.hullMk + 1;
	document.querySelector("#vehicle_hull_curr").textContent = g_vehicle.hullCurr;
	document.querySelector("#vehicle_hull_max").textContent = g_vehicle.hullMax;

	document.querySelector("#vehicle_cargo_mk").textContent = g_vehicle.cargoMk + 1;
	document.querySelector("#vehicle_cargo_curr").textContent = g_vehicle.cargoCurr;
	document.querySelector("#vehicle_cargo_max").textContent = g_vehicle.cargoMax;
	document.querySelector("#vehicle_cargo_money").textContent = g_vehicle.cargoMoney;

	document.querySelector("#vehicle_drill_mk").textContent = g_vehicle.drillMk + 1;
	document.querySelector("#vehicle_drill_curr").textContent = g_vehicle.drillCurr;
	document.querySelector("#vehicle_drill_max").textContent = g_vehicle.drillMax;

	document.querySelector("#vehicle_money").textContent = g_vehicle.money;
}

function updateCellEvents(cell) {
	cell.addEventListener('mousedown', onCellTryExcavate);
	cell.addEventListener('mousemove', onCellTryExcavate);
}

function drawTiles(tileMap) {
	let table = document.querySelector("#mine_preview");
	table.innerHTML = '';

	let sizeX = tileMap.tiles.length - 1; // skip dummy column on the left
	let sizeY = tileMap.tiles[0].length;

	for(let y = 0; y < sizeY; ++y) {
		let tr = document.createElement("tr");
		for(let x = 1; x < sizeX; ++x) {
			let td = document.createElement("td");
			if(tileMap.tiles[x][y].mineralType == MineralType.DIRT) {
				td.classList.add('tile_dirt');
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.SILVER) {
				td.classList.add('tile_silver');
				td.textContent = "S" + tileMap.tiles[x][y].mineralAmount;
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.GOLD) {
				td.classList.add('tile_gold');
				td.textContent = "G" + tileMap.tiles[x][y].mineralAmount;
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.EMERALD) {
				td.classList.add('tile_emerald');
				td.textContent = "E" + tileMap.tiles[x][y].mineralAmount;
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.RUBY) {
				td.classList.add('tile_ruby');
				td.textContent = "R" + tileMap.tiles[x][y].mineralAmount;
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.MOONSTONE) {
				td.classList.add('tile_moonstone');
				td.textContent = "M" + tileMap.tiles[x][y].mineralAmount;
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.AIR) {
				td.classList.add('tile_bg');
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.MAGMA) {
				td.classList.add('tile_magma');
				td.textContent = "🔥";
			}
			else if(tileMap.tiles[x][y].mineralType == MineralType.ROCK) {
				td.classList.add('tile_stone');
				td.textContent = "R";
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
			updateCellEvents(td);

			tr.appendChild(td);
		}
		let tdRowIndex = document.createElement("td");
		tdRowIndex.textContent = y;
		tdRowIndex.classList.add("row_index");
		tr.appendChild(tdRowIndex);
		table.appendChild(tr);
	}
}

function onRestockClicked(evt) {
	g_vehicle.restock();
	updateVehicleStats();
}

window.addEventListener("load", function() {
	document.querySelector("#btn_vehicle_restock").addEventListener("click", onRestockClicked);

	g_tileMap = tileGenerate(g_rand);
	g_vehicle = new Vehicle();
	drawTiles(g_tileMap);
	updateVehicleStats();
});
