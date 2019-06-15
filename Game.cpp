#include "Game.h"

Game::Game(const std::string level)
	:blockWidth(width)
	,blockHeight(height)
	,marksCount(0)
	,boxCount(0)
	,boxesOnPlace(0)
	,playerCount(0)
{
	if (loadLevel(level))
	{
		renderManager.Render(map, 1, 0);
		validateMap();
	}
	else
	{
		renderManager.RenderInvalid();
		SDL_Delay(3000);
		std::exit(-1);
	}
}

void Game::runGame()
{
	bool loop = true;
	bool isFirst = true;
	bool shouldFlip = false;

	while (loop)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				loop = false;
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				std::cout << event.button.x << ' ' << event.button.y << '\n';
				if (event.button.x > blockCols * blockWidth ||
					event.button.y > blockRows * blockHeight)
					continue;
				else if (findPath(event.button.x, event.button.y))
				{
					map[playerPos.y / blockHeight][playerPos.x / blockWidth].state = FLOOR;
					playerPos.x = event.button.x;
					playerPos.y = event.button.y;
					playerPos.x /= 50;
					playerPos.x *= 50;
					playerPos.y /= 50;
					playerPos.y *= 50;
					map[playerPos.y / blockHeight][playerPos.x / blockWidth].state = PLAYER;
				}
			}
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_RIGHT:
					movePlayer(RIGHT);
					shouldFlip = true;
					break;
				case SDLK_LEFT:
					movePlayer(LEFT);
					shouldFlip = false;
					break;
				case SDLK_UP:
					movePlayer(UP);
					break;
				case SDLK_DOWN:
					movePlayer(DOWN);
					break;
				default:
					break;
				}
			}
		}
		renderManager.Render(map, isFirst, shouldFlip);
		isFirst = false;
		SDL_Delay(30); //changing the number of fps
		if (boxCount == boxesOnPlace)
		{
			renderManager.gameWon();
			SDL_Delay(3000);
			loop = false;
		}
	}
}

bool Game::loadLevel(const std::string level)
{
	std::ifstream file(level);
	if (!file.is_open())
	{
		std::cerr << "Error when loading level. The program will quit.\n";
		return false;
	}
	map.push_back(std::vector<Block>());
	playerPos.h = blockHeight;
	playerPos.w = blockWidth;

	std::string current;
	std::getline(file, current, '\n');

	blockCols = current.size();
	blockRows = 0;
	
	loadRow(current, blockRows++);
	while (!file.eof())
	{
		map.push_back(std::vector<Block>());
		std::getline(file, current, '\n');
		loadRow(current, blockRows++);
	}
	file.close();
	return levelIsValid();
}

void Game::validateMap()
{
	for (unsigned int i = 0; i < blockRows; ++i)
	{
		for (unsigned int j = map[i].size(); j < blockCols; ++j)
			map[i].push_back(Block(i, j, FLOOR, false));
	}
}

void Game::loadRow(const std::string line, unsigned int row)
{
	State currState;
	bool onGoal = false;
	unsigned int len = line.size();
	if (len > blockCols)
		blockCols = len;
	for (unsigned int i = 0; i < len; ++i)
	{
		switch (line[i])
		{
		case '#':
			currState = WALL;
			break;
		case '@':
			currState = PLAYER;
			playerPos.x = blockWidth * i;
			playerPos.y = blockHeight * row;
			++playerCount;
			break;
		case '+':
			currState = PLAYER;
			onGoal = true;
			playerPos.x = blockWidth * i;
			playerPos.y = blockHeight * row;
			++playerCount;
			++marksCount;
			break;
		case '$':
			currState = BOX;
			++boxCount;
			break;
		case '*':
			currState = BOX;
			onGoal = true;
			++boxCount;
			++boxesOnPlace;
			++marksCount;
			break;
		case '.':
			currState = FLOOR;
			onGoal = true;
			++marksCount;
			break;
		case ' ':
			currState = FLOOR;
			break;
		default:
			currState = INVALID;
			break;
		}
		map[row].push_back(Block(row, i, currState, onGoal));
		onGoal = false;
	}
}

void Game::movePlayer(Direction direction)
{
	unsigned int row = playerPos.y / blockWidth;
	unsigned int col = playerPos.x / blockHeight;

	move(row, col, direction, PLAYER);
}

bool Game::move(unsigned int row, unsigned int col, Direction direction, State state)
{
	if (direction == UP && map[row - 1][col].state != WALL && row)
	{
		if (state == BOX && map[row - 1][col].state == BOX) //we can't move more than 1 box at a time
			return false;
		if (map[row - 1][col].state == BOX) //the player is below a box
		{
			if (row > 1 && move(row - 1, col, UP, BOX))
				map[row - 2][col].state = BOX;
			else
				return false;
		}
		if (state == PLAYER)
		{
			map[row - 1][col].state = PLAYER;
			playerPos.y -= blockHeight;
		}
		else if (state == BOX) //if we get to this line, the box can be moved
		{
			if (map[row][col].onMark)
				--boxesOnPlace;
			if (map[row - 1][col].onMark)
				++boxesOnPlace;
		}
	}
	else if (direction == DOWN && map[row + 1][col].state != WALL &&
		row * blockHeight + blockHeight < renderManager.getWindowHeight())
	{
		if (state == BOX && map[row + 1][col].state == BOX) //we can't move more than 1 box at a time
			return false;
		if (map[row + 1][col].state == BOX) //the player is above a box
		{
			if (move(row + 1, col, DOWN, BOX))
				map[row + 2][col].state = BOX;
			else
				return false;
		}
		if (state == PLAYER)
		{
			map[row + 1][col].state = PLAYER;
			playerPos.y += blockHeight;
		}
		else if (state == BOX) //if we get to this line, the box can be moved
		{
			if (map[row][col].onMark)
				--boxesOnPlace;
			if (map[row + 1][col].onMark)
				++boxesOnPlace;
		}
	}
	else if (direction == LEFT && map[row][col - 1].state != WALL && col)
	{
		if (state == BOX && map[row][col - 1].state == BOX) //we can't move more than 1 box at a time
			return false;
		if (map[row][col - 1].state == BOX) //the player is above a box
		{
			if (move(row, col - 1, LEFT, BOX))
				map[row][col - 2].state = BOX;
			else
				return false;
		}
		if (state == PLAYER)
		{
			map[row][col - 1].state = PLAYER;
			playerPos.x -= blockWidth;
		}
		else if (state == BOX) //if we get to this line, the box can be moved
		{
			if (map[row][col].onMark)
				--boxesOnPlace;
			if (map[row][col - 1].onMark)
				++boxesOnPlace;
		}
	}
	else if (direction == RIGHT && map[row][col + 1].state != WALL &&
		col * blockWidth + blockWidth < renderManager.getWindowWidth())
	{
		if (state == BOX && map[row][col + 1].state == BOX) //we can't move more than 1 box at a time
			return false;
		if (map[row][col + 1].state == BOX) //the player is above a box
		{
			if (move(row, col + 1, RIGHT, BOX))
				map[row][col + 2].state = BOX;
			else
				return false;
		}
		if (state == PLAYER)
		{
			map[row][col + 1].state = PLAYER;
			playerPos.x += blockWidth;
		}
		else if (state == BOX) //if we get to this line, the box can be moved
		{
			if (map[row][col].onMark)
				--boxesOnPlace;
			if (map[row][col + 1].onMark)
				++boxesOnPlace;
		}
	}
	else
		return false;
	map[row][col].state = FLOOR;
	return true;
}

bool Game::levelIsValid()
{
	if (boxCount > marksCount || !boxCount || playerCount != 1)
		return false;
	return true;
}

bool Game::findPath(int x, int y)
{
	vertex target = std::make_pair(y / blockHeight, x / blockWidth);
	if (map[target.first][target.second].state != FLOOR) //we cannot step on walls or boxes
		return false;
	std::set<vertex> visited;
	std::map<vertex, vertex> parent;

	std::queue<vertex> wave;
	vertex player = std::make_pair(playerPos.y / blockHeight, playerPos.x / blockWidth);
	wave.push(player);
	visited.insert(player);

	while (!wave.empty())
	{
		if (BFS(wave, parent, visited, wave.front(), target))
			return true;
		wave.pop();
	}
	return false;
}

bool Game::BFS(std::queue<vertex>& wave, std::map<vertex, vertex>& parent,
	std::set<vertex>& visited, vertex& current, vertex& target)
{
	if (current.first > 0 && map[current.first - 1][current.second].state == FLOOR) //up
	{
		vertex neighbour = std::make_pair(current.first - 1, current.second);
		if (visited.find(neighbour) == visited.end())
		{
			if (neighbour == target)
				return true;
			wave.push(neighbour);
			parent.insert_or_assign(neighbour, current);
			visited.insert(neighbour);
		}
	}
	if (current.first < blockRows && map[current.first + 1][current.second].state == FLOOR) //down
	{
		vertex neighbour = std::make_pair(current.first + 1, current.second);
		if (visited.find(neighbour) == visited.end())
		{
			if (neighbour == target)
				return true;
			wave.push(neighbour);
			parent.insert_or_assign(neighbour, current);
			visited.insert(neighbour);
		}
	}
	if (current.first > 0 && map[current.first][current.second - 1].state == FLOOR) //left
	{
		vertex neighbour = std::make_pair(current.first, current.second - 1);
		if (visited.find(neighbour) == visited.end())
		{
			if (neighbour == target)
				return true;
			wave.push(neighbour);
			parent.insert_or_assign(neighbour, current);
			visited.insert(neighbour);
		}
	}
	if (current.second < blockCols && map[current.first ][current.second + 1].state == FLOOR) //right
	{
		vertex neighbour = std::make_pair(current.first, current.second + 1);
		if (visited.find(neighbour) == visited.end())
		{
			if (neighbour == target)
				return true;
			wave.push(neighbour);
			parent.insert_or_assign(neighbour, current);
			visited.insert(neighbour);
		}
	}
	return false;
}
