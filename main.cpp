#include <array>
#include <cmath>
#include <vector>
#include <random>
#include <limits>
#include <chrono>
#include <iostream>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>

#include "scene.hpp"
#include "utility.hpp"
#include "entities.hpp"

void render_mesh(SDL_Renderer* renderer,const asteroids::mesh& mesh)
{
	const auto& vertices = mesh.get_transformed_vertices();
	if(vertices.size() > 0)
	{
		SDL_RenderDrawLinesF(renderer,vertices.data(),static_cast<int>(vertices.size()));
		SDL_RenderDrawLineF(renderer,vertices.back().x,vertices.back().y,vertices.front().x,vertices.front().y);
	}
}

int main()
{
	SDL_SetMainReady();
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't init SDL library.",nullptr);
		return 1;
	}
	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't init SDL2_image library.",nullptr);
		SDL_Quit();
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Asteroids Clone",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1024,768,SDL_WINDOW_SHOWN);
	if(!window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't create a window.",nullptr);
		IMG_Quit();
		SDL_Quit();
		return 1;
	}
	SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
	if(!renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't create a renderer.",nullptr);
		SDL_DestroyWindow(window);
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	asteroids::scene scene{};
	std::array<bool,SDL_NUM_SCANCODES> keyboard_keys{};
	std::array<bool,SDL_NUM_SCANCODES> keyboard_keys_once{};

	Uint64 timer_start = SDL_GetPerformanceCounter();
	SDL_Event event{};
	bool is_running = true;
	while(is_running)
	{
		Uint64 timer_end = SDL_GetPerformanceCounter();
		float delta_time = static_cast<float>(timer_end - timer_start) / SDL_GetPerformanceFrequency();
		timer_start = timer_end;

		keyboard_keys_once.fill(false);
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					is_running = false;
				break;
				case SDL_KEYDOWN:
					keyboard_keys[event.key.keysym.scancode] = true;
					if(event.key.repeat == 0)
					{
						keyboard_keys_once[event.key.keysym.scancode] = true;
					}
				break;
				case SDL_KEYUP:
					keyboard_keys[event.key.keysym.scancode] = false;
				break;
			}
		}

		if(keyboard_keys_once[SDL_SCANCODE_ESCAPE])
		{
			is_running = false;
		}

		scene.update(delta_time,keyboard_keys,keyboard_keys_once);

		SDL_SetRenderDrawColor(renderer,0,0,0,255);
		SDL_RenderClear(renderer);

		const auto& player = scene.get_player();
		if(player.is_invulnerable())
		{
			SDL_SetRenderDrawColor(renderer,0,128,255,255);
		}
		else
		{
			SDL_SetRenderDrawColor(renderer,255,255,255,255);
		}
		if(!player.is_dead())
		{
			render_mesh(renderer,player.get_mesh());
		}

		SDL_SetRenderDrawColor(renderer,255,255,255,255);
		for(const auto& rock : scene.get_rocks())
		{
			render_mesh(renderer,rock.get_mesh());
		}

		for(const auto& projectile : scene.get_projectiles())
		{
			if(!projectile.physical)
			{
				SDL_SetRenderDrawColor(renderer,0,128,255,255);
			}
			else if(projectile.player_friendly)
			{
				SDL_SetRenderDrawColor(renderer,255,255,255,255);
			}
			else
			{
				SDL_SetRenderDrawColor(renderer,255,0,0,255);
			}
			render_mesh(renderer,projectile.get_mesh());
		}

		SDL_SetRenderDrawColor(renderer,255,0,0,255);
		for(const auto& ufo : scene.get_ufos())
		{
			render_mesh(renderer,ufo.get_mesh());
		}

		SDL_RenderPresent(renderer);
	}
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}