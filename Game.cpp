#include "Game.h"

Game::Game(const std::string level)
	:blockWidth(width)
	,blockHeight(height)
	,marksCount(0)
	,filledMarks(0)
	,boxCount(0)
	,boxesOnPlace(0)
{
	if (loadLevel(level))
	{
		renderManager.Render(map, 1, 0);
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
			SDL_Delay(5000);
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
}

void Game::loadRow(const std::string line, unsigned int row)
{
	State currState;
	bool onGoal = false;
	unsigned int len = line.size();
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
			break;
		case '+':
			currState = PLAYER;
			onGoal = true;
			playerPos.x = blockWidth * i;
			playerPos.y = blockHeight * row;
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
			++filledMarks;
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
			if (map[row][col].onGoal)
				--boxesOnPlace; //--filledMarks;
			if (map[row - 1][col].onGoal)
				++boxesOnPlace; //++filledMarks;
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
			if (map[row][col].onGoal)
				--boxesOnPlace; // --filledMarks;
			if (map[row + 1][col].onGoal)
				++boxesOnPlace; // ++filledMarks;
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
			if (map[row][col].onGoal)
				--boxesOnPlace; // --filledMarks;
			if (map[row][col - 1].onGoal)
				++boxesOnPlace; //++filledMarks;
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
			if (map[row][col].onGoal)
				--boxesOnPlace; //--filledMarks;
			if (map[row][col + 1].onGoal)
				++boxesOnPlace; //++filledMarks;
		}
	}
	else
		return false;
	map[row][col].state = FLOOR;
	return true;
}
