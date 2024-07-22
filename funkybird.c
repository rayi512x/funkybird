/* Simple Flappy Bird clone written in C using SDL2
 * Author: rayi512x
 * License: WTFPL */

#include <SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static const SDL_Rect border = {0, 0, 800, 600};
static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Event* event;
static bool running;
static bool paused;
static bool pressed;
static int jumpVel;
static Uint32 jumpQ;
static SDL_Rect bird = {200, ((600 / 2) - (32 / 2)), 32, 32};
static SDL_Rect pipe[6];
static int nextpipe;
static Uint32 score;
static Uint32 best;
static char title_buf[100];
#ifndef __EMSCRIPTEN__
static Uint64 start;
static Uint64 end;
#endif

void init(void);
void quit(void);
void update(void);
void render(void);
void reset(void);
void mainloop(void);

int main(int argc, char** argv) {
	init();
	SDL_Event mainevent;
	event = &mainevent;

	#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(mainloop, 60, 1);
	#else
	while (running) mainloop();
	quit();
	#endif

	return 0;
}

void mainloop(void) {
	#ifdef __EMSCRIPTEN__
	if (!running) {
		quit();
		emscripten_cancel_main_loop();
	}
	#endif

	#ifndef __EMSCRIPTEN__
	start = SDL_GetTicks64();
	#endif

	update();
	render();

	#ifndef __EMSCRIPTEN__
	end = SDL_GetTicks64();
	if ((1000 / 60) > (end - start))
		SDL_Delay((1000 / 60) - (end - start));
	#endif
}

void update(void) {
	while (SDL_PollEvent(event)) {
		switch (event->type) {
			case SDL_QUIT: running = false; break;
			case SDL_MOUSEBUTTONDOWN: pressed = true; break;
			case SDL_KEYDOWN:
				switch (event->key.keysym.sym) {
					case SDLK_ESCAPE: if(!paused) paused = true; else paused = false; break;
					case SDLK_SPACE: pressed = true; break;
					#ifdef __ANDROID__
					case SDLK_AC_BACK: running = false; break;
					#endif
				} break;
		}
	}

	if (pressed) {
		paused = false;
		jumpQ = 15;
		jumpVel = 3;
		pressed = false;
	}

	if (!paused) {
		if (jumpQ == 0) {
			jumpVel = -3;
		}

		bird.y -= jumpVel;
		if (jumpQ > 0) jumpQ--;
		for (int i = 0; i < 6; i++) {
			pipe[i].x -= 2;
		}
	}

	if (bird.y < 0 || bird.y + 32 > 600) reset();
	if (SDL_HasIntersection(&bird, &pipe[nextpipe])) reset();
	if (SDL_HasIntersection(&bird, &pipe[nextpipe + 1])) reset();

	if (bird.x == pipe[nextpipe].x + 32) {
		if (++score > best) best = score;
		#ifndef __ANDROID__
		SDL_snprintf(title_buf, 100, "FunkyBird - %i", score);
		SDL_SetWindowTitle(window, title_buf);
		#else
		SDL_snprintf(title_buf, 100, "%i", score);
		SDL_AndroidShowToast(title_buf, 0, 0, 0, 0);
		#endif
	}

	if (pipe[nextpipe].x + 32 < 0) {
		switch (nextpipe) {
			case 0:
				pipe[nextpipe].x = ((10 * 32) + pipe[4].x);
				pipe[nextpipe].h = 32 * ((rand() % 14) + 1);
				break;
			case 2:
				pipe[nextpipe].x = ((10 * 32) + pipe[0].x);
				pipe[nextpipe].h = 32 * ((rand() % 14) + 1);
				break;
			case 4:
				pipe[nextpipe].x = ((10 * 32) + pipe[2].x);
				pipe[nextpipe].h = 32 * ((rand() % 14) + 1);
				break;
		}

		pipe[nextpipe + 1].x = pipe[nextpipe].x;
		pipe[nextpipe + 1].y = pipe[nextpipe].h + (3 * 32);
		pipe[nextpipe + 1].h = 600 - pipe[nextpipe].y;

		if (nextpipe == 4) nextpipe = 0;
		else nextpipe += 2;
	}
}

void reset(void) {
	#ifndef __ANDROID__
	SDL_snprintf(title_buf, 100, "FunkyBird - Best: %i Last: %i", best, score);
	SDL_SetWindowTitle(window, title_buf);
	#else
	SDL_snprintf(title_buf, 100, "Best: %i Last: %i", best, score);
	SDL_AndroidShowToast(title_buf, 1, 0, 0, 0);
	#endif
	score = 0;
	paused = true;

	for (int i = 0; i < 6; i++) {
		if ((i + 1) % 2 == 0) {
			pipe[i].x = pipe[i - 1].x;
			pipe[i].y = pipe[i - 1].h + (3 * 32);
			pipe[i].h = 600 - pipe[i].y;
		} else {
			pipe[i].x = (800 - 32) + ((i * 5) * 32);
			pipe[i].y = 0;
			pipe[i].h = 32 * ((rand() % 14) + 1);
		}

		pipe[i].w = 32;
	}

	bird.y = ((600 / 2) - (32 / 2));
	nextpipe = 0;
}

void render(void) {
	SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderFillRect(renderer, &bird);

	SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
	SDL_RenderFillRects(renderer, pipe, 6);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(renderer, &border);

	SDL_RenderPresent(renderer);
}

void init(void) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("Failure initializing SDL: %s\n", SDL_GetError());
		exit(1);
	}

	if (SDL_CreateWindowAndRenderer(800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE, &window, &renderer) < 0) {
		SDL_Log("Failure creating window and renderer: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	SDL_RenderSetLogicalSize(renderer, 800, 600);
	running = true;
	pressed = false;
	best = 0;
	score = 0;
	srand(time(0));

	#ifdef __EMSCRIPTEN__
	printf("Hello, rayi512x here. Thanks for playing my game!\nDo not fullscreen! It does not work.\n");
	#endif

	reset();
}

void quit(void) {
	SDL_DestroyRenderer(renderer);
	renderer = NULL;

	SDL_DestroyWindow(window);
	window = NULL;

	SDL_Quit();
	event = NULL;
}
