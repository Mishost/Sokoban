#ifndef __GAME__HEADER__INCLUDED
#define __GAME__HEADER__INCLUDED

#include <string>
#include <fstream>
#include <queue> //for std::queue
#include <set> //for std::set
#include <map> //for std::map
#include <utility> //for std::pair
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
	Game();
	~Game() = default;

	void runGame();

private:
	void handleInput(bool& loop, bool& shouldFlip);
	void update(bool& loop);

	void clearLevelData();
	void validateMap();
	bool levelIsValid() const;
	bool loadLevel(const std::string level);
	void loadRow(const std::string line, const unsigned int row);
	bool loadNextLevel();

	void movePlayer(const Direction direction);
	bool move(const unsigned int row, const unsigned int col,
		const Direction direction, const State state);
	bool moveObject(const unsigned int row, const unsigned int col, int rowMovement,
		int colMovement, const State state);
	void findPath(const int x, const int y);
	bool BFS(std::queue<vertex>& wave, std::map<vertex, vertex>& parent, 
		std::set<vertex>& visited, vertex& current, vertex& target);
	void animatePlayerMoving(const std::map<vertex, vertex>& parent,
		const vertex& current, const vertex& target);

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
	bool playerIsMoving;

	unsigned int currentLevel;
	unsigned int levelNum;
};
#endif // !__GAME__HEADER__INCLUDED
