#include "utility.hpp"

#include <cmath>

namespace asteroids
{
	bool equal(float a,float b)
	{
		return std::abs(a - b) <= std::numeric_limits<float>::epsilon();
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

	bool intersect_rects(const SDL_FRect& a,const SDL_FRect& b)
	{
		return ((a.x + a.w) >= b.x) && (a.x <= (b.x + b.w)) && ((a.y + a.h) >= b.y) && (a.y <= (b.y + b.h));
	}
}