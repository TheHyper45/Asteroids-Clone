#include <array>
#include <cmath>
#include <vector>
#include <limits>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>

bool equal(float a,float b)
{
	return std::abs(a - b) <= std::numeric_limits<float>::epsilon();
}

bool operator == (const SDL_FPoint& a,const SDL_FPoint& b)
{
	return equal(a.x,b.x) && equal(a.y,b.y);
}

class mesh
{
public:
	SDL_FPoint position{};
	float rotation{};

	mesh(const std::vector<SDL_FPoint>& _vertices) : vertices(_vertices) {}
	mesh(const mesh& other) : position(other.position),rotation(other.rotation),vertices(other.vertices),transformed_vertices(other.transformed_vertices) {}
	mesh(mesh&& other) noexcept
		 : position(other.position),rotation(other.rotation),vertices(std::move(other.vertices)),transformed_vertices(std::move(other.transformed_vertices))
	{
		other.position = {};
		other.rotation = 0;
		other.vertices.clear();
		other.transformed_vertices.clear();
		other.bounding_box = {};
	}
	mesh& operator = (const mesh& other)
	{
		if(&other != this)
		{
			position = other.position;
			rotation = other.rotation;
			vertices = other.vertices;
			transformed_vertices = other.transformed_vertices;
		}
		return *this;
	}

	void update()
	{
		if(position != cache_position || !equal(rotation,cache_rotation))
		{
			transformed_vertices.clear();
			bounding_box = {};
			SDL_FPoint bbmin{
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity()
			};
			SDL_FPoint bbmax{
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity()
			};
			for(const auto& vertex : vertices)
			{
				SDL_FPoint new_point{};
				new_point.x = std::cos(rotation) * vertex.x - std::sin(rotation) * vertex.y + position.x;
				new_point.y = std::sin(rotation) * vertex.x + std::cos(rotation) * vertex.y + position.y;

				if(std::isinf(bbmin.x) || bbmin.x > new_point.x)
				{
					bbmin.x = new_point.x;
				}
				if(std::isinf(bbmin.y) ||bbmin.y > new_point.y)
				{
					bbmin.y = new_point.y;
				}
				if(std::isinf(bbmax.x) ||bbmax.x < new_point.x)
				{
					bbmax.x = new_point.x;
				}
				if(std::isinf(bbmax.y) ||bbmax.y < new_point.y)
				{
					bbmax.y = new_point.y;
				}
				transformed_vertices.push_back(new_point);
			}
			cache_position = position;
			cache_rotation = rotation;
			bounding_box.x = bbmin.x;
			bounding_box.y = bbmin.y;
			bounding_box.w = bbmax.x - bbmin.x;
			bounding_box.h = bbmax.y - bbmin.y;
		}
	}

	const std::vector<SDL_FPoint>& get_transformed_vertices() const
	{
		return transformed_vertices;
	}

	const SDL_FRect& get_bounding_box() const
	{
		return bounding_box;
	}
private:
	SDL_FPoint cache_position{};
	float cache_rotation{};
	std::vector<SDL_FPoint> vertices;
	std::vector<SDL_FPoint> transformed_vertices;
	SDL_FRect bounding_box{};
};

class entity
{
public:
	SDL_FPoint position;
	float rotation;
	float move_speed;
	float rotation_speed;

	entity(SDL_FPoint _position,float _rotation,float _move_speed,float _rotation_speed,const mesh& _mesh)
		: position(_position),rotation(_rotation),move_speed(_move_speed),rotation_speed(_rotation_speed),mesh(_mesh)
	{}
	entity(const entity&) = delete;
	entity(entity&& other) noexcept
		: position(other.position),rotation(other.rotation),move_speed(other.move_speed),rotation_speed(other.rotation_speed),mesh(std::move(other.mesh))
	{
		other.position = {};
		other.rotation = other.move_speed = other.rotation_speed;
	}
	entity& operator = (const entity&) = delete;

	void update()
	{
		mesh.position = position;
		mesh.rotation = rotation;
		mesh.update();
	}

	SDL_FPoint get_forward()
	{
		return {std::cos(rotation),std::sin(rotation)};
	}

	const mesh& get_mesh() const
	{
		return mesh;
	}
private:
	mesh mesh;
};

void render_mesh(SDL_Renderer* renderer,const mesh& mesh)
{
	const auto& vertices = mesh.get_transformed_vertices();
	SDL_RenderDrawLinesF(renderer,vertices.data(),static_cast<int>(vertices.size()));
	SDL_RenderDrawLineF(renderer,vertices.back().x,vertices.back().y,vertices.front().x,vertices.front().y);
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

	SDL_Window* window = SDL_CreateWindow("Asteroids45",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1024,768,SDL_WINDOW_SHOWN);
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

	std::vector<SDL_FPoint> rock_vertices{
		{-20,-55},
		{50,-50},
		{70,40},
		{30,55},
		{-55,50},
	};

	mesh rock_mesh{rock_vertices};
	rock_mesh.position = {256,256};
	rock_mesh.rotation = 0;

	entity player{{512,384},0,300,3,std::vector<SDL_FPoint>{{-30,-30},{30,0},{-30,30}}};
	player.update();

	std::vector<entity> rocks{};
	rocks.push_back({{800,600},0,10,2,rock_mesh});

	std::array<bool,SDL_NUM_SCANCODES> keyboard_keys{};
	Uint64 timer_start = SDL_GetPerformanceCounter();
	SDL_Event event{};
	bool is_running = true;
	while(is_running)
	{
		Uint64 timer_end = SDL_GetPerformanceCounter();
		float delta_time = static_cast<float>(timer_end - timer_start) / SDL_GetPerformanceFrequency();
		timer_start = timer_end;

		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					is_running = false;
				break;
				case SDL_KEYDOWN:
					keyboard_keys[event.key.keysym.scancode] = true;
				break;
				case SDL_KEYUP:
					keyboard_keys[event.key.keysym.scancode] = false;
				break;
			}
		}

		const SDL_FRect& player_bounding_box = player.get_mesh().get_bounding_box();
		SDL_FPoint player_forward = player.get_forward();

		SDL_FPoint collision_player_forward = player_forward;
		if(keyboard_keys[SDL_SCANCODE_W])
		{
			player.position.x += player_forward.x * player.move_speed * delta_time;
			player.position.y += player_forward.y * player.move_speed * delta_time;
		}
		if(keyboard_keys[SDL_SCANCODE_S])
		{
			player.position.x -= player_forward.x * player.move_speed * delta_time;
			player.position.y -= player_forward.y * player.move_speed * delta_time;
			collision_player_forward.x *= -1;
			collision_player_forward.y *= -1;
		}
		if(keyboard_keys[SDL_SCANCODE_A])
		{
			player.rotation -= player.rotation_speed * delta_time;
		}
		if(keyboard_keys[SDL_SCANCODE_D])
		{
			player.rotation += player.rotation_speed * delta_time;
		}

		if((player_bounding_box.x + player_bounding_box.w) <= 0 && collision_player_forward.x < 0)
		{
			player.position.x = 1024 + player_bounding_box.w;
		}
		if(player_bounding_box.x >= 1024 && collision_player_forward.x > 0)
		{
			player.position.x = -player_bounding_box.w;
		}
		if((player_bounding_box.y + player_bounding_box.h) <= 0 && collision_player_forward.y < 0)
		{
			player.position.y = 768 + player_bounding_box.h;
		}
		if(player_bounding_box.y >= 768 && collision_player_forward.y > 0)
		{
			player.position.y = -player_bounding_box.h;
		}

		player.update();
		for(auto& rock : rocks)
		{
			rock.update();
		}

		SDL_SetRenderDrawColor(renderer,0,0,0,255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer,255,255,255,255);
		render_mesh(renderer,player.get_mesh());
		SDL_SetRenderDrawColor(renderer,0,128,255,255);
		SDL_RenderDrawRectF(renderer,&player_bounding_box);

		for(const auto& rock : rocks)
		{
			SDL_SetRenderDrawColor(renderer,255,255,255,255);
			render_mesh(renderer,rock.get_mesh());
			SDL_SetRenderDrawColor(renderer,0,128,255,255);
			SDL_RenderDrawRectF(renderer,&rock.get_mesh().get_bounding_box());
		}

		SDL_RenderPresent(renderer);
	}
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}