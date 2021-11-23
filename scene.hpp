#ifndef ASTEROIDS_SCENE_HPP
#define ASTEROIDS_SCENE_HPP

#include <array>
#include <random>
#include <SDL_keycode.h>
#include "utility.hpp"
#include "entities.hpp"

namespace asteroids
{
	inline const mesh PLAYER_MESH{
		mesh({
			{-30,-30},
			{30,0},
			{-30,30}
		})
	};

	inline const mesh DESTRUCTION_FRAGMENT_MESH{
		{
			{-10,-10},
			{10,-10},
			{10,10},
			{-10,10}
		}
	};

	inline const std::array<mesh,2> BIG_ROCK_MESHES{
		mesh{{
			{-25,-50},
			{45,-45},
			{65,35},
			{25,50},
			{-50,45},
		}},
		mesh{{
			{-55,-55},
			{35,-45},
			{45,45},
			{-45,55}
		}}
	};

	inline const std::array<mesh,2> SMALL_ROCK_MESHES{
		mesh{{
			{-10,-27.5f},
			{25,-25},
			{35,20},
			{15,27.5f},
			{-27.5f,25},
		}},
		mesh{{
			{-30,-30},
			{20,-25},
			{25,25},
			{-25,30}
		}}
	};

	inline const mesh BULLET_MESH{
		{
			{-2.5f,-2.5f},
			{+2.5f,-2.5f},
			{+2.5f,+2.5f},
			{-2.5f,+2.5f}
		}
	};

	inline const mesh UFO_MESH{
		{
			{-10,-20},
			{10,-20},
			{10,0},
			{40,0},
			{40,20},
			{-40,20},
			{-40,0},
			{-10,0},
		}
	};

	struct rock_template
	{
		mesh mesh;
		float speed;
		std::uintmax_t aword_points;
		bool spawns_smaller_rocks_on_desstruction;
	};

	inline const std::array<rock_template,4> ROCK_TEMPLATES{
		rock_template{BIG_ROCK_MESHES[0],300,100,true},
		rock_template{BIG_ROCK_MESHES[1],300,100,true},
		rock_template{SMALL_ROCK_MESHES[0],350,150,false},
		rock_template{SMALL_ROCK_MESHES[1],350,150,false}
	};

	inline const std::array<rock_template,2> SMALL_ROCK_TEMPLATES{
		rock_template{SMALL_ROCK_MESHES[0],300,150,false},
		rock_template{SMALL_ROCK_MESHES[1],300,150,false}
	};

	inline constexpr std::array<SDL_FPoint,8> ROCK_SPAWN_POINTS{
		SDL_FPoint{-25,-25},
		SDL_FPoint{512,-25},
		SDL_FPoint{1049,-25},
		SDL_FPoint{1049,384},
		SDL_FPoint{1049,793},
		SDL_FPoint{512,793},
		SDL_FPoint{-25,793},
		SDL_FPoint{-25,384}
	};

	class scene
	{
		void spawn_destruction_particles(SDL_FPoint position,std::size_t count);
	public:
		scene();
		scene(const scene&) = delete;
		scene& operator = (const scene&) = delete;

		void update(float delta_time,const std::array<bool,SDL_NUM_SCANCODES>& keyboard_keys,const std::array<bool,SDL_NUM_SCANCODES>& once_keyboard_keys);
		const player& get_player() const;
		const std::vector<rock>& get_rocks() const;
		const std::vector<projectile>& get_projectiles() const;
		const std::vector<ufo>& get_ufos() const;
	private:
		std::mt19937_64 random_engine{};
		player player;
		std::vector<rock> rocks{};
		float max_rock_spawn_timer{};
		float rock_spawn_timer{};
		std::vector<projectile> projectiles{};
		float max_ufo_spawn_timer{};
		float ufo_spawn_timer{};
		std::vector<ufo> ufos{};
	};
}

#endif