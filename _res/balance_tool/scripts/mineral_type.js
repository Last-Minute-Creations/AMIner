class MineralType {
	static SILVER = {id: 0, name: 'silver', isCollectible: true, reward: 5};
	static GOLD = {id: 1, name: 'gold', isCollectible: true, reward: 10};
	static EMERALD = {id: 2, name: 'emerald', isCollectible: true, reward: 15};
	static RUBY = {id: 3, name: 'ruby', isCollectible: true, reward: 20};
	static MOONSTONE = {id: 4, name: 'moonstone', isCollectible: true, reward: 25};
	static DIRT = {id: 5, name: 'dirt', isCollectible: false, reward: 0};
	static AIR = {id: 6, name: 'air', isCollectible: false, reward: 0};
	static MAGMA = {id: 7, name: 'magma', isCollectible: false, reward: 0};
	static ROCK = {id: 8, name: 'rock', isCollectible: false, reward: 0};
	static UNKNOWN = {id: 9, name: 'unknown', isCollectible: false, reward: 0};

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
