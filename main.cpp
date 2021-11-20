#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>

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

	SDL_Event event{};
	bool is_running = true;
	while(is_running)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					is_running = false;
				break;
			}
		}

		SDL_SetRenderDrawColor(renderer,0,128,255,255);
		SDL_RenderClear(renderer);

		SDL_Rect rect{};
		rect.x = 50;
		rect.y = 50;
		rect.w = 50;
		rect.h = 50;

		SDL_SetRenderDrawColor(renderer,255,255,255,255);
		SDL_RenderFillRect(renderer,&rect);

		SDL_RenderPresent(renderer);
	}
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}