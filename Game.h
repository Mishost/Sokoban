#ifndef __GAME__HEADER__INCLUDED
#define __GAME__HEADER__INCLUDED

#include <string>
#include <fstream>
#include <set>
#include <map>
#include <queue>
#include "RenderManager.h"
#include "Block.h"

using vertex = std::pair<int, int>;

enum Direction
{
	INVALID_DIR = -1,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Game
{
public:
	Game(const std::string level);
	~Game() = default;

	void runGame();
	bool loadLevel(const std::string level);

private:
	void validateMap();
	void loadRow(const std::string line, unsigned int row);
	void movePlayer(Direction direction);
	bool move(unsigned int row, unsigned int col, Direction direction, State state);
	bool levelIsValid();
	bool findPath(int x, int y);
	bool BFS(std::queue<vertex>& wave, std::map<vertex, vertex>& parent, 
		std::set<vertex>& visited, vertex& current, vertex& target);

private:
	std::vector<std::vector<Block>> map;
	SDL_Rect playerPos;

	unsigned int blockCols; //the number of columns in the map
	unsigned int blockRows; //the number of rows in the map
	int blockWidth; //the width of each block
	int blockHeight; //the height of each block
	RenderManager renderManager; //responsible for the rendering

	unsigned int playerCount; //to check if the game has exactly one player
	unsigned int boxCount;
	unsigned int boxesOnPlace;
	unsigned int marksCount;
};
#endif // !__GAME__HEADER__INCLUDED
