#include <SDL.h>
#include <iostream>
#include <cmath>
using namespace std;
#define PI 3.14159265358979323846

int main(int, char**) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}//初始化SDL

	SDL_Rect screen_rect;
	SDL_GetDisplayBounds(0, &screen_rect);//获取屏幕大小
	

	const int window_width = 200;
	const int window_height = 150;//窗口大小
	int x = 0;
	int y = screen_rect.h / 2 - window_height / 2;//窗口初始位置
	SDL_Window* window = SDL_CreateWindow("Mouse_Cage", x, y, 
		window_width, window_height, SDL_WINDOW_SHOWN);//创建窗口
	SDL_SetWindowMouseGrab(window, SDL_TRUE);//控制鼠标
	
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);//创建着色器 

	bool isquit = false;
	SDL_Event event;//初始化事件队列


	while (!isquit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				isquit = true;//如果遇到退出事件就关闭窗口
			}
		}
		SDL_Rect rect = { 10, 10, 150, 100 };//定义一个矩形
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);//设置画笔颜色
		SDL_RenderDrawRect(renderer, &rect);//画矩形
		SDL_RenderPresent(renderer);//刷新画布
		x += 1;
		y = screen_rect.h / 2 - window_height / 2 + 200 * sin(4 * PI * x / screen_rect.w);//窗口运动轨迹
		if (x >= screen_rect.w) x = -window_width;//循环运动
		SDL_SetWindowPosition(window, x, y);//设置窗口位置
		SDL_Delay(2.5);//延时
	}
	
	SDL_DestroyRenderer(renderer);//销毁着色器
	SDL_DestroyWindow(window);//销毁窗口
	SDL_Quit();//退出SDL
	return 0;
}