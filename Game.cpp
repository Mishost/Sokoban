#include "Game.h"

Game::Game()
	:blockWidth(width)
	,blockHeight(height)
	,marksCount(0)
	,boxCount(0)
	,boxesOnPlace(0)
	,playerCount(0)
	,playerIsMoving(false)
	,currentLevel(0)
	,levelNum(3)
{
	if (loadNextLevel())
		renderManager.Render(map, true, false);
	else
	{
		renderManager.RenderMessage("No valid levels.");
		SDL_Delay(2000);
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
		handleInput(loop, shouldFlip);
		renderManager.Render(map, isFirst, shouldFlip);
		isFirst = false;
		SDL_Delay(30); //changing the number of fps
		update(loop);
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

void Game::handleInput(bool& loop, bool& shouldFlip)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (playerIsMoving)
			continue;
		if (event.type == SDL_QUIT)
			loop = false;
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (event.button.x > blockCols * blockWidth ||
				event.button.y > blockRows * blockHeight)
				continue;
			else
				findPath(event.button.x, event.button.y);
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
}

void Game::update(bool& loop)
{
	if (boxCount == boxesOnPlace)
	{
		std::string strLevel = "Level ";
		strLevel += std::to_string(currentLevel);
		strLevel.append(" completed");
		renderManager.RenderMessage(strLevel);
		SDL_Delay(2000);

		if (!loadNextLevel())
		{
			renderManager.RenderMessage("YOU WON!");
			SDL_Delay(2000);
			loop = false;
		}
	}
}

void Game::clearLevelData()
{
	if (!map.empty())
	{
		for (int i = blockRows - 1; i >= 0; --i)
		{
			for (unsigned int j = map[i].size(); j > 0; --j)
				map[i].pop_back();
			map.pop_back();
		}
	}
	
	marksCount = 0;
	boxCount = 0;
	boxesOnPlace = 0;
	playerCount = 0;
	playerIsMoving = false;
}

void Game::validateMap()
{
	for (unsigned int i = 0; i < blockRows; ++i)
	{
		for (unsigned int j = map[i].size(); j < blockCols; ++j)
			map[i].push_back(Block(i, j, FLOOR, false));
	}
}

void Game::loadRow(const std::string line, const unsigned int row)
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
			currState = WALL;
			break;
		}
		map[row].push_back(Block(row, i, currState, onGoal));
		onGoal = false;
	}
}

bool Game::loadNextLevel()
{
	clearLevelData();
	std::string strLevel = "level";
	strLevel += std::to_string(++currentLevel);
	strLevel += ".txt";
	while (currentLevel <= levelNum && !loadLevel(strLevel))
	{
		std::string message = "Level ";
		message += std::to_string(currentLevel);
		message += " is invalid.";
		renderManager.RenderMessage(message);
		SDL_Delay(2000);

		clearLevelData();
		
		strLevel = "level";
		strLevel += std::to_string(++currentLevel);
		strLevel += ".txt";
	}
	if (currentLevel <= levelNum)
	{
		validateMap();
		return true;
	}
	return false;
}

void Game::movePlayer(Direction direction)
{
	unsigned int row = playerPos.y / blockWidth;
	unsigned int col = playerPos.x / blockHeight;

	move(row, col, direction, PLAYER);
}

bool Game::move(const unsigned int row, const unsigned int col,
	const Direction direction, const State state)
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

bool Game::levelIsValid() const
{
	if (boxCount > marksCount || !boxCount || playerCount != 1)
		return false;
	return true;
}

void Game::findPath(const int x, const int y)
{
	vertex target = std::make_pair(y / blockHeight, x / blockWidth);
	if (map[target.first][target.second].state != FLOOR) //we cannot step on walls or boxes
		return;
	std::set<vertex> visited;
	std::map<vertex, vertex> parent;

	std::queue<vertex> wave;
	vertex player = std::make_pair(playerPos.y / blockHeight, playerPos.x / blockWidth);
	wave.push(player);
	visited.insert(player);

	while (!wave.empty())
	{
		if (BFS(wave, parent, visited, wave.front(), target))
		{
			animatePlayerMoving(parent, player, target);
			return;
		}
		wave.pop();
	}
}

bool Game::BFS(std::queue<vertex>& wave, std::map<vertex, vertex>& parent,
	std::set<vertex>& visited, vertex& current, vertex& target)
{
	if (current.first > 0 && map[current.first - 1][current.second].state == FLOOR) //up
	{
		vertex neighbour = std::make_pair(current.first - 1, current.second);
		if (visited.find(neighbour) == visited.end())
		{
			wave.push(neighbour);
			parent.insert_or_assign(neighbour, current);
			visited.insert(neighbour);
			if (neighbour == target)
				return true;
		}
	}
	if (current.first < blockRows && map[current.first + 1][current.second].state == FLOOR) //down
	{
		vertex neighbour = std::make_pair(current.first + 1, current.second);
		if (visited.find(neighbour) == visited.end())
		{
			wave.push(neighbour);
			parent.insert_or_assign(neighbour, current);
			visited.insert(neighbour);
			if (neighbour == target)
				return true;
		}
	}
	if (current.first > 0 && map[current.first][current.second - 1].state == FLOOR) //left
	{
		vertex neighbour = std::make_pair(current.first, current.second - 1);
		if (visited.find(neighbour) == visited.end())
		{
			wave.push(neighbour);
			parent.insert_or_assign(neighbour, current);
			visited.insert(neighbour);
			if (neighbour == target)
				return true;
		}
	}
	if (current.second < blockCols && map[current.first ][current.second + 1].state == FLOOR) //right
	{
		vertex neighbour = std::make_pair(current.first, current.second + 1);
		if (visited.find(neighbour) == visited.end())
		{
			wave.push(neighbour);
			parent.insert_or_assign(neighbour, current);
			visited.insert(neighbour);
			if (neighbour == target)
				return true;
		}
	}
	return false;
}

void Game::animatePlayerMoving(const std::map<vertex, vertex>& parent,
	const vertex& first, const vertex& target)
{
	playerIsMoving = true;
	std::vector<vertex> path;
	vertex current = target;
	vertex temp = first;
	while (parent.find(current) != parent.end() && (first != parent.find(current)->second))
	{
		path.push_back(current);
		current = parent.find(current)->second;
	}
	path.push_back(current);
	while (!path.empty())
	{
		current = path.back();
		map[playerPos.y / blockHeight][playerPos.x / blockWidth].state = FLOOR;

		if (temp.first > current.first)
			playerPos.y -= blockHeight;
		else if (temp.first < current.first)
			playerPos.y += blockHeight;
		else if (temp.second > current.second) 
			playerPos.x -= blockWidth;
		else if (temp.second < current.second)
			playerPos.x += blockWidth;

		temp = current;
		path.pop_back();

		map[playerPos.y / blockHeight][playerPos.x / blockWidth].state = PLAYER;
		renderManager.Render(map, false, false);
		SDL_Delay(20);
	}
	playerIsMoving = false;
}
