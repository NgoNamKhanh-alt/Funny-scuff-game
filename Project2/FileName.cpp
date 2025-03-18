#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;
class Game {
public: 
	SDL_Window* window;
	SDL_Renderer* renderer;
	bool running;
	Game() {
		running = true;
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::cerr << "SDL could not initialize! SDL Error:" << SDL_GetError() << "\n";
				running=false;
			window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		}
		if (!window) {
			std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
				running=false;
				renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			}
		if (!renderer) {
			std::cerr << "Renderer could not be created! SDL Error: "<< SDL_GetError() << "\n";
					running = false;
			}
	}
	void render() {
		SDL_SetRenderDrawColor(renderer, 128,128,128,255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		for (int i = 1; i <= MAP_HEIGHT; ++i) {
			for (int j = 1; j <= MAP_WIDTH; ++j) {
				SDL_Rect rect = { i * TILE_SIZE, j * TILE_SIZE, TILE_SIZE, TILE_SIZE };
				SDL_RenderFillRect(renderer, &rect);
			}
		}
		SDL_RenderPresent(renderer);
}
	void run() {
		while (running) {
			render();
			SDL_Delay(16);
		}
	}
	~Game() {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
};
int main(int argc, char** argv) {
	Game game;
	if (game.running) {
		game.run();
	}
	return 0;
}