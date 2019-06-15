#ifndef __BLOCK__HEADER__INCLUDED
#define __BLOCK__HEADER__INCLUDED

#include <SDL.h>

const unsigned int width = 50;
const unsigned int height = 50;

enum State
{
	INVALID = -1,
	WALL,
	PLAYER,
	BOX,
	FLOOR
};

struct Block
{
	SDL_Rect position;
	State state;
	bool onMark;
	Block(unsigned int row, unsigned int col, State state, bool onMark);
};

#endif // !__BLOCK__HEADER__INCLUDED