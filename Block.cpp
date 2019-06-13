#include "Block.h"

Block::Block(unsigned int row, unsigned int col, State state, bool onGoal)
	:onGoal(onGoal)
	,state(state)
{
	position.w = width;
	position.h = height;
	position.x = col * height;
	position.y = row * width;
}
