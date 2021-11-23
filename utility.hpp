#ifndef ASTEROIDS_UTILITY_HPP
#define ASTEROIDS_UTILITY_HPP

#include <SDL_rect.h>

namespace asteroids
{
	inline constexpr float CONSTANT_PI = 3.141592653589793f;

	bool equal(float a,float b);
	float magnitude(const SDL_FPoint& v);
	SDL_FPoint normalize(const SDL_FPoint& v);
	SDL_FPoint perpendicular(const SDL_FPoint& v);
	float dot_product(const SDL_FPoint& a,const SDL_FPoint& b);
	float distance(const SDL_FPoint& a,const SDL_FPoint& b);
	bool intersect_rects(const SDL_FRect& a,const SDL_FRect& b);
}

#endif