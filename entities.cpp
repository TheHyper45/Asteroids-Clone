#include "entities.hpp"

#include <cmath>
#include "utility.hpp"

namespace asteroids
{
	mesh::mesh(const std::vector<SDL_FPoint>& _vertices,SDL_FPoint _position,float _rotation)
		: vertices(_vertices),position(_position),rotation(_rotation)
	{}

	mesh::mesh(const mesh& _mesh)
	{
		*this = _mesh;
	}

	mesh::mesh(mesh&& _mesh) noexcept
	{
		*this = std::move(_mesh);
	}

	mesh& mesh::operator = (const mesh& _mesh)
	{
		if(&_mesh != this)
		{
			position = _mesh.position;
			rotation = _mesh.rotation;
			vertices = _mesh.vertices;
			transformed_vertices = _mesh.transformed_vertices;
			bounding_box = _mesh.bounding_box;
			transformed_bounding_box = _mesh.transformed_bounding_box;
			transformed_edge_normals = _mesh.transformed_edge_normals;
		}
		return *this;
	}

	mesh& mesh::operator = (mesh&& _mesh) noexcept
	{
		if(&_mesh != this)
		{
			position = _mesh.position;
			rotation = _mesh.rotation;
			vertices = std::move(_mesh.vertices);
			transformed_vertices = std::move(_mesh.transformed_vertices);
			bounding_box = _mesh.bounding_box;
			transformed_bounding_box = _mesh.transformed_bounding_box;
			transformed_edge_normals = std::move(_mesh.transformed_edge_normals);
		}
		return *this;
	}

	void mesh::update()
	{
		transformed_vertices.clear();
		transformed_edge_normals.clear();

		SDL_FPoint bounding_box_min{
			std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity()
		};
		SDL_FPoint bounding_box_max{
			std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity()
		};
		SDL_FPoint bounding_box_min_transformed{
			std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity()
		};
		SDL_FPoint bounding_box_max_transformed{
			std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity()
		};

		for(const auto& vertex : vertices)
		{
			if(std::isinf(bounding_box_min.x) || bounding_box_min.x > vertex.x)
			{
				bounding_box_min.x = vertex.x;
			}
			if(std::isinf(bounding_box_min.y) || bounding_box_min.y > vertex.y)
			{
				bounding_box_min.y = vertex.y;
			}
			if(std::isinf(bounding_box_max.x) || bounding_box_max.x < vertex.x)
			{
				bounding_box_max.x = vertex.x;
			}
			if(std::isinf(bounding_box_max.y) || bounding_box_max.y < vertex.y)
			{
				bounding_box_max.y = vertex.y;
			}

			SDL_FPoint new_point{};
			new_point.x = std::cos(rotation) * vertex.x - std::sin(rotation) * vertex.y + position.x;
			new_point.y = std::sin(rotation) * vertex.x + std::cos(rotation) * vertex.y + position.y;
			if(std::isinf(bounding_box_min_transformed.x) || bounding_box_min_transformed.x > new_point.x)
			{
				bounding_box_min_transformed.x = new_point.x;
			}
			if(std::isinf(bounding_box_min_transformed.y) || bounding_box_min_transformed.y > new_point.y)
			{
				bounding_box_min_transformed.y = new_point.y;
			}
			if(std::isinf(bounding_box_max_transformed.x) || bounding_box_max_transformed.x < new_point.x)
			{
				bounding_box_max_transformed.x = new_point.x;
			}
			if(std::isinf(bounding_box_max_transformed.y) || bounding_box_max_transformed.y < new_point.y)
			{
				bounding_box_max_transformed.y = new_point.y;
			}
			transformed_vertices.push_back(new_point);
		}
		bounding_box.x = bounding_box_min.x;
		bounding_box.y = bounding_box_min.y;
		bounding_box.w = bounding_box_max.x - bounding_box_min.x;
		bounding_box.h = bounding_box_max.y - bounding_box_min.y;
		transformed_bounding_box.x = bounding_box_min_transformed.x;
		transformed_bounding_box.y = bounding_box_min_transformed.y;
		transformed_bounding_box.w = bounding_box_max_transformed.x - bounding_box_min_transformed.x;
		transformed_bounding_box.h = bounding_box_max_transformed.y - bounding_box_min_transformed.y;

		std::size_t length = transformed_vertices.size();
		for(std::size_t i = 0;i < length;++i)
		{
			SDL_FPoint current = transformed_vertices[i];
			SDL_FPoint next = transformed_vertices[(i + 1) % length];
			SDL_FPoint diff{
				next.x - current.x,
				next.y - current.y
			};
			transformed_edge_normals.push_back(perpendicular(normalize(diff)));
		}
	}

	bool mesh::check_collision_with(const mesh & other) const
	{
		if(!intersect_rects(bounding_box,other.bounding_box))
		{
			return false;
		}
		auto for_each_normal = [](	const std::vector<SDL_FPoint>& in_normals,
									const std::vector<SDL_FPoint>& transformed_vertices,
									const std::vector<SDL_FPoint>& other_transformed_vertices	)
		{
			for(const auto& normal : in_normals)
			{
				float min = std::numeric_limits<float>::infinity();
				float max = std::numeric_limits<float>::infinity();
				float other_min = std::numeric_limits<float>::infinity();
				float other_max = std::numeric_limits<float>::infinity();
				for(const auto& vertex : transformed_vertices)
				{
					float value = dot_product(normal,vertex);
					if(std::isinf(min) || value < min)
					{
						min = value;
					}
					if(std::isinf(max) || value > max)
					{
						max = value;
					}
				}
				for(const auto& vertex : other_transformed_vertices)
				{
					float value = dot_product(normal,vertex);
					if(std::isinf(other_min) || value < other_min)
					{
						other_min = value;
					}
					if(std::isinf(other_max) || value > other_max)
					{
						other_max = value;
					}
				}
				if(!((min < other_max && min > other_min) || (other_min < max && other_min > min)))
				{
					return false;
				}
			}
			return true;
		};

		if(!for_each_normal(transformed_edge_normals,transformed_vertices,other.transformed_vertices))
		{
			return false;
		}
		return for_each_normal(other.transformed_edge_normals,transformed_vertices,other.transformed_vertices);
	}

	SDL_FRect mesh::get_bounding_box() const
	{
		return bounding_box;
	}

	SDL_FRect mesh::get_transformed_bounding_box() const
	{
		return transformed_bounding_box;
	}

	const std::vector<SDL_FPoint>& mesh::get_transformed_vertices() const
	{
		return transformed_vertices;
	}
}