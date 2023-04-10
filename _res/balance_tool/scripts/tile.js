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
