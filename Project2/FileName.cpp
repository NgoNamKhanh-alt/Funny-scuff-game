#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
const int SCREEN_WIDTH = 1260;
const int SCREEN_HEIGHT = 720;
const int TILE_SIZE = 36;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;
const int SHOT_DELAY = 250;
const int ENEMY_SHOT_DELAY = 1500;
const int ENEMY_SPEED_DELAY = 2500;
const int FODDER_DELAY = 1000;
int last_shot_time = 0;
int last_enemy_shot_time = 0;
int last_speed_change = 0;
int fodder_spawn = 0;
std::mt19937 mt{ static_cast<std::mt19937::result_type>(
	std::chrono::steady_clock::now().time_since_epoch().count()
	) };
std::uniform_int_distribution<> speed_enemy{ -2, 2 };
std::uniform_int_distribution<> fodder_plm{ -350, 350 };
std::uniform_int_distribution<> fodder_dis{ -37, 37 };
class Enemy_Bullet {
public:
	int x, y;
	double dirX, dirY;
	int damage = 1;
	double speed = 6.25;
	SDL_Rect rect;
	Enemy_Bullet(int startX, int startY, double dirX, double dirY) {
		x = startX;
		y = startY;
		this->dirX = dirX;
		this->dirY = dirY;
		rect = { x, y, TILE_SIZE / 2, TILE_SIZE / 2 };
	}
	void move() {
		x = x + (speed * dirX);
		y = y + (speed * dirY);
		rect.x = x;
		rect.y = y;
	}
	void render(SDL_Renderer* renderer) {
		SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
		SDL_RenderFillRect(renderer, &rect);
	}
};
class Enemy {
public:
	int x, y;
	int y_speed = 1;
	SDL_Rect rect;
	Enemy(int startX, int startY) {
		x = startX;
		y = startY;
		rect = { x, y, TILE_SIZE * 3, TILE_SIZE * 3 };
	}
	void move(int x_speed, int y_speed) {
		x = x + x_speed;
		y = y + y_speed;
		rect.x = x;
		rect.y = y;
	}
	void render(SDL_Renderer* renderer) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		SDL_RenderFillRect(renderer, &rect);
	}
	};
class Player_Bullet {
public:
	int x, y;
	int damage = 1;
	int speed = -9;
	int dirX, dirY;
	SDL_Rect rect;
	Player_Bullet(int startX, int startY) {
		x = startX;
		y = startY;
		rect = { x, y, TILE_SIZE/3, TILE_SIZE/3 };
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
class Player {
public:
	int x, y;
	int dirX, dirY;
	SDL_Rect rect;
	Player() : x(0), y(0), dirX(0), dirY(0){ rect = { x, y, TILE_SIZE, TILE_SIZE }; }
	Player(int startX, int startY){
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
class Fodders {
public:
	int x, y;
	double dirX, dirY;
	int damage = 1;
	double speed = 6;
	SDL_Rect rect;
	int lastDirectionChange = SDL_GetTicks();
    Fodders(int startX, int startY, int targetX) {
        x = startX;
        y = startY;
        rect = { x, y, TILE_SIZE / 2, TILE_SIZE / 2 };
		UpdateDirection(targetX);
    }
	void UpdateDirection(int targetX) {
		double toX = targetX - x;
		double toY = SCREEN_HEIGHT - y;
		double length = sqrt(toX * toX + toY * toY);
		dirX = (toX / length) * speed;
		dirY = (toY / length) * speed;
	}

    void move(Player &player) {
		Uint32 currentTime = SDL_GetTicks();
		if (currentTime - lastDirectionChange >= 200) {
			UpdateDirection(player.x);
			lastDirectionChange = currentTime;
		}
        x += dirX;
        y += dirY;
		if (player.y < y) {
			y += speed/5;
		}
        rect.x = x;
        rect.y = y;
    }
	void render(SDL_Renderer* renderer) {
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &rect);
	}
};
class Game {
public: 
	Player player;
	SDL_Window* window;
	SDL_Renderer* renderer;
	std::vector<Player_Bullet> bullets;
	Enemy enemy; 
	std::vector<Enemy_Bullet> enemy_bullets;
	std::vector<Fodders> fodders;
	bool running;
	Game() : enemy(SCREEN_WIDTH / 2.2, SCREEN_HEIGHT / 6) {
		running = true;
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::cerr << "SDL could not initialize! SDL Error:" << SDL_GetError() << "\n";
			running = false;
		}
		window = SDL_CreateWindow("Funny Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (!window) {
			std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
			running = false;
		}
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (!renderer) {
			std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << "\n";
			running = false;
		}
		player = Player((((MAP_WIDTH - 1) / 2) * TILE_SIZE), ((MAP_HEIGHT - 2) * TILE_SIZE));
	}
	void update() {
		Uint32 currentTime = SDL_GetTicks();
		if (currentTime - fodder_spawn >= FODDER_DELAY) {
			fodders.push_back(Fodders(player.x + fodder_plm(mt), 0, player.x));
			fodders.push_back(Fodders(player.x + fodder_plm(mt) + fodder_dis(mt), 0, player.x));
			fodders.push_back(Fodders(player.x + fodder_plm(mt) - fodder_dis(mt), 0, player.x));
			fodder_spawn = currentTime;
		}
		for (int i = 0; i < fodders.size(); ++i) {
			fodders[i].move(player);
			if (fodders[i].y > 720) {
				fodders.erase(fodders.begin() + i);
			}
		}
		for (int i = 0; i < bullets.size(); ++i) {
			bullets[i].move();
			if (bullets[i].y < 0) {
				bullets.erase(bullets.begin() + i);
			}
		}
		int enemy_center = enemy.x + enemy.rect.w / 2;
		int player_center = player.x + player.rect.w / 2;
		if (enemy.y < 20) {
			enemy.y_speed = 2;
			last_speed_change = currentTime;
		}
		else if (enemy.y > 315) {
			enemy.y_speed = -2;
			last_speed_change = currentTime;
		}
		else if (currentTime - last_speed_change >= ENEMY_SPEED_DELAY) {
			enemy.y_speed = speed_enemy(mt);
			if (enemy.y_speed == 0) {
				enemy.y_speed = 1;
			}
			last_speed_change = currentTime;
		}
		if (enemy_center > player_center) {
			enemy.move(-3,enemy.y_speed);
		}
		else if (enemy_center < player_center) {
			enemy.move(3,enemy.y_speed);
		}
		Uint32 enemy_currentTime = SDL_GetTicks();
		if (enemy_currentTime - last_enemy_shot_time >= ENEMY_SHOT_DELAY) {
			enemy_bullets.push_back(Enemy_Bullet(enemy_center, enemy.y + TILE_SIZE * 3, 0, 1));
			enemy_bullets.push_back(Enemy_Bullet(enemy_center + TILE_SIZE, enemy.y + TILE_SIZE * 3, 0.75, 0.85));
			enemy_bullets.push_back(Enemy_Bullet(enemy_center - TILE_SIZE, enemy.y + TILE_SIZE * 3, -0.75, 0.85));
			enemy_bullets.push_back(Enemy_Bullet(enemy_center - TILE_SIZE * 2, enemy.y + TILE_SIZE , -1, 0));
			enemy_bullets.push_back(Enemy_Bullet(enemy_center + TILE_SIZE * 2, enemy.y + TILE_SIZE, 1, 0));
			enemy_bullets.push_back(Enemy_Bullet(enemy_center - TILE_SIZE, enemy.y - TILE_SIZE, -0.75, -0.85));
			enemy_bullets.push_back(Enemy_Bullet(enemy_center + TILE_SIZE, enemy.y - TILE_SIZE, 0.75, -0.85));
			enemy_bullets.push_back(Enemy_Bullet(enemy_center, enemy.y - TILE_SIZE, 0, -1));
			last_enemy_shot_time = enemy_currentTime;
		}
		for (int i = 0; i < enemy_bullets.size(); ++i) {
			enemy_bullets[i].move();
			if (enemy_bullets[i].y > 720 || enemy_bullets[i].y < 0) {
				enemy_bullets.erase(enemy_bullets.begin() + i);
			}
		}

	}
	void render() {
	
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		for (int i = 0; i < MAP_HEIGHT; ++i) {
			for (int j = 0; j < MAP_WIDTH; ++j) {
				SDL_Rect rect = {j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE};
				SDL_RenderFillRect(renderer, &rect);
			}
		}
		for (int i = 0; i < fodders.size(); ++i) {
			fodders[i].render(renderer);
		}
		player.render(renderer);
		for (int i = 0; i < bullets.size(); ++i) {
			bullets[i].render(renderer);
		}
		for (int i = 0; i < enemy_bullets.size(); ++i) {
			enemy_bullets[i].render(renderer);
		}
		enemy.render(renderer);
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