class TileMap {
	constructor(width, height) {
		this.tiles = new Array(width).fill(0).map(() => new Array(height).fill(0));
		this.width = width;
		this.height = height;

		let tileRowBaseDirt = 8;

		for(let x = 1; x < width; ++x) {
			for(let y = tileRowBaseDirt + 2; y < height; ++y) {
				// 2000 is max
				let what = Math.floor((g_rand.next16() * 1000) / 65535);
				let chanceAir = 50;
				let chanceRock, chanceSilver, chanceGold, chanceEmerald, chanceRuby, chanceMoonstone, chanceMagma;
				chanceRock = Utils.clamp(y * 500 / 2000, 0, 500);
				chanceSilver = 75;
				chanceGold = y > 60 ? 75 : 0;
				chanceEmerald = y > 200 ? 75 : 0;
				chanceRuby = y  > 400 ? 75 : 0;
				chanceMoonstone = y > 175 ? 75 : 0;
				chanceMagma = Utils.chanceTrapezoid(y, 50, 900, 1000, 1100, 0, 75);

				let chance;
				if(what < (chance = chanceRock)) {
					this.tiles[x][y] = new Tile(g_rand.next16MinMax(TileIndex.STONE_1, TileIndex.STONE_4), MineralType.ROCK, 0);
				}
				else if(what < (chance += chanceMagma)) {
					this.tiles[x][y] = new Tile(g_rand.next16MinMax(TileIndex.MAGMA_1, TileIndex.MAGMA_2), MineralType.MAGMA, 0);
				}
				else if(
					what < (chance += chanceAir) &&
					this.isSolid(x - 1, y) && this.isSolid(x, y - 1)
				) {
					this.tiles[x][y] = new Tile(TileIndex.CAVE_BG_1+15, MineralType.AIR, 0);
				}
				else if(what < (chance += chanceSilver)) {
					let add = g_rand.next16MinMax(0, 2);
					this.tiles[x][y] = new Tile(TileIndex.SILVER_1 + add, MineralType.SILVER, 1 + add);
				}
				else if(what < (chance += chanceGold)) {
					let add = g_rand.next16MinMax(0, 2);
					this.tiles[x][y] = new Tile(TileIndex.GOLD_1 + add, MineralType.GOLD, 1 + add);
				}
				else if(what < (chance += chanceEmerald)) {
					let add = g_rand.next16MinMax(0, 2);
					this.tiles[x][y] = new Tile(TileIndex.EMERALD_1 + add, MineralType.EMERALD, 1 + add);
				}
				else if(what < (chance += chanceRuby)) {
					let add = g_rand.next16MinMax(0, 2);
					this.tiles[x][y] = new Tile(TileIndex.RUBY_1 + add, MineralType.RUBY, 1 + add);
				}
				else if(what < (chance += chanceMoonstone)) {
					let add = g_rand.next16MinMax(0, 2);
					this.tiles[x][y] = new Tile(TileIndex.MOONSTONE_1 + add, MineralType.MOONSTONE, 1 + add);
				}
				else {
					this.tiles[x][y] = new Tile(TileIndex.DIRT_1 + ((x & 1) ^ (y & 1)), MineralType.DIRT, 0);
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
		for(let y = 0; y < height; ++y) {
			this.tiles[0][y] = new Tile(TileIndex.STONE_1, MineralType.ROCK, 0);
		}

		// Rock bottom
		for(let x = 1; x < width; ++x) {
			this.tiles[x][height - 1] = new Tile(TileIndex.STONE_1 + (x & 3), MineralType.ROCK, 0);
		}

		// Dino bones
		// this.tiles[5][g_dinoDepths[0]] = new Tile(TileIndex.BONE_HEAD, MineralType.COUNT, 0);
		// this.tiles[3][g_dinoDepths[1]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
		// this.tiles[7][g_dinoDepths[2]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
		// this.tiles[1][g_dinoDepths[3]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
		// this.tiles[4][g_dinoDepths[4]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
		// this.tiles[6][g_dinoDepths[5]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
		// this.tiles[8][g_dinoDepths[6]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
		// this.tiles[2][g_dinoDepths[7]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);
		// this.tiles[9][g_dinoDepths[8]] = new Tile(TileIndex.BONE_1, MineralType.COUNT, 0);

		this.totalMineralCounts = {};
		for(let mineralType of MineralType.all) {
			this.totalMineralCounts[mineralType.id] = 0;
		}
		for(let x = 1; x < width; ++x) {
			for(let y = tileRowBaseDirt + 2; y < height; ++y) {
				this.totalMineralCounts[this.tiles[x][y].mineralType.id] += this.tiles[x][y].mineralAmount;
			}
		}

		this.currentMineralCounts = {};
		for(let mineralType of MineralType.all) {
			this.currentMineralCounts[mineralType.id] = this.totalMineralCounts[mineralType.id];
		}
	}


	isSolid(x, y) {
		return this.tiles[x][y].index >= TileIndex.DIRT_1;
	}
}
