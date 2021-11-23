#ifndef ASTEROIDS_ENTITIES_HPP
#define ASTEROIDS_ENTITIES_HPP

#include <vector>
#include <SDL_rect.h>

namespace asteroids
{
	class mesh
	{
	public:
		SDL_FPoint position{};
		float rotation{};

		mesh(const std::vector<SDL_FPoint>& _vertices,SDL_FPoint _position = {0,0},float _rotation = 0);
		mesh(const mesh& _mesh);
		mesh(mesh&& _mesh) noexcept;
		mesh& operator = (const mesh& _mesh);
		mesh& operator = (mesh&& _mesh) noexcept;

		void update();
		bool check_collision_with(const mesh& other) const;
		SDL_FRect get_transformed_bounding_box() const;
		const std::vector<SDL_FPoint>& get_transformed_vertices() const;

	private:
		std::vector<SDL_FPoint> vertices{};
		std::vector<SDL_FPoint> transformed_vertices{};
		SDL_FRect transformed_bounding_box{};
		std::vector<SDL_FPoint> transformed_edge_normals{};
	};

	class entity
	{
	public:
		SDL_FPoint position{};
		float rotation{};
		float move_speed{};
		float rotation_speed{};
		bool destroyed{};

		entity(SDL_FPoint _position,float _rotation,float _move_speed,float _rotation_speed,const mesh& _mesh);
		entity(const entity& _entity);
		entity(entity&& _entity) noexcept;
		entity& operator = (const entity& _entity);
		entity& operator = (entity&& _entity) noexcept;

		void update();
		const mesh& get_mesh() const;
		SDL_FPoint get_forward() const;

	private:
		mesh mesh;
		SDL_FPoint forward{};
	};

	class player : public entity
	{
	public:
		SDL_FPoint velocity{};
		std::uintmax_t points{};
		float max_invulnerability_timer{};
		float max_respawn_timer{};
		float max_shoot_timer{};

		player(SDL_FPoint _position,float _rotation,float _move_speed,float _rotation_speed,const asteroids::mesh& _mesh);
		player(const player& _player);
		player(player&& _player) noexcept;
		player& operator = (const player& _player);
		player& operator = (player&& _player) noexcept;

		void update(float delta_time);
		bool is_dead() const noexcept;
		bool is_invulnerable() const noexcept;
		bool can_shoot() const noexcept;
		void make_it_shoot() noexcept;
		void make_invulnerable() noexcept;
		void kill() noexcept;

	private:
		bool dead{};
		float invulnerability_timer{};
		float respawn_timer{};
		float shoot_timer{};
	};

	class rock : public entity
	{
	public:
		std::uintmax_t award_points{};
		bool spawns_smaller_rocks_on_destruction{};

		rock(SDL_FPoint _position,float _rotation,float _move_speed,std::uintmax_t _award_points,bool _spawns_smaller_rocks_on_destruction,const asteroids::mesh& _mesh);
		rock(const rock& _rock);
		rock(rock&& _rock) noexcept;
		rock& operator = (const rock& _rock);
		rock& operator = (rock&& _rock) noexcept;

		void update(float delta_time);
	};

	class projectile : public entity
	{
	public:
		bool physical{};
		bool player_friendly{};

		projectile(SDL_FPoint _position,float _rotation,float _move_speed,bool _physical,bool _player_friendly,const asteroids::mesh& _mesh);
		projectile(const projectile& _projectile);
		projectile(projectile&& _projectile) noexcept;
		projectile& operator = (const projectile& _projectile);
		projectile& operator = (projectile&& _projectile) noexcept;

		void update(float delta_time);
	};

	class ufo : public entity
	{
	public:
		std::uintmax_t award_points{};
		float max_shoot_timer{};
		SDL_FPoint direction{};

		ufo(SDL_FPoint _position,float _move_speed,std::uintmax_t _award_points,float _max_shoot_timer,SDL_FPoint _direction,const asteroids::mesh& _mesh);
		ufo(const ufo& _ufo);
		ufo(ufo&& _ufo) noexcept;
		ufo& operator = (const ufo& _ufo);
		ufo& operator = (ufo&& _ufo) noexcept;

		bool can_shoot() const noexcept;
		void make_it_shoot() noexcept;
		void update(float delta_time);

	private:
		float shoot_timer{};
	};
}

#endif