	#include <SDL.h>
	#include <SDL_image.h>
	#include <SDL_ttf.h>
	#include <iostream>
	#include <vector>
	#include <random>
	#include <chrono>
	#include <SDL_mixer.h>
	const int SCREEN_WIDTH = 1260;
	const int SCREEN_HEIGHT = 720;
	const int TILE_SIZE = 36;
	const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
	const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;
	const int SHOT_DELAY = 100;
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
			rect = { x, y, TILE_SIZE * 6 / 7 , TILE_SIZE * 6 / 7 };
		}
		void move() {
			x = x + (speed * dirX);
			y = y + (speed * dirY);
			rect.x = x;
			rect.y = y;
		}
		void render(SDL_Renderer* renderer,SDL_Texture* texture) {
			SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
			SDL_RenderCopy(renderer, texture, NULL, &rect);
		}
	};
	class Enemy {
	public:
		bool alive = true;
		int health = 40;
		int x, y;
		int y_speed = 1;
		SDL_Rect rect;
		Enemy(int startX, int startY) {
			x = startX;
			y = startY;
			rect = { x, y, TILE_SIZE * 4, TILE_SIZE * 4 };
		}
		void move(int x_speed, int y_speed) {
			x = x + x_speed;
			y = y + y_speed;
			rect.x = x;
			rect.y = y;
		}
		void render(SDL_Renderer* renderer, SDL_Texture* texture) {
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderCopy(renderer, texture, NULL, &rect);
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
			rect = { x, y, TILE_SIZE / 4, TILE_SIZE / 3 };
			dirX = 0;
			dirY = -1;
		}
		void move() {
			int new_Y = y + speed;
			y = new_Y;
			rect.x = x;
			rect.y = y;
		}
		void render(SDL_Renderer* renderer,SDL_Texture* texture) {
			SDL_RenderCopy(renderer, texture, NULL, &rect);
		}
	};
	class Player {
	public:
		int health = 3;
		int x, y;
		int dirX, dirY;
		SDL_Rect rect;
		Player() : x(0), y(0), dirX(0), dirY(0) { rect = { x, y, TILE_SIZE* 2, TILE_SIZE /2 }; }
		Player(int startX, int startY) {
			x = startX;
			y = startY;
			rect = { x, y, TILE_SIZE , TILE_SIZE * 3/2 };
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
		void render(SDL_Renderer* renderer,SDL_Texture* texture) {
			SDL_RenderCopy(renderer, texture, NULL, &rect);
		}

	};
	class Fodders {
	public:
		int x, y;
		double dirX, dirY;
		int health = 1;
		int damage = 1;
		double speed = 6;
		SDL_Rect rect;
		int lastDirectionChange = SDL_GetTicks();
		Fodders(int startX, int startY, int targetX) {
			x = startX;
			y = startY;
			rect = { x, y, TILE_SIZE , TILE_SIZE};
			UpdateDirection(targetX);
		}
		void UpdateDirection(int targetX) {
			double toX = targetX - x;
			double toY = SCREEN_HEIGHT - y;
			double length = sqrt(toX * toX + toY * toY);
			dirX = (toX / length) * speed;
			dirY = (toY / length) * speed;
		}

		void move(Player& player) {
			Uint32 currentTime = SDL_GetTicks();
			if (currentTime - lastDirectionChange >= 200) {
				UpdateDirection(player.x);
				lastDirectionChange = currentTime;
			}
			x += dirX;
			y += dirY;
			if (player.y < y) {
				y += speed / 5;
			}
			rect.x = x;
			rect.y = y;
		}
		void render(SDL_Renderer* renderer, SDL_Texture* texture) {
			SDL_RenderCopy(renderer, texture, NULL, &rect);
		}
	};
	class Game {
	public:
		SDL_Texture* loadTexture(const char* path) {
			SDL_Surface* loadedSurface = IMG_Load(path);
			if (!loadedSurface) {
				std::cerr << "Failed to load image: " << path << " SDL_image Error: " << IMG_GetError() << std::endl;
				return nullptr;
			}
			SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
			SDL_FreeSurface(loadedSurface);
			return newTexture;
		}
		Mix_Chunk* loadSound(const char* path) {
			Mix_Chunk* sound = Mix_LoadWAV(path);
			if (!sound) {
				std::cerr << "Failed to load sound: " << Mix_GetError() << std::endl;
			}
			return sound;
		}
		Mix_Music* loadMusic(const char* path) {
			Mix_Music* music = Mix_LoadMUS(path);
			if (!music) {
				std::cerr << "Failed to load music: " << Mix_GetError() << std::endl;
			}
			return music;
		}
		Player player;
		SDL_Window* window;
		SDL_Renderer* renderer;
		Mix_Chunk* fumo_hit = NULL;
		Mix_Chunk* player_hit = NULL;
		Mix_Chunk* fodder_hit = NULL;
		Mix_Music* music = NULL;
		std::vector<Player_Bullet> bullets;
		Enemy enemy;
		std::vector<Enemy_Bullet> enemy_bullets;
		std::vector<Fodders> fodders;
		SDL_Texture* enemy_texture = NULL;
		SDL_Texture* enemy_bullet_texture = NULL;
		SDL_Texture* fodder_texture = NULL;
		SDL_Texture* player_texture = NULL;
		SDL_Texture* bullet_texture = NULL;
		SDL_Texture* background_texture = NULL;
		SDL_Texture* start_button = NULL;
		SDL_Texture* quit_button = NULL;
		SDL_Rect start_rect = { SCREEN_WIDTH / 2 - (TILE_SIZE * 2 * 7 / 4) / 2, SCREEN_HEIGHT * 2 / 3,TILE_SIZE * 2 * 7 / 4, TILE_SIZE * 5 / 3 };
		SDL_Rect quit_rect = { SCREEN_WIDTH / 2 - TILE_SIZE, SCREEN_HEIGHT * 2 / 3 + TILE_SIZE * 5 / 3 + TILE_SIZE / 2, TILE_SIZE * 2, TILE_SIZE * 4 / 3 };
		bool menu_running;
		bool running;
		Game() : enemy(SCREEN_WIDTH / 2.2, SCREEN_HEIGHT / 6) {
			menu_running = true;
			running = false;
			if (SDL_Init(SDL_INIT_AUDIO) < 0) {
				std::cerr << "SDL Init Error: " << SDL_GetError() << "\n";
			}
			if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
				std::cerr << "Mix_OpenAudio Error: " << Mix_GetError() << "\n";
			}
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
			player_hit = loadSound("sound/player_hit.wav");
			fumo_hit = loadSound("sound/fumo_hit.wav");
			fodder_hit = loadSound("sound/ghost_hit.wav");
			enemy_texture = loadTexture("img/cirno_fumo.png");
			enemy_bullet_texture = loadTexture("img/snowyflake.png");
			fodder_texture = loadTexture("img/fairy.png");
			player_texture = loadTexture("img/REI.png");
			bullet_texture = loadTexture("img/bullet.png");
			background_texture = loadTexture("img/background.png");
			start_button = loadTexture("img/start.png");
			quit_button = loadTexture("img/quit.png");
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
					enemy.move(-3, enemy.y_speed);
				}
				else if (enemy_center < player_center) {
					enemy.move(3, enemy.y_speed);
				}
				Uint32 enemy_currentTime = SDL_GetTicks();
				if (enemy_currentTime - last_enemy_shot_time >= ENEMY_SHOT_DELAY) {
					enemy_bullets.push_back(Enemy_Bullet(enemy_center, enemy.y + TILE_SIZE * 3, 0, 1));
					enemy_bullets.push_back(Enemy_Bullet(enemy_center + TILE_SIZE, enemy.y + TILE_SIZE * 3, 0.75, 0.85));
					enemy_bullets.push_back(Enemy_Bullet(enemy_center - TILE_SIZE, enemy.y + TILE_SIZE * 3, -0.75, 0.85));
					enemy_bullets.push_back(Enemy_Bullet(enemy_center - TILE_SIZE * 2, enemy.y + TILE_SIZE, -1, 0));
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
			SDL_RenderCopy(renderer, background_texture, NULL, NULL);
			for (int i = 0; i < fodders.size(); ++i) {
				fodders[i].render(renderer, fodder_texture);
			}
			player.render(renderer, player_texture);
			for (int i = 0; i < bullets.size(); ++i) {
				bullets[i].render(renderer, bullet_texture);
			}
			for (int i = 0; i < enemy_bullets.size(); ++i) {
				enemy_bullets[i].render(renderer, enemy_bullet_texture);
			}
			
				enemy.render(renderer, enemy_texture);
			
			SDL_RenderPresent(renderer);

		}
		void render_menu() {
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			SDL_RenderCopy(renderer, background_texture, NULL, NULL);
			SDL_RenderCopy(renderer, start_button, NULL, &start_rect);
			SDL_RenderCopy(renderer, quit_button, NULL, &quit_rect);
			SDL_RenderPresent(renderer);
		}
		void menu() {
			SDL_Event event;
			while (menu_running) {
				render_menu();
				while (SDL_PollEvent(&event)) {
					if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
						SDL_Point mousePoint = { event.button.x, event.button.y };
						if (SDL_PointInRect(&mousePoint, &start_rect)) {
							running = true;
							menu_running = false;
						}
						else if (SDL_PointInRect(&mousePoint, &quit_rect)) {
							menu_running = false;
							running = false;
						}
					}
				}
			}
		}
		void run() {
			while (menu_running) {
				menu();
			}
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
				player.move(0, -5);
			}
			if (keystates[SDL_SCANCODE_S]) {
				player.move(0, 5);
			}
			if (keystates[SDL_SCANCODE_A]) {
				player.move(-5, 0);
			}
			if (keystates[SDL_SCANCODE_D]) {
				player.move(5, 0);
			}
			Uint32 currentTime = SDL_GetTicks();
			if (keystates[SDL_SCANCODE_SPACE]) {
				if (currentTime - last_shot_time >= SHOT_DELAY) {

					bullets.push_back(Player_Bullet(player.x + TILE_SIZE / 3, player.y + TILE_SIZE / 3 - TILE_SIZE + 4));
					last_shot_time = currentTime;
				}
			}
			for (int i = fodders.size() - 1; i >= 0; --i) {
				if (SDL_HasIntersection(&player.rect, &fodders[i].rect)) {
					player.health -= 1;
					int channel = Mix_PlayChannel(-1, player_hit, 0);
					Mix_Volume(channel, 64);
					fodders.erase(fodders.begin() + i);
				}
			}
			for (int i = enemy_bullets.size() - 1; i >= 0; --i) {
				if (SDL_HasIntersection(&player.rect, &enemy_bullets[i].rect)) {
					player.health -= enemy_bullets[i].damage;
					int channel = Mix_PlayChannel(-1, player_hit, 0);
					Mix_Volume(channel, 64);
					enemy_bullets.erase(enemy_bullets.begin() + i);
				}
			}
			for (int i = bullets.size() - 1; i >= 0; --i) {
				for (int j = fodders.size() - 1; j >= 0; --j) {
					if (SDL_HasIntersection(&bullets[i].rect, &fodders[j].rect)) {
						fodders[j].health -= bullets[i].damage;
						bullets.erase(bullets.begin() + i);
						if (fodders[j].health <= 0) {
							int channel = Mix_PlayChannel(-1, fodder_hit, 0);
							Mix_Volume(channel, 9);
							fodders.erase(fodders.begin() + j);
						}
						break;
					}
				}
			}
			for (int i = bullets.size() - 1; i >= 0; i--) {
				if (SDL_HasIntersection(&bullets[i].rect, &enemy.rect)) {
					enemy.health -= bullets[i].damage;
					int channel = Mix_PlayChannel(-1, fumo_hit, 0);
					Mix_Volume(channel, 15);
					bullets.erase(bullets.begin() + i);
					if (enemy.health <= 0) {
						enemy.alive = false;
					}
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
		if (game.running || game.menu_running) {
			game.run();
			SDL_Delay(16);
		}
		return 0;
	}