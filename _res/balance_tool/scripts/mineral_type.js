class MineralType {
	static SILVER = {id: 0, name: 'silver', isCollectible: true, reward: 5, isDamaging: false, tileIndex: TileIndex.SILVER_1};
	static GOLD = {id: 1, name: 'gold', isCollectible: true, reward: 10, isDamaging: false, tileIndex: TileIndex.GOLD_1};
	static EMERALD = {id: 2, name: 'emerald', isCollectible: true, reward: 15, isDamaging: false, tileIndex: TileIndex.EMERALD_1};
	static RUBY = {id: 3, name: 'ruby', isCollectible: true, reward: 20, isDamaging: false, tileIndex: TileIndex.RUBY_1};
	static MOONSTONE = {id: 4, name: 'moonstone', isCollectible: true, reward: 25, isDamaging: false, tileIndex: TileIndex.MOONSTONE_1};
	static DIRT = {id: 5, name: 'dirt', isCollectible: false, reward: 0, isDamaging: false, tileIndex: TileIndex.DIRT_1};
	static AIR = {id: 6, name: 'air', isCollectible: false, reward: 0, isDamaging: false, tileIndex: TileIndex.CAVE_BG_1};
	static MAGMA = {id: 7, name: 'magma', isCollectible: false, reward: 0, isDamaging: true, tileIndex: TileIndex.MAGMA_1};
	static ROCK = {id: 8, name: 'rock', isCollectible: false, reward: 0, isDamaging: false, tileIndex: TileIndex.STONE_1};
	static UNKNOWN = {id: 9, name: 'unknown', isCollectible: false, reward: 0, isDamaging: false, tileIndex: null};

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
		this.UNKNOWN,
	];

	static collectibles = (() => this.all.filter((x) => x.isCollectible))();
}
