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
#include <SDL_ttf.h>

constexpr float CONSTANT_PI = 3.141592653589793f;

bool equal(float a,float b)
{
	return std::abs(a - b) <= std::numeric_limits<float>::epsilon();
}

bool operator == (const SDL_FPoint& a,const SDL_FPoint& b)
{
	return equal(a.x,b.x) && equal(a.y,b.y);
}

bool intersect_rects(const SDL_FRect& a,const SDL_FRect& b)
{
	return ((a.x + a.w) >= b.x) && (a.x <= (b.x + b.w)) && ((a.y + a.h) >= b.y) && (a.y <= (b.y + b.h));
}

float magnitude(const SDL_FPoint& v)
{
	return std::hypot(v.x,v.y);
}

SDL_FPoint normalize(const SDL_FPoint& v)
{
	float n = magnitude(v);
	return {v.x / n,v.y / n};
}

SDL_FPoint perpendicular(const SDL_FPoint& v)
{
	return {-v.y,v.x};
}

float dot_product(const SDL_FPoint& a,const SDL_FPoint& b)
{
	return a.x * b.x + a.y * b.y;
}

float distance(const SDL_FPoint& a,const SDL_FPoint& b)
{
	return std::hypot(b.x - a.x,b.y - a.y);
}

class mesh
{
public:
	SDL_FPoint position;
	float rotation;

	mesh(const std::vector<SDL_FPoint>& _vertices)
		: vertices(_vertices),position(),rotation(),transformed_vertices(),edge_normals(),
		bounding_box(),transformed_bounding_box(),cache_position(),cache_rotation()
	{}
	mesh(const mesh& other)
		: position(other.position),rotation(other.rotation),vertices(other.vertices),transformed_vertices(other.transformed_vertices),
		edge_normals(other.edge_normals),bounding_box(other.bounding_box),transformed_bounding_box(other.bounding_box),
		cache_position(other.cache_position),cache_rotation(other.cache_rotation)
	{}
	mesh(mesh&& other) noexcept
		 : position(other.position),rotation(other.rotation),vertices(std::move(other.vertices)),transformed_vertices(std::move(other.transformed_vertices)),
		 edge_normals(std::move(other.edge_normals)),bounding_box(other.bounding_box),transformed_bounding_box(other.bounding_box),
		 cache_position(other.cache_position),cache_rotation(other.cache_rotation)
	{
		other.position = {};
		other.rotation = 0;
		other.bounding_box = {};
		other.transformed_bounding_box = {};
		other.cache_position = {};
		other.cache_rotation = 0;
	}
	mesh& operator = (const mesh& other)
	{
		if(&other != this)
		{
			position = other.position;
			rotation = other.rotation;
			vertices = other.vertices;
			transformed_vertices = other.transformed_vertices;
			bounding_box = other.bounding_box;
			transformed_bounding_box = other.transformed_bounding_box;
			edge_normals = other.edge_normals;
			cache_position = other.cache_position;
			cache_rotation = other.cache_rotation;
		}
		return *this;
	}
	mesh& operator = (mesh&& other) noexcept
	{
		if(&other != this)
		{
			position = other.position;
			rotation = other.rotation;
			vertices = std::move(other.vertices);
			transformed_vertices = std::move(other.transformed_vertices);
			bounding_box = other.bounding_box;
			transformed_bounding_box = other.transformed_bounding_box;
			edge_normals = std::move(other.edge_normals);
			cache_position = other.cache_position;
			cache_rotation = other.cache_rotation;
		}
		return *this;
	}

	bool check_collision_with(const mesh& other) const
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
		if(!for_each_normal(edge_normals,transformed_vertices,other.transformed_vertices))
		{
			return false;
		}
		return for_each_normal(other.edge_normals,transformed_vertices,other.transformed_vertices);
	}

	void update()
	{
		if(position != cache_position || !equal(rotation,cache_rotation))
		{
			transformed_vertices.clear();
			bounding_box = {};
			transformed_bounding_box = {};
			edge_normals.clear();
			SDL_FPoint bbmin{
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity()
			};
			SDL_FPoint bbmax{
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity()
			};
			SDL_FPoint bbmin_transformed{
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity()
			};
			SDL_FPoint bbmax_transformed{
				std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity()
			};
			for(const auto& vertex : vertices)
			{
				SDL_FPoint new_point{};
				new_point.x = std::cos(rotation) * vertex.x - std::sin(rotation) * vertex.y + position.x;
				new_point.y = std::sin(rotation) * vertex.x + std::cos(rotation) * vertex.y + position.y;

				if(std::isinf(bbmin_transformed.x) || bbmin_transformed.x > new_point.x)
				{
					bbmin_transformed.x = new_point.x;
				}
				if(std::isinf(bbmin_transformed.y) || bbmin_transformed.y > new_point.y)
				{
					bbmin_transformed.y = new_point.y;
				}
				if(std::isinf(bbmax_transformed.x) || bbmax_transformed.x < new_point.x)
				{
					bbmax_transformed.x = new_point.x;
				}
				if(std::isinf(bbmax_transformed.y) || bbmax_transformed.y < new_point.y)
				{
					bbmax_transformed.y = new_point.y;
				}
				transformed_vertices.push_back(new_point);

				if(std::isinf(bbmin.x) || bbmin.x > vertex.x)
				{
					bbmin.x = vertex.x;
				}
				if(std::isinf(bbmin.y) || bbmin.y > vertex.y)
				{
					bbmin.y = vertex.y;
				}
				if(std::isinf(bbmax.x) || bbmax.x < vertex.x)
				{
					bbmax.x = vertex.x;
				}
				if(std::isinf(bbmax.y) || bbmax.y < vertex.y)
				{
					bbmax.y = vertex.y;
				}
			}
			bounding_box.x = bbmin.x;
			bounding_box.y = bbmin.y;
			bounding_box.w = bbmax.x - bbmin.x;
			bounding_box.h = bbmax.y - bbmin.y;
			transformed_bounding_box.x = bbmin_transformed.x;
			transformed_bounding_box.y = bbmin_transformed.y;
			transformed_bounding_box.w = bbmax_transformed.x - bbmin_transformed.x;
			transformed_bounding_box.h = bbmax_transformed.y - bbmin_transformed.y;

			std::size_t length = transformed_vertices.size();
			for(std::size_t i = 0;i < length;++i)
			{
				SDL_FPoint current = transformed_vertices[i];
				SDL_FPoint next = transformed_vertices[(i + 1) % length];
				SDL_FPoint diff{
					next.x - current.x,
					next.y - current.y
				};
				edge_normals.push_back(perpendicular(normalize(diff)));
			}
			cache_position = position;
			cache_rotation = rotation;
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

	const SDL_FRect& get_transformed_bounding_box() const
	{
		return transformed_bounding_box;
	}
private:
	SDL_FPoint cache_position{};
	float cache_rotation{};
	std::vector<SDL_FPoint> vertices;
	std::vector<SDL_FPoint> transformed_vertices;
	SDL_FRect bounding_box{};
	SDL_FRect transformed_bounding_box{};
	std::vector<SDL_FPoint> edge_normals{};
};

class entity
{
public:
	SDL_FPoint position;
	float rotation;
	float move_speed;
	float rotation_speed;
	bool destroyed;
	std::uintmax_t rock_index;
	std::uintmax_t rock_pass_count;

	entity(SDL_FPoint _position,float _rotation,float _move_speed,float _rotation_speed,const mesh& _mesh)
		: position(_position),rotation(_rotation),move_speed(_move_speed),rotation_speed(_rotation_speed),mesh(_mesh),destroyed(false),rock_index(),rock_pass_count()
	{
		update();
	}
	entity(const entity& other)
		: position(other.position),rotation(other.rotation),move_speed(other.move_speed),rotation_speed(other.rotation_speed),
		mesh(other.mesh),destroyed(other.destroyed),rock_index(other.rock_index),rock_pass_count(other.rock_pass_count)
	{
		update();
	}
	entity(entity&& other) noexcept
		: position(other.position),rotation(other.rotation),move_speed(other.move_speed),rotation_speed(other.rotation_speed),
		mesh(std::move(other.mesh)),destroyed(other.destroyed),rock_index(other.rock_index),rock_pass_count(other.rock_pass_count)
	{
		update();
		other.position = {};
		other.rotation = other.move_speed = other.rotation_speed = 0;
		other.destroyed = {};
	}
	entity& operator = (const entity& other)
	{
		if(&other != this)
		{
			position = other.position;
			rotation = other.rotation;
			move_speed = other.move_speed;
			rotation_speed = other.rotation_speed;
			destroyed = other.destroyed;
			mesh = other.mesh;
			rock_index = other.rock_index;
			rock_pass_count = other.rock_pass_count;
			update();
		}
		return *this;
	}
	entity& operator = (entity&& other) noexcept
	{
		if(&other != this)
		{
			position = other.position;
			rotation = other.rotation;
			move_speed = other.move_speed;
			rotation_speed = other.rotation_speed;
			destroyed = other.destroyed;
			mesh = std::move(other.mesh);
			rock_index = other.rock_index;
			rock_pass_count = other.rock_pass_count;
			update();
		}
		return *this;
	}

	void update()
	{
		mesh.position = position;
		mesh.rotation = rotation;
		mesh.update();
	}

	SDL_FPoint get_shoot_point()
	{
		const SDL_FRect& rect = mesh.get_bounding_box();
		SDL_FPoint point{
			rect.x + rect.w,
			rect.y + rect.h / 2.0f
		};
		return {
			std::cos(rotation) * point.x - std::sin(rotation) * point.y + position.x,
			std::sin(rotation) * point.x + std::cos(rotation) * point.y + position.y
		};
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
	if(vertices.size() > 0)
	{
		SDL_RenderDrawLinesF(renderer,vertices.data(),static_cast<int>(vertices.size()));
		SDL_RenderDrawLineF(renderer,vertices.back().x,vertices.back().y,vertices.front().x,vertices.front().y);
	}
}

bool render_text(SDL_Renderer* renderer,TTF_Font* font,std::string_view text,SDL_Color color,SDL_Texture** out_texture,int* out_width,int* out_height)
{
	SDL_Surface* surface = TTF_RenderText_Solid(font,text.data(),color);
	if(!surface)
	{
		return false;
	}
	*out_width = surface->w;
	*out_height = surface->h;
	*out_texture = SDL_CreateTextureFromSurface(renderer,surface);
	SDL_FreeSurface(surface);
	return !!*out_texture;
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
	if(TTF_Init() != 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't init SDL2_ttf library.",nullptr);
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Asteroids Clone",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1024,768,SDL_WINDOW_SHOWN);
	if(!window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't create a window.",nullptr);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return 1;
	}
	SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
	if(!renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't create a renderer.",nullptr);
		SDL_DestroyWindow(window);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	int window_width{};
	int window_height{};
	if(SDL_GetRendererOutputSize(renderer,&window_width,&window_height) < 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't get window size.",nullptr);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	TTF_Font* font = TTF_OpenFont("Assets/cascadia_mono.ttf",30);
	if(!font)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't open a font.",nullptr);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	std::mt19937_64 random{static_cast<std::uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count())};

	entity player{{window_width / 2.0f,window_height / 2.0f},0,300,3,{std::vector<SDL_FPoint>{{-30,-30},{30,0},{-30,30}}}};

	std::vector<mesh> big_rock_meshes{
		{std::vector<SDL_FPoint>{
			{-20,-55},
			{50,-50},
			{70,40},
			{30,55},
			{-55,50},
		}},
		{std::vector<SDL_FPoint>{
			{-60,-60},
			{40,-50},
			{50,50},
			{-50,60}
		}}
	};

	std::vector<mesh> small_rock_meshes{
		{std::vector<SDL_FPoint>{
			{-10,-20},
			{0,-10},
			{10,20},
			{-10,10}
		}}
	};

	std::uniform_int_distribution<std::size_t> big_rock_random_range{0,big_rock_meshes.size() - 1};
	std::uniform_int_distribution<std::size_t> small_rock_random_range{0,small_rock_meshes.size() - 1};

	std::uniform_real_distribution<float> angle_random_range{0,CONSTANT_PI * 2.0f};

	std::vector<entity> rocks{};
	std::vector<entity> bullets{};

	std::vector<SDL_FPoint> rock_spawn_points{
		{-25,-25},
		{512,-25},
		{1049,-25},
		{1049,384},
		{1049,793},
		{512,793},
		{-25,793},
		{-25,384}
	};

	mesh bullet_mesh{std::vector<SDL_FPoint>{
		{-2.5f,-2.5f},
		{+2.5f,-2.5f},
		{+2.5f,+2.5f},
		{-2.5f,+2.5f}
	}};

	SDL_Texture* score_text{};
	int score_text_width{};
	int score_text_height{};
	if(!render_text(renderer,font,"SCORE",SDL_Color{255,255,255,255},&score_text,&score_text_width,&score_text_height))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't render text.",nullptr);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_CloseFont(font);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	SDL_Texture* lifes_text{};
	int lifes_text_width{};
	int lifes_text_height{};
	if(!render_text(renderer,font,"LIFES",SDL_Color{255,255,255,255},&lifes_text,&lifes_text_width,&lifes_text_height))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't render text.",nullptr);
		SDL_DestroyTexture(score_text);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_CloseFont(font);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	SDL_Texture* score_number_text{};
	int score_number_text_width{};
	int score_number_text_height{};
	if(!render_text(renderer,font,"0",SDL_Color{255,255,255,255},&score_number_text,&score_number_text_width,&score_number_text_height))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't render text.",nullptr);
		SDL_DestroyTexture(lifes_text);
		SDL_DestroyTexture(score_text);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_CloseFont(font);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	std::uintmax_t lifes = 3;
	std::uintmax_t lifes_cache = 3;

	SDL_Texture* lifes_number_text{};
	int lifes_number_text_width{};
	int lifes_number_text_height{};
	if(!render_text(renderer,font,std::to_string(lifes).c_str(),SDL_Color{255,255,255,255},&lifes_number_text,&lifes_number_text_width,&lifes_number_text_height))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't render text.",nullptr);
		SDL_DestroyTexture(score_number_text);
		SDL_DestroyTexture(lifes_text);
		SDL_DestroyTexture(score_text);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_CloseFont(font);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	std::uintmax_t score = 0;
	std::uintmax_t score_cache = 0;
	float invulnerability_timer_max = 3;
	float invulnerability_timer = invulnerability_timer_max;
	float respawn_timer = 0;
	float respawn_timer_max = 3;
	float shoot_timer = 0;
	float shoot_max_timer = 0.2f;
	bool player_dead = false;
	float rock_spawn_timer = 0;
	float rock_spawn_timer_max = 2;
	std::uintmax_t big_rock_count = 0;
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
		if(shoot_timer <= 0.0f)
		{
			if(keyboard_keys_once[SDL_SCANCODE_X] && !player_dead)
			{
				SDL_FPoint point = player.get_shoot_point();
				bullets.push_back(entity{point,player.rotation,500,0,bullet_mesh});
				shoot_timer = shoot_max_timer;
			}
		}
		else
		{
			shoot_timer -= delta_time;
			if(shoot_timer < 0.0f)
			{
				shoot_timer = 0;
			}
		}

		const SDL_FRect& player_bounding_box = player.get_mesh().get_transformed_bounding_box();
		SDL_FPoint player_forward = player.get_forward();

		SDL_FPoint collision_player_forward = player_forward;
		if(keyboard_keys[SDL_SCANCODE_UP])
		{
			player.position.x += player_forward.x * player.move_speed * delta_time;
			player.position.y += player_forward.y * player.move_speed * delta_time;
		}
		if(keyboard_keys[SDL_SCANCODE_DOWN])
		{
			player.position.x -= player_forward.x * player.move_speed * delta_time;
			player.position.y -= player_forward.y * player.move_speed * delta_time;
			collision_player_forward.x *= -1;
			collision_player_forward.y *= -1;
		}
		if(keyboard_keys[SDL_SCANCODE_LEFT])
		{
			player.rotation -= player.rotation_speed * delta_time;
		}
		if(keyboard_keys[SDL_SCANCODE_RIGHT])
		{
			player.rotation += player.rotation_speed * delta_time;
		}

		if((player_bounding_box.x + player_bounding_box.w) <= 0 && collision_player_forward.x < 0)
		{
			player.position.x = window_width + player_bounding_box.w;
		}
		if(player_bounding_box.x >= window_width && collision_player_forward.x > 0)
		{
			player.position.x = -player_bounding_box.w;
		}
		if((player_bounding_box.y + player_bounding_box.h) <= 0 && collision_player_forward.y < 0)
		{
			player.position.y = window_height + player_bounding_box.h;
		}
		if(player_bounding_box.y >= window_height && collision_player_forward.y > 0)
		{
			player.position.y = -player_bounding_box.h;
		}

		invulnerability_timer -= delta_time;
		if(invulnerability_timer < 0)
		{
			invulnerability_timer = 0;
		}
		if(player_dead)
		{
			respawn_timer -= delta_time;
			if(respawn_timer < 0)
			{
				respawn_timer = 0;
				player_dead = false;
				player.position = {window_width / 2.0f,window_height / 2.0f};
				player.rotation = 0;
				invulnerability_timer = invulnerability_timer_max;
			}
		}

		rock_spawn_timer -= delta_time;
		if(rock_spawn_timer <= 0)
		{
			if(big_rock_count < 3)
			{
				std::sort(rock_spawn_points.begin(),rock_spawn_points.end(),[&](const SDL_FPoint& a,const SDL_FPoint& b){
					return distance(player.position,a) < distance(player.position,b);
				});

				SDL_FPoint furthest = rock_spawn_points.back();
				float angle = std::atan2(player.position.y - furthest.y,player.position.x - furthest.x);
				entity big_rock{furthest,angle,400,0,big_rock_meshes[big_rock_random_range(random)]};
				big_rock.rock_index = 3;
				rocks.push_back(big_rock);
				++big_rock_count;
			}
			rock_spawn_timer = rock_spawn_timer_max;
		}

		player.update();
		const mesh& player_mesh = player.get_mesh();
		for(auto& rock : rocks)
		{
			rock.position.x += rock.get_forward().x * rock.move_speed * delta_time;
			rock.position.y += rock.get_forward().y * rock.move_speed * delta_time;

			SDL_FPoint rock_forward = rock.get_forward();
			const SDL_FRect& rock_bounding_box = rock.get_mesh().get_transformed_bounding_box();

			if(rock.rock_index == 3 || rock.rock_pass_count > 0)
			{
				bool changed_pos =  false;
				if((rock_bounding_box.x + rock_bounding_box.w) <= 0 && rock_forward.x < 0)
				{
					rock.position.x = window_width + rock_bounding_box.w;
					changed_pos = true;
				}
				if(rock_bounding_box.x >= window_width && rock_forward.x > 0)
				{
					rock.position.x = -rock_bounding_box.w;
					changed_pos = true;
				}
				if((rock_bounding_box.y + rock_bounding_box.h) <= 0 && rock_forward.y < 0)
				{
					rock.position.y = window_height + rock_bounding_box.h;
					changed_pos = true;
				}
				if(rock_bounding_box.y >= window_height && rock_forward.y > 0)
				{
					rock.position.y = -rock_bounding_box.h;
					changed_pos = true;
				}

				if(changed_pos && rock.rock_pass_count > 0)
				{
					--rock.rock_pass_count;
				}
			}
			else if(((rock_bounding_box.x + rock_bounding_box.w) <= 0 && rock_forward.x < 0) ||
				   (rock_bounding_box.x >= window_width && rock_forward.x > 0) ||
				   ((rock_bounding_box.y + rock_bounding_box.h) <= 0 && rock_forward.y < 0) ||
				   (rock_bounding_box.y >= window_height && rock_forward.y > 0))
			{
				rock.destroyed = true;
			}

			rock.update();
			if(!player_dead && invulnerability_timer <= 0)
			{
				const mesh& rock_mesh = rock.get_mesh();
				if(rock_mesh.check_collision_with(player_mesh))
				{
					respawn_timer = respawn_timer_max;
					player_dead = true;
					--lifes;
					if(lifes == 0)
					{
						respawn_timer = 36000;
						//TODO: Player lost.
					}
				}
			}
		}
		std::vector<SDL_FPoint> small_rock_spawn_positions{};
		for(auto& bullet : bullets)
		{
			bullet.position.x += bullet.get_forward().x * bullet.move_speed * delta_time;
			bullet.position.y += bullet.get_forward().y * bullet.move_speed * delta_time;
			bullet.update();
			const mesh& bullet_mesh = bullet.get_mesh();
			const SDL_FRect bullet_bounding_box = bullet_mesh.get_transformed_bounding_box();

			if(((bullet_bounding_box.x + bullet_bounding_box.w) <= 0) ||
			   (bullet_bounding_box.x >= window_width) ||
			   ((bullet_bounding_box.y + bullet_bounding_box.h) <= 0) ||
			   (bullet_bounding_box.y >= window_height))
			{
				bullet.destroyed = true;
			}
			else
			{
				for(auto& rock : rocks)
				{
					const mesh& rock_mesh = rock.get_mesh();
					if(rock_mesh.check_collision_with(bullet_mesh))
					{
						score += ((rock.rock_index == 3) ? 100 : 150);
						if(rock.rock_index == 3)
						{
							--big_rock_count;
							small_rock_spawn_positions.push_back(rock.position);
						}
						bullet.destroyed = true;
						rock.destroyed = true;
					}
				}
			}
		}

		for(const auto& pos : small_rock_spawn_positions)
		{
			for(std::size_t i = 0;i < 4;++i)
			{
				float angle = angle_random_range(random);
				entity small_rock{pos,angle,425,0,small_rock_meshes[small_rock_random_range(random)]};
				small_rock.rock_index = 2;
				small_rock.rock_pass_count = 1;
				rocks.push_back(small_rock);
			}
		}
		small_rock_spawn_positions.clear();

		rocks.erase(std::remove_if(rocks.begin(),rocks.end(),[](entity rock){
			return rock.destroyed;
		}),rocks.end());

		bullets.erase(std::remove_if(bullets.begin(),bullets.end(),[](entity bullet){
			return bullet.destroyed;
		}),bullets.end());

		SDL_SetRenderDrawColor(renderer,0,0,0,255);
		SDL_RenderClear(renderer);

		if(invulnerability_timer > 0)
		{
			SDL_SetRenderDrawColor(renderer,0,128,255,255);
		}
		else
		{
			SDL_SetRenderDrawColor(renderer,255,255,255,255);
		}
		
		if(!player_dead)
		{
			render_mesh(renderer,player.get_mesh());
		}

		SDL_SetRenderDrawColor(renderer,255,255,255,255);
		for(const auto& rock : rocks)
		{
			render_mesh(renderer,rock.get_mesh());
		}
		for(const auto& bullet : bullets)
		{
			render_mesh(renderer,bullet.get_mesh());
		}

		SDL_Rect score_text_rect{
			.x = window_width / 2 - score_text_width / 2,
			.y = 0,
			.w = score_text_width,
			.h = score_text_height
		};
		SDL_RenderCopy(renderer,score_text,nullptr,&score_text_rect);

		if(score_cache != score)
		{
			SDL_DestroyTexture(score_number_text);
			if(!render_text(renderer,font,std::to_string(score).c_str(),SDL_Color{255,255,255,255},&score_number_text,&score_number_text_width,&score_number_text_height))
			{
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't render text.",nullptr);
				SDL_DestroyTexture(lifes_number_text);
				SDL_DestroyTexture(lifes_text);
				SDL_DestroyTexture(score_text);
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
				TTF_CloseFont(font);
				TTF_Quit();
				IMG_Quit();
				SDL_Quit();
				return 1;
			}
			score_cache = score;
		}
		if(lifes_cache != lifes)
		{
			SDL_DestroyTexture(lifes_number_text);
			if(!render_text(renderer,font,std::to_string(lifes).c_str(),SDL_Color{255,255,255,255},&lifes_number_text,&lifes_number_text_width,&lifes_number_text_height))
			{
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error!","Couldn't render text.",nullptr);
				SDL_DestroyTexture(score_number_text);
				SDL_DestroyTexture(lifes_text);
				SDL_DestroyTexture(score_text);
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
				TTF_CloseFont(font);
				TTF_Quit();
				IMG_Quit();
				SDL_Quit();
				return 1;
			}
			lifes_cache = lifes;
		}

		SDL_Rect score_number_text_rect{
			.x = window_width / 2 - score_number_text_width / 2,
			.y = score_text_height,
			.w = score_number_text_width,
			.h = score_number_text_height
		};
		SDL_RenderCopy(renderer,score_number_text,nullptr,&score_number_text_rect);

		SDL_Rect lifes_number_text_rect{
			.x = 0,
			.y = lifes_text_height,
			.w = lifes_number_text_width,
			.h = lifes_number_text_height
		};
		SDL_RenderCopy(renderer,lifes_number_text,nullptr,&lifes_number_text_rect);

		SDL_Rect lifes_text_rect{
			.x = 0,
			.y = 0,
			.w = lifes_text_width,
			.h = lifes_text_height
		};
		SDL_RenderCopy(renderer,lifes_text,nullptr,&lifes_text_rect);

		SDL_RenderPresent(renderer);
	}
	
	SDL_DestroyTexture(lifes_number_text);
	SDL_DestroyTexture(score_number_text);
	SDL_DestroyTexture(lifes_text);
	SDL_DestroyTexture(score_text);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(font);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}