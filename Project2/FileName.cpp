#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;
class Wall {
public:
	int x, y;
	SDL_Rect rect;
	bool active;
	Wall(int startX, int startY) {
		x = startX;
		y = startY;
		rect = { x, y, TILE_SIZE, TILE_SIZE };
		active = true;
	}
	void render(SDL_Renderer* renderer) {
		if (active) {
			SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255);
			SDL_RenderFillRect(renderer, &rect);

		}
	}
};
class PlayerTank {
public:
	int x, y;
	int dirX, dirY;
	SDL_Rect rect;
	PlayerTank() : x(0), y(0), dirX(0), dirY(0){}
	PlayerTank(int startX, int startY){
		x = startX;
		y = startY;
		dirX = 0;
		dirY = -1;
	}
	void move(int dx, int dy, const std::vector<Wall>& walls) {
		int newX = x + dx;
		int newY = y + dy;
		this->dirX = dx;
		this->dirY = dy;
		SDL_Rect newRect = { newX, newY, TILE_SIZE, TILE_SIZE };
		for (int i = 0; i < walls.size(); ++i) {
			if (SDL_HasIntersection(&newRect, &walls[i].rect)) {
				return;
			}
		}
		if (newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE * 2 && newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE * 2) {
			x = newX;
			y = newY;
			rect.x = x;
			rect.y = y;
		}
	}
	void render(SDL_Renderer* renderer) {
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
		SDL_RenderFillRect(renderer, &rect);
	}

};
class Game {
public: 
	PlayerTank player;
	std::vector<Wall> walls;
	SDL_Window* window;
	SDL_Renderer* renderer;
	bool running;
	Game() {
		running = true;
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::cerr << "SDL could not initialize! SDL Error:" << SDL_GetError() << "\n";
			running = false;
			window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		}
		if (!window) {
			std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
			running = false;
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		}
		if (!renderer) {
			std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << "\n";
			running = false;
		}
		generateWalls(); 
		player = PlayerTank((((MAP_WIDTH - 1) / 2) * TILE_SIZE), ((MAP_HEIGHT - 2) * TILE_SIZE));
	}
	void generateWalls() {
		for (int i{ 3 }; i < MAP_HEIGHT - 3; i += 2) {
			for (int j{ 3 }; j < MAP_WIDTH - 3; j += 2) {
				Wall w = Wall(j * TILE_SIZE, i * TILE_SIZE);
				walls.push_back(w);
			}
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
	void handleEvents() {
		SDL_Event event;
		const Uint8* keystates = SDL_GetKeyboardState(NULL);

		if (keystates[SDL_SCANCODE_W]) {
			player.move(0, -5, walls);  // Move up
		}
		if (keystates[SDL_SCANCODE_S]) {
			player.move(0, 5, walls);   // Move down
		}
		if (keystates[SDL_SCANCODE_A]) {
			player.move(-5, 0, walls);  // Move left
		}
		if (keystates[SDL_SCANCODE_D]) {
			player.move(5, 0, walls);   // Move right
		}
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
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
		game.handleEvents();
		SDL_Delay(16);
	}
	return 0;
}