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
				let chanceRock, chanceMagma;
				chanceRock = Utils.clamp(y * 500 / 2000, 0, 500);
				chanceMagma = Utils.chanceTrapezoid(y, 50, 900, 1000, 1100, 0, 75);

				////////////////////////////////////////////////////////////////////////
				let chanceSilver, chanceGold, chanceEmerald, chanceRuby, chanceMoonstone;
				chanceSilver = y > g_defs.depthSilver ? 75 : 0;
				chanceGold = y > g_defs.depthGold ? 75 : 0;
				chanceEmerald = y > g_defs.depthEmerald ? 75 : 0;
				chanceRuby = y  > g_defs.depthRuby ? 75 : 0;
				chanceMoonstone = y > g_defs.depthMoonstone ? 75 : 0;
				////////////////////////////////////////////////////////////////////////

				let chance;
				if(what < (chance = chanceRock)) {
					this.tiles[x][y] = new Tile(g_rand.next16MinMax(TileIndex.STONE_1, TileIndex.STONE_4));
				}
				else if(what < (chance += chanceMagma)) {
					this.tiles[x][y] = new Tile(g_rand.next16MinMax(TileIndex.MAGMA_1, TileIndex.MAGMA_2));
				}
				else if(
					what < (chance += chanceAir) &&
					this.isSolid(x - 1, y) && this.isSolid(x, y - 1)
				) {
					this.tiles[x][y] = new Tile(TileIndex.CAVE_BG_1+15);
				}
				////////////////////////////////////////////////////////////////////////
				else if(what < (chance += chanceSilver)) {
					let add = g_rand.next16MinMax(0, 2);
					this.tiles[x][y] = new Tile(TileIndex.SILVER_1 + add);
				}
				else if(what < (chance += chanceGold)) {
					let add = g_rand.next16MinMax(0, 2);
					this.tiles[x][y] = new Tile(TileIndex.GOLD_1 + add);
				}
				else if(what < (chance += chanceEmerald)) {
					let add = g_rand.next16MinMax(0, 2);
					this.tiles[x][y] = new Tile(TileIndex.EMERALD_1 + add);
				}
				else if(what < (chance += chanceRuby)) {
					let add = g_rand.next16MinMax(0, 2);
					this.tiles[x][y] = new Tile(TileIndex.RUBY_1 + add);
				}
				else if(what < (chance += chanceMoonstone)) {
					let add = g_rand.next16MinMax(0, 2);
					this.tiles[x][y] = new Tile(TileIndex.MOONSTONE_1 + add);
				}
				////////////////////////////////////////////////////////////////////////
				else {
					this.tiles[x][y] = new Tile(TileIndex.DIRT_1 + ((x & 1) ^ (y & 1)));
				}
			}
		}

		// Draw bases
		for(let baseIndex = 0; baseIndex < TileMap.bases.length; ++baseIndex) {
			let base = TileMap.bases[baseIndex];
			for(let y = 0; y < base.pattern.length; ++y) {
				for(let x = 0; x < width; ++x) {
					this.tiles[x][base.level + y] = new Tile(base.pattern[y][x]);
				}
			}
		}

		// Fill left invisible col with rocks
		for(let y = 0; y < height; ++y) {
			this.tiles[0][y] = new Tile(TileIndex.STONE_1, MineralType.ROCK, 0);
		}

		// Rock bottom
		for(let x = 1; x < width; ++x) {
			this.tiles[x][height - 1] = new Tile(TileIndex.BASE_GROUND_1, MineralType.ROCK, 0);
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
		this.totalMoney = 0;
		for(let mineralType of MineralType.all) {
			this.totalMineralCounts[mineralType.id] = 0;
		}
		for(let x = 1; x < width; ++x) {
			for(let y = 0; y < height; ++y) {
				this.totalMineralCounts[this.tiles[x][y].mineralType.id] += this.tiles[x][y].mineralAmount;
				this.totalMoney += this.tiles[x][y].mineralAmount * this.tiles[x][y].mineralType.reward;
			}
		}

		this.currentMineralCounts = {};
		for(let mineralType of MineralType.all) {
			this.currentMineralCounts[mineralType.id] = this.totalMineralCounts[mineralType.id];
		}
		this.currentMoney = this.totalMoney;
	}


	isSolid(x, y) {
		return this.tiles[x][y].index >= TileIndex.DIRT_1;
	}

	static bases = [
		{
			level: 0,
			pattern: [
				[ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
				[ 1,  1,  1,  1,  1,  1,  1,  1,  1,  1],
				[ 2,  2,  2,  2,  2,  2,  2,  2,  2,  2],
				[ 3,  3,  3,  3,  3,  3,  3,  3,  3,  3],
				[ 4,  4,  4,  4,  4,  4,  4,  4,  4,  5],
				[ 6,  6,  7,  6,  6,  8,  9, 10, 11, 12],
				[13, 14, 15, 16, 17, 18, 19, 20, 21, 22],
				[23, 24, 25, 26, 27, 28, 29, 30, 31, 32],
				[34, 33, 35, 36, 37, 38, 39, 40, 41, 42],
				[63, 57, 63, 64, 63, 64, 63, 64, 63, 64],
			]
		},
		{
			level: 100,
			pattern: [
				[44, 44, 44, 44, 44, 44, 44, 44, 44, 44],
				[ 0, 43, 43, 43, 43,  1, 43,  0, 43, 43],
				[43,  1, 43, 43, 43, 43, 43, 43, 43,  2],
				[43, 43, 43, 43, 43,  3,  1, 43, 43,  1],
				[43, 43, 43, 43,  3, 43, 43, 43,  0, 43],
				[43,  1, 43, 43, 43, 43, 43, 43, 43,  2],
				[13, 14, 15, 16, 17, 18, 19, 20, 21, 22],
				[23, 24, 25, 26, 27, 28, 29, 30, 31, 32],
				[34, 35, 36, 37, 38, 33, 39, 40, 41, 42],
				[63, 64, 63, 64, 63, 57, 63, 64, 63, 64],
			]
		},
		{
			level: 209,
			pattern: [
				[44, 44, 44, 44, 44, 44, 44, 44, 44, 44],
				[ 0, 43, 43, 43, 43,  1, 43,  0, 43, 43],
				[43,  1, 43, 43, 43, 43, 43, 43, 43,  2],
				[43, 43, 43, 43, 43,  3,  1, 43, 43,  1],
				[ 4,  5, 43, 43,  6,  5, 43, 43,  0, 43],
				[ 7,  8,  1, 43,  9, 10,  2, 11, 43, 43],
				[12, 13, 43, 43, 14, 15, 16, 17, 18, 19],
				[20, 21, 22, 23, 24, 25, 26, 27, 28, 29],
				[34, 35, 36, 37, 38, 33, 39, 40, 41, 42],
				[63, 64, 63, 64, 63, 57, 63, 64, 63, 64],
			]
		},
	];

}
