#include "Game.h"

int main(int argc, char* args[])
{
	//std::ofstream file("Assets\\test.txt", std::ios::trunc);
	//file.write("sdfsd", 5);
	//file.close();
	//std::cin.get();
	Game game("level5.txt");
	game.runGame();
	std::cin.get();
	return 0;
}
