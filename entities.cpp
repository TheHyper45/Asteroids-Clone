#include "entities.hpp"

#include <cmath>
#include <iostream>
#include "utility.hpp"

namespace asteroids
{
	mesh::mesh(const std::vector<SDL_FPoint>& _vertices,SDL_FPoint _position,float _rotation)
		: vertices(_vertices),position(_position),rotation(_rotation)
	{
		update();
	}

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
			transformed_bounding_box = _mesh.transformed_bounding_box;
			transformed_edge_normals = _mesh.transformed_edge_normals;
			update();
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
			transformed_bounding_box = _mesh.transformed_bounding_box;
			transformed_edge_normals = std::move(_mesh.transformed_edge_normals);
			update();

		}
		return *this;
	}

	void mesh::update()
	{
		transformed_vertices = {};
		transformed_edge_normals = {};
		transformed_bounding_box = {};

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
		if(!intersect_rects(transformed_bounding_box,other.transformed_bounding_box))
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

	SDL_FRect mesh::get_transformed_bounding_box() const
	{
		return transformed_bounding_box;
	}

	const std::vector<SDL_FPoint>& mesh::get_transformed_vertices() const
	{
		return transformed_vertices;
	}

	entity::entity(SDL_FPoint _position,float _rotation,float _move_speed,float _rotation_speed,const asteroids::mesh & _mesh)
		: position(_position),rotation(_rotation),move_speed(_move_speed),rotation_speed(_rotation_speed),$mesh(_mesh)
	{
		update();
	}

	entity::entity(const entity& _entity)
		: position(_entity.position),rotation(_entity.rotation),move_speed(_entity.move_speed),
			rotation_speed(_entity.rotation_speed),$mesh(_entity.$mesh),destroyed(_entity.destroyed),forward(_entity.forward)
	{
		update();
	}

	entity::entity(entity&& _entity) noexcept
		: position(_entity.position),rotation(_entity.rotation),move_speed(_entity.move_speed),
		rotation_speed(_entity.rotation_speed),$mesh(std::move(_entity.$mesh)),destroyed(_entity.destroyed),forward(_entity.forward)
	{
		update();
	}

	entity& entity::operator = (const entity& _entity)
	{
		if(&_entity != this)
		{
			position = _entity.position;
			rotation = _entity.rotation;
			move_speed = _entity.move_speed;
			rotation_speed = _entity.rotation_speed;
			$mesh = _entity.$mesh;
			destroyed = _entity.destroyed;
			forward = _entity.forward;
			update();
		}
		return *this;
	}

	entity& entity::operator = (entity&& _entity) noexcept
	{
		if(&_entity != this)
		{
			position = _entity.position;
			rotation = _entity.rotation;
			move_speed = _entity.move_speed;
			rotation_speed = _entity.rotation_speed;
			$mesh = std::move(_entity.$mesh);
			destroyed = _entity.destroyed;
			forward = _entity.forward;
			_entity.$mesh = {{}};
		}
		return *this;
	}

	void entity::update()
	{
		$mesh.position = position;
		$mesh.rotation = rotation;
		$mesh.update();
		forward.x = std::cos(rotation);
		forward.y = std::sin(rotation);
	}

	const mesh& entity::get_mesh() const
	{
		return $mesh;
	}

	SDL_FPoint entity::get_forward() const
	{
		return forward;
	}

	player::player(SDL_FPoint _position,float _rotation,float _move_speed,float _rotation_speed,const asteroids::mesh& _mesh)
		: entity(_position,_rotation,_move_speed,_rotation_speed,_mesh)
	{}

	player::player(const player& _player) : entity(_player),
											points(_player.points),dead(_player.dead),max_invulnerability_timer(_player.max_invulnerability_timer),
											max_respawn_timer(_player.max_respawn_timer),max_shoot_timer(_player.max_shoot_timer),invulnerability_timer(_player.invulnerability_timer),
											respawn_timer(_player.respawn_timer),shoot_timer(_player.shoot_timer),velocity(_player.velocity)
	{}

	player::player(player&& _player) noexcept : entity(std::move(_player)),
												points(_player.points),dead(_player.dead),max_invulnerability_timer(_player.max_invulnerability_timer),
												max_respawn_timer(_player.max_respawn_timer),max_shoot_timer(_player.max_shoot_timer),invulnerability_timer(_player.invulnerability_timer),
												respawn_timer(_player.respawn_timer),shoot_timer(_player.shoot_timer),velocity(_player.velocity)
	{}

	player& player::operator = (const player& _player)
	{
		if(&_player != this)
		{
			entity::operator=(_player);
			points = _player.points;
			dead = _player.dead;
			max_invulnerability_timer = _player.max_invulnerability_timer;
			max_respawn_timer = _player.max_respawn_timer;
			max_shoot_timer = _player.max_shoot_timer;
			invulnerability_timer = _player.invulnerability_timer;
			respawn_timer = _player.respawn_timer;
			shoot_timer = _player.shoot_timer;
			velocity = _player.velocity;
		}
		return *this;
	}

	player& player::operator = (player&& _player) noexcept
	{
		if(&_player != this)
		{
			entity::operator=(std::move(_player));
			points = _player.points;
			dead = _player.dead;
			max_invulnerability_timer = _player.max_invulnerability_timer;
			max_respawn_timer = _player.max_respawn_timer;
			max_shoot_timer = _player.max_shoot_timer;
			invulnerability_timer = _player.invulnerability_timer;
			respawn_timer = _player.respawn_timer;
			shoot_timer = _player.shoot_timer;
			velocity = _player.velocity;
		}
		return *this;
	}

	void player::update(float delta_time)
	{
		entity::update();
		if(invulnerability_timer > 0.0f)
		{
			invulnerability_timer -= delta_time;
			if(invulnerability_timer < 0.0f)
			{
				invulnerability_timer = 0.0f;
			}
		}

		if(shoot_timer > 0.0f)
		{
			shoot_timer -= delta_time;
			if(shoot_timer < 0.0f)
			{
				shoot_timer = 0.0f;
			}
		}

		if(respawn_timer > 0.0f)
		{
			respawn_timer -= delta_time;
			if(respawn_timer < 0.0f)
			{
				respawn_timer = 0.0f;
				dead = false;
				position = {512,384}; //TODO: This shouldn't be hardcoded.
				rotation = 0;
				velocity = {};
				make_invulnerable();
			}
		}
	}

	bool player::is_dead() const noexcept
	{
		return dead;
	}

	bool player::is_invulnerable() const noexcept
	{
		return invulnerability_timer > 0.0f;
	}

	bool player::can_shoot() const noexcept
	{
		return shoot_timer <= 0.0f;
	}

	void player::make_it_shoot() noexcept
	{
		shoot_timer = max_shoot_timer;
	}

	void player::make_invulnerable() noexcept
	{
		invulnerability_timer = max_invulnerability_timer;
	}

	void player::kill() noexcept
	{
		dead = true;
		respawn_timer = max_respawn_timer;
	}

	rock::rock(SDL_FPoint _position,float _rotation,float _move_speed,std::uintmax_t _award_points,bool _spawns_smaller_rocks_on_destruction,const asteroids::mesh& _mesh)
		 : entity(_position,_rotation,_move_speed,0,_mesh),award_points(_award_points),spawns_smaller_rocks_on_destruction(_spawns_smaller_rocks_on_destruction)
	{}

	rock::rock(const rock& _rock) : entity(_rock),award_points(_rock.award_points),spawns_smaller_rocks_on_destruction(_rock.spawns_smaller_rocks_on_destruction)
	{}

	rock::rock(rock&& _rock) noexcept : entity(std::move(_rock)),award_points(_rock.award_points),spawns_smaller_rocks_on_destruction(_rock.spawns_smaller_rocks_on_destruction)
	{}

	rock& rock::operator = (const rock& _rock)
	{
		if(&_rock != this)
		{
			entity::operator=(_rock);
			award_points = _rock.award_points;
			spawns_smaller_rocks_on_destruction = _rock.spawns_smaller_rocks_on_destruction;
		}
		return *this;
	}

	rock& rock::operator = (rock&& _rock) noexcept
	{
		if(&_rock != this)
		{
			entity::operator=(std::move(_rock));
			award_points = _rock.award_points;
			spawns_smaller_rocks_on_destruction = _rock.spawns_smaller_rocks_on_destruction;
		}
		return *this;
	}

	void rock::update(float delta_time)
	{
		entity::update();
		position.x += get_forward().x * move_speed * delta_time;
		position.y += get_forward().y * move_speed * delta_time;
	}

	projectile::projectile(SDL_FPoint _position,float _rotation,float _move_speed,bool _physical,bool _player_friendly,const asteroids::mesh& _mesh)
		: entity(_position,_rotation,_move_speed,0,_mesh),physical(_physical),player_friendly(_player_friendly)
	{}

	projectile::projectile(const projectile& _projectile) : entity(_projectile),physical(_projectile.physical),player_friendly(_projectile.player_friendly)
	{}

	projectile::projectile(projectile&& _projectile) noexcept : entity(std::move(_projectile)),physical(_projectile.physical),player_friendly(_projectile.player_friendly)
	{}

	projectile& projectile::operator = (const projectile& _projectile)
	{
		if(&_projectile != this)
		{
			entity::operator=(_projectile);
			physical = _projectile.physical;
			player_friendly = _projectile.player_friendly;
		}
		return *this;
	}

	projectile& projectile::operator = (projectile&& _projectile) noexcept
	{
		if(&_projectile != this)
		{
			entity::operator=(std::move(_projectile));
			physical = _projectile.physical;
			player_friendly = _projectile.player_friendly;
		}
		return *this;
	}

	void projectile::update(float delta_time)
	{
		entity::update();
		position.x += get_forward().x * move_speed * delta_time;
		position.y += get_forward().y * move_speed * delta_time;
	}

	ufo::ufo(SDL_FPoint _position,float _move_speed,std::uintmax_t _award_points,float _max_shoot_timer,SDL_FPoint _direction,const asteroids::mesh& _mesh)
		: entity(_position,0,_move_speed,0,_mesh),award_points(_award_points),max_shoot_timer(_max_shoot_timer),direction(_direction)
	{}

	ufo::ufo(const ufo& _ufo)
		: entity(_ufo),award_points(_ufo.award_points),max_shoot_timer(_ufo.max_shoot_timer),direction(_ufo.direction)
	{}

	ufo::ufo(ufo&& _ufo) noexcept
		: entity(std::move(_ufo)),award_points(_ufo.award_points),max_shoot_timer(_ufo.max_shoot_timer),direction(_ufo.direction)
	{}

	ufo& ufo::operator = (const ufo& _ufo)
	{
		if(&_ufo != this)
		{
			entity::operator=(_ufo);
			award_points = _ufo.award_points;
			max_shoot_timer = _ufo.max_shoot_timer;
			direction = _ufo.direction;
		}
		return *this;
	}

	ufo& ufo::operator = (ufo&& _ufo) noexcept
	{
		if(&_ufo != this)
		{
			entity::operator=(std::move(_ufo));
			award_points = _ufo.award_points;
			max_shoot_timer = _ufo.max_shoot_timer;
			direction = _ufo.direction;
		}
		return *this;
	}

	bool ufo::can_shoot() const noexcept
	{
		return shoot_timer <= 0.0f;
	}

	void ufo::make_it_shoot() noexcept
	{
		shoot_timer = max_shoot_timer;
	}

	void ufo::update(float delta_time)
	{
		entity::update();
		if(shoot_timer > 0.0f)
		{
			shoot_timer -= delta_time;
			if(shoot_timer < 0.0f)
			{
				shoot_timer = 0.0f;
			}
		}
		position.x += direction.x * move_speed * delta_time;
		position.y += direction.y * move_speed * delta_time;
	}
}