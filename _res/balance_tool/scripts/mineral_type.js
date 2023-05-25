class MineralType {
	static SILVER = {id: 0, name: 'silver', isGate: false, isDino: false, isCollectible: true, reward: 5, isDamaging: false, tileIndex: TileIndex.SILVER_1};
	static GOLD = {id: 1, name: 'gold', isGate: false, isDino: false, isCollectible: true, reward: 10, isDamaging: false, tileIndex: TileIndex.GOLD_1};
	static EMERALD = {id: 2, name: 'emerald', isGate: false, isDino: false, isCollectible: true, reward: 15, isDamaging: false, tileIndex: TileIndex.EMERALD_1};
	static RUBY = {id: 3, name: 'ruby', isGate: false, isDino: false, isCollectible: true, reward: 20, isDamaging: false, tileIndex: TileIndex.RUBY_1};
	static MOONSTONE = {id: 4, name: 'moonstone', isGate: false, isDino: false, isCollectible: true, reward: 25, isDamaging: false, tileIndex: TileIndex.MOONSTONE_1};
	static DIRT = {id: 5, name: 'dirt', isGate: false, isDino: false, isCollectible: false, reward: 0, isDamaging: false, tileIndex: TileIndex.DIRT_1};
	static AIR = {id: 6, name: 'air', isGate: false, isDino: false, isCollectible: false, reward: 0, isDamaging: false, tileIndex: TileIndex.CAVE_BG_1};
	static MAGMA = {id: 7, name: 'magma', isGate: false, isDino: false, isCollectible: false, reward: 0, isDamaging: true, tileIndex: TileIndex.MAGMA_1};
	static ROCK = {id: 8, name: 'rock', isGate: false, isDino: false, isCollectible: false, reward: 0, isDamaging: false, tileIndex: TileIndex.STONE_1};
	static BONE = {id: 9, name: 'bone', isGate: false, isDino: true, isCollectible: false, reward: 0, isDamaging: false, tileIndex: TileIndex.BONE_1};
	static GATE = {id: 10, name: 'gate', isGate: true, isDino: false, isCollectible: false, reward: 0, isDamaging: false, tileIndex: TileIndex.GATE_1};
	static UNKNOWN = {id: 11, name: 'unknown', isGate: false, isDino: false, isCollectible: false, reward: 0, isDamaging: false, tileIndex: null};

	static all = [
		this.SILVER,
		this.GOLD,
		this.EMERALD,
		this.RUBY,
		this.MOONSTONE,
		this.DIRT,
		this.AIR,
		this.MAGMA,
		this.ROCK,
		this.BONE,
		this.GATE,
		this.UNKNOWN,
	];

	static collectibles = (() => this.all.filter((x) => x.isCollectible))();
	static questables = (() => this.all.filter((x) => x.isGate || x.isDino))();
	static statables = (() => this.collectibles.concat(this.questables))();
}
