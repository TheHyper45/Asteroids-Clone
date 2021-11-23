#include "scene.hpp"

#include <chrono>
#include <iostream>

namespace asteroids
{
	scene::scene() : player({512,384},0,400,7,PLAYER_MESH),max_rock_spawn_timer(1.25f),max_ufo_spawn_timer(15.0f)
	{
		ufo_spawn_timer = max_ufo_spawn_timer;
		player.max_invulnerability_timer = 3.0f;
		player.max_respawn_timer = 3.0f;
		player.max_shoot_timer = 0.2f;
		player.make_invulnerable();
		random_engine = std::mt19937_64(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	}

	void scene::update(float delta_time,const std::array<bool,SDL_NUM_SCANCODES>& keyboard_keys,const std::array<bool,SDL_NUM_SCANCODES>& once_keyboard_keys)
	{
		if(!player.is_dead())
		{
			SDL_FPoint player_forward = player.get_forward();
			if(keyboard_keys[SDL_SCANCODE_UP])
			{
				player.velocity.x += player_forward.x * player.move_speed * delta_time;
				player.velocity.y += player_forward.y * player.move_speed * delta_time;
			}
			if(keyboard_keys[SDL_SCANCODE_DOWN])
			{
				player.velocity.x -= player_forward.x * player.move_speed * delta_time;
				player.velocity.y -= player_forward.y * player.move_speed * delta_time;
			}
			if(keyboard_keys[SDL_SCANCODE_LEFT])
			{
				player.rotation -= player.rotation_speed * delta_time;
			}
			if(keyboard_keys[SDL_SCANCODE_RIGHT])
			{
				player.rotation += player.rotation_speed * delta_time;
			}
			if(once_keyboard_keys[SDL_SCANCODE_X] && player.can_shoot())
			{
				projectiles.push_back(projectile{player.position,player.rotation,600,true,true,BULLET_MESH});
				player.make_it_shoot();
			}
			player.position.x += player.velocity.x * delta_time;
			player.position.y += player.velocity.y * delta_time;

			SDL_FRect player_bounding_box = player.get_mesh().get_transformed_bounding_box();
			if((player_bounding_box.x + player_bounding_box.w) <= 0 && player.velocity.x < 0)
			{
				player.position.x = 1024 + player_bounding_box.w;
			}
			if(player_bounding_box.x >= 1024 && player.velocity.x > 0)
			{
				player.position.x = -player_bounding_box.w;
			}
			if((player_bounding_box.y + player_bounding_box.h) <= 0 && player.velocity.y < 0)
			{
				player.position.y = 768 + player_bounding_box.h;
			}
			if(player_bounding_box.y >= 768 && player.velocity.y > 0)
			{
				player.position.y = -player_bounding_box.h;
			}
		}
		player.update(delta_time);

		rock_spawn_timer -= delta_time;
		if(rock_spawn_timer < 0.0f)
		{
			auto spawn_points = ROCK_SPAWN_POINTS;
			std::sort(spawn_points.begin(),spawn_points.end(),[&](const SDL_FPoint& a,const SDL_FPoint& b){
				return distance(player.position,a) > distance(player.position,b);
			});

			std::uniform_int_distribution<std::size_t> random_spawn_range{0,std::min(2ULL,ROCK_SPAWN_POINTS.size() - 1)};
			SDL_FPoint spawn_point = spawn_points[random_spawn_range(random_engine)];
			float angle_to_player = std::atan2(player.position.y - spawn_point.y,player.position.x - spawn_point.x);

			std::uniform_int_distribution<std::size_t> rock_mesh_random_range{0,ROCK_TEMPLATES.size() - 1};
			const rock_template& rock_template = ROCK_TEMPLATES[rock_mesh_random_range(random_engine)];
			rocks.push_back(rock{spawn_point,angle_to_player,rock_template.speed,rock_template.aword_points,rock_template.spawns_smaller_rocks_on_desstruction,rock_template.mesh});
			rock_spawn_timer = max_rock_spawn_timer;
		}

		ufo_spawn_timer -= delta_time;
		if(ufo_spawn_timer < 0.0f)
		{
			SDL_FPoint spawn_point = ((player.position.y > 384) ? SDL_FPoint{1024,192} : SDL_FPoint{0,576});
			SDL_FPoint direction = ((player.position.y > 384) ? SDL_FPoint{-1,0} : SDL_FPoint{1,0});
			ufos.push_back(ufo{spawn_point,100,2000,3.0f,direction,UFO_MESH});
			ufo_spawn_timer = max_ufo_spawn_timer;
		}

		for(auto& rock : rocks)
		{
			SDL_FPoint rock_forward_copy = rock.get_forward();
			SDL_FRect rock_bounding_box = rock.get_mesh().get_transformed_bounding_box();

			if(	((rock_bounding_box.x + rock_bounding_box.w) <= 0) || 
				(rock_bounding_box.x >= 1024) ||
				((rock_bounding_box.y + rock_bounding_box.h) <= 0) ||
				(rock_bounding_box.y >= 768) )
			{
				rock.destroyed = true;
			}
			else
			{
				rock.update(delta_time);
				if(!player.is_dead() && !player.is_invulnerable())
				{
					if(rock.get_mesh().check_collision_with(player.get_mesh()))
					{
						player.kill();
						spawn_destruction_particles(player.position,6);
					}
				}
			}
		}

		for(auto& ufo : ufos)
		{
			SDL_FPoint ufo_forward_copy = ufo.direction;
			SDL_FRect ufo_bounding_box = ufo.get_mesh().get_transformed_bounding_box();

			if(	((ufo_bounding_box.x + ufo_bounding_box.w) <= 0 && ufo_forward_copy.x < 0) || 
				(ufo_bounding_box.x >= 1024 && ufo_forward_copy.x > 0) ||
				((ufo_bounding_box.y + ufo_bounding_box.h) <= 0 && ufo_forward_copy.y < 0) ||
				(ufo_bounding_box.y >= 768 && ufo_forward_copy.y > 0) )
			{
				ufo.destroyed = true;
			}
			else
			{
				ufo.update(delta_time);
				if(!player.is_dead() && !player.is_invulnerable())
				{
					if(ufo.get_mesh().check_collision_with(player.get_mesh()))
					{
						player.kill();
						spawn_destruction_particles(player.position,6);
					}
				}
				if(!player.is_dead() && ufo.can_shoot())
				{
					float angle_to_player = std::atan2(player.position.y - ufo.position.y,player.position.x - ufo.position.x);
					projectiles.push_back(projectile{ufo.position,angle_to_player,400,true,false,BULLET_MESH});
					ufo.make_it_shoot();
				}
			}
		}

		std::vector<SDL_FPoint> additional_rock_spawn_positions{};
		for(auto& projectile : projectiles)
		{
			SDL_FRect fragment_bounding_box = projectile.get_mesh().get_transformed_bounding_box();
			if(	((fragment_bounding_box.x + fragment_bounding_box.w) <= 0) || 
				(fragment_bounding_box.x >= 1024) ||
				((fragment_bounding_box.y + fragment_bounding_box.h) <= 0) ||
				(fragment_bounding_box.y >= 768) )
			{
				projectile.destroyed = true;
			}
			else
			{
				projectile.update(delta_time);
				if(projectile.physical)
				{
					if(projectile.player_friendly)
					{
						for(auto& rock : rocks)
						{
							if(rock.get_mesh().check_collision_with(projectile.get_mesh()))
							{
								projectile.destroyed = true;
								rock.destroyed = true;
								player.points += rock.award_points;
								if(rock.spawns_smaller_rocks_on_destruction)
								{
									additional_rock_spawn_positions.push_back(rock.position);
									spawn_destruction_particles(rock.position,4);
								}
								else
								{
									spawn_destruction_particles(rock.position,3);
								}
							}
						}
						for(auto& ufo : ufos)
						{
							if(ufo.get_mesh().check_collision_with(projectile.get_mesh()))
							{
								projectile.destroyed = true;
								ufo.destroyed = true;
								player.points += ufo.award_points;
								spawn_destruction_particles(ufo.position,3);
							}
						}
					}
					else if(!player.is_dead() && !player.is_invulnerable())
					{
						if(projectile.get_mesh().check_collision_with(player.get_mesh()))
						{
							player.kill();
							spawn_destruction_particles(player.position,6);
							projectile.destroyed = true;
						}
					}
				}
			}
		}

		rocks.erase(std::remove_if(rocks.begin(),rocks.end(),[](rock rock){
			return rock.destroyed;
		}),rocks.end());

		projectiles.erase(std::remove_if(projectiles.begin(),projectiles.end(),[](projectile projectile){
			return projectile.destroyed;
		}),projectiles.end());

		ufos.erase(std::remove_if(ufos.begin(),ufos.end(),[](ufo ufo){
			return ufo.destroyed;
		}),ufos.end());

		for(const auto& additional_rock_spawn_position : additional_rock_spawn_positions)
		{
			std::uniform_int_distribution<std::size_t> rock_mesh_random_range{0,SMALL_ROCK_TEMPLATES.size() - 1};
			constexpr std::size_t frag_count = 4;
			for(std::size_t i = 0;i < frag_count;++i)
			{
				const rock_template& rock_template = SMALL_ROCK_TEMPLATES[rock_mesh_random_range(random_engine)];
				rocks.push_back(rock{additional_rock_spawn_position,CONSTANT_PI * 2.0f * (1.0f / frag_count) * i,rock_template.speed,rock_template.aword_points,false,rock_template.mesh});
			}
		}
		additional_rock_spawn_positions.clear();
	}

	const player& scene::get_player() const
	{
		return player;
	}

	const std::vector<rock>& scene::get_rocks() const
	{
		return rocks;
	}

	const std::vector<projectile>& scene::get_projectiles() const
	{
		return projectiles;
	}

	const std::vector<ufo>& scene::get_ufos() const
	{
		return ufos;
	}

	void scene::spawn_destruction_particles(SDL_FPoint position,std::size_t count)
	{
		for(std::size_t i = 0;i < count;++i)
		{
			projectiles.push_back(projectile{position,CONSTANT_PI * 2.0f * (1.0f / count) * i,200,false,true,DESTRUCTION_FRAGMENT_MESH});
		}
	}
}