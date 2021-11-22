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
		SDL_FRect get_bounding_box() const;
		SDL_FRect get_transformed_bounding_box() const;
		const std::vector<SDL_FPoint>& get_transformed_vertices() const;

	private:
		std::vector<SDL_FPoint> vertices{};
		std::vector<SDL_FPoint> transformed_vertices{};
		SDL_FRect bounding_box{};
		SDL_FRect transformed_bounding_box{};
		std::vector<SDL_FPoint> transformed_edge_normals{};
	};

	class entity
	{
	public:
		

	private:
	};
}

#endif