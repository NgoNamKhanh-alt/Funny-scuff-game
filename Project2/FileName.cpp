#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int TILE_SIZE = 36;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;
const int SHOT_DELAY = 250;
int last_shot_time = 0;
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
class Player_Bullet {
public:
	int x, y;
	int speed = -9;
	int dirX, dirY;
	SDL_Rect rect;
	Player_Bullet(int startX, int startY) {
		x = startX;
		y = startY;
		rect = { x, y, TILE_SIZE/3, TILE_SIZE/3};
		dirX = 0;
		dirY = -1;
	}
	void move() {
		int new_Y = y + speed;
			y = new_Y;
			rect.x = x;
			rect.y = y;
		
	}
	void render(SDL_Renderer* renderer) {
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderFillRect(renderer, &rect);
	}
};
class Player_Tank {
public:
	int x, y;
	int dirX, dirY;
	SDL_Rect rect;
	Player_Tank() : x(0), y(0), dirX(0), dirY(0){ rect = { x, y, TILE_SIZE, TILE_SIZE }; }
	Player_Tank(int startX, int startY){
		x = startX;
		y = startY;
		rect = { x, y, TILE_SIZE, TILE_SIZE };
		dirX = 0;
		dirY = -1;
	}

	void move(int dx, int dy) {
		int newX = x + dx;
		int newY = y + dy;
		this->dirX = dx;
		this->dirY = dy;
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
	Player_Tank player;
	std::vector<Wall> walls;
	SDL_Window* window;
	SDL_Renderer* renderer;
	std::vector<Player_Bullet> bullets;
	bool running;
	Game() {
		running = true;
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::cerr << "SDL could not initialize! SDL Error:" << SDL_GetError() << "\n";
			running = false;
		}
		window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (!window) {
			std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
			running = false;
		}
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (!renderer) {
			std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << "\n";
			running = false;
		}
		player = Player_Tank((((MAP_WIDTH - 1) / 2) * TILE_SIZE), ((MAP_HEIGHT - 2) * TILE_SIZE));
	}
	void update() {
		for (int i = 0; i < bullets.size(); ++i) {
			bullets[i].move();
			if (bullets[i].y < 0) {
				bullets.erase(bullets.begin() + i);
			}
		}
	}
	void render() {
		SDL_SetRenderDrawColor(renderer, 128,128,128,255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		for (int i = 0; i < MAP_HEIGHT; ++i) {
			for (int j = 0; j < MAP_WIDTH; ++j) {
				SDL_Rect rect = { j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE };
				SDL_RenderFillRect(renderer, &rect);
			}
		}
		player.render(renderer);
		for (int i = 0; i < bullets.size(); ++i) {
			bullets[i].render(renderer);
		}
		SDL_RenderPresent(renderer);
	}
	void run() {
		while (running) {
			handleEvents();
			render();
			update();
			SDL_Delay(16);
		}
	}
	void handleEvents() {
		SDL_Event event;
		const Uint8* keystates = SDL_GetKeyboardState(NULL);

		if (keystates[SDL_SCANCODE_W]) {
			player.move(0, -5);  // Move up
		}
		if (keystates[SDL_SCANCODE_S]) {
			player.move(0, 5);   // Move down
		}
		if (keystates[SDL_SCANCODE_A]) {
			player.move(-5, 0);  // Move left
		}
		if (keystates[SDL_SCANCODE_D]) {
			player.move(5, 0);   // Move right
		}
		Uint32 currentTime = SDL_GetTicks();
		if (keystates[SDL_SCANCODE_SPACE]) {
			if (currentTime - last_shot_time >= SHOT_DELAY) {
				bullets.push_back(Player_Bullet(player.x + TILE_SIZE / 3, player.y + TILE_SIZE / 3 - TILE_SIZE + 4));
				last_shot_time = currentTime;
			}
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
		SDL_Delay(16);
	}
	return 0;
}