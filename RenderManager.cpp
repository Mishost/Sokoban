#include <SDL_image.h>
#include "RenderManager.h"
#include "Block.h"

bool RenderManager::InitEverything()
{
	if (!InitSDL())
		return false;

	if (!CreateWindow())
		return false;

	if (!CreateRenderer())
		return false;

	if (!SetupTTF("STIXGeneral.ttf"))
		return false;

	SetupRenderer();

	return true;
}
bool RenderManager::InitSDL()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		std::cout << " Failed to initialize SDL : " << SDL_GetError() << std::endl;
		return false;
	}

	return true;
}
bool RenderManager::CreateWindow()
{
	window = SDL_CreateWindow("Sokoban", posX, posY, sizeX, sizeY, 0);

	if (window == nullptr)
	{
		std::cout << "Failed to create window : " << SDL_GetError();
		return false;
	}

	return true;
}
bool RenderManager::CreateRenderer()
{
	renderer = SDL_CreateRenderer(window, -1, 0);

	if (renderer == nullptr)
	{
		std::cout << "Failed to create renderer : " << SDL_GetError();
		return false;
	}

	return true;
}
void RenderManager::SetupRenderer()
{
	// Set size of renderer to the same as window
	SDL_RenderSetLogicalSize(renderer, sizeX, sizeY);

	// Set color of renderer to black
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

bool RenderManager::SetupTTF(const std::string& fontName)
{
	if (TTF_Init() == -1)
	{
		std::cout << " Failed to initialize TTF : " << SDL_GetError() << std::endl;
		return false;
	}

	// Load our fonts, with a huge size
	font = TTF_OpenFont(fontName.c_str(), 90);

	// Error check
	if (font == nullptr)
	{
		std::cout << " Failed to load font : " << SDL_GetError() << std::endl;
		return false;
	}

	return true;
}

SDL_Texture* RenderManager::SurfaceToTexture(SDL_Surface* surf)
{
	SDL_Texture* text;

	text = SDL_CreateTextureFromSurface(renderer, surf);

	SDL_FreeSurface(surf);

	return text;
}

unsigned int RenderManager::getWindowHeight() const
{
	return sizeY;
}

void RenderManager::Render(std::vector<std::vector<Block>>& map, bool isFirstRender, bool shouldFlip)
{
	if (isFirstRender)
	{
		playerTexture = LoadTexture(playerImage);
		wallTexture = LoadTexture(wallImage);
		boxTexture = LoadTexture(boxImage);
		markTexture = LoadTexture(markImage);
	}

	SDL_RenderClear(renderer);
	for (std::vector<Block>& currRow : map)
	{
		for (Block& currBlock : currRow)
		{
			if (currBlock.state == WALL)
				SDL_RenderCopy(renderer, wallTexture, NULL, &currBlock.position);
			else if (currBlock.state == BOX)
				SDL_RenderCopy(renderer, boxTexture, NULL, &currBlock.position);
			else if (currBlock.state == PLAYER)
			{
				if (shouldFlip)
				{
					SDL_RendererFlip flipPlayer = SDL_FLIP_HORIZONTAL;
					SDL_RenderCopyEx(renderer, playerTexture, NULL,
						&currBlock.position, 0, NULL, flipPlayer);
				}
				else
					SDL_RenderCopy(renderer, playerTexture, NULL, &currBlock.position);
			}
			else if (currBlock.state == FLOOR && currBlock.onMark)
				SDL_RenderCopy(renderer, markTexture, NULL, &currBlock.position);
			else
				SDL_RenderFillRect(renderer, &currBlock.position);
		}
	}
	SDL_RenderPresent(renderer);
}

void RenderManager::RenderMessage(std::string msg)
{
	SDL_RenderClear(renderer);
	
	SDL_Surface* blendedText = TTF_RenderText_Blended(font, msg.c_str(), textColor);
	SDL_Texture* textTexture = SurfaceToTexture(blendedText);

	SDL_Rect textRect = { sizeX / 4, sizeY / 4, 50, 50 };
	SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);

	if (msg == "YOU WON!")
		textRect.x -= 50;
	else 
		textRect.x -= 150;

	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
	SDL_RenderPresent(renderer);
}
unsigned int RenderManager::getWindowWidth() const
{
	return sizeX;
}

SDL_Texture* RenderManager::LoadTexture(const std::string& str)
{
	// Load image as SDL_Surface
	SDL_Surface* surface = IMG_Load(str.c_str());

	// SDL_Surface is just the raw pixels
	// Convert it to a hardware-optimzed texture so we can render it
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	// Don't need the orignal texture, release the memory
	SDL_FreeSurface(surface);

	return texture;
}
