//这个文件是一个包含SDL的教学性代码，用于展示SDL的基本使用方法，包括窗口的创建、渲染器的创建、事件的轮询、窗口的移动、窗口的控制等。

#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
using namespace std;
#define PI 3.14159265358979323846

int main(int, char**) {

	//初始化SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());//如果初始化失败就输出错误信息
		return 1;
	}
	//注意下方有SDL_Quit()函数，有创建就要有销毁，否则会出现内存泄漏

	bool isquit = false;//初始化退出标志
	SDL_Event event;//初始化事件队列，用于接收事件，如退出事件、鼠标事件等

	SDL_Rect screen_rect;//矩形类的定义
	SDL_GetDisplayBounds(0, &screen_rect);//获取屏幕大小


	/* 关于窗口 */
	const int window_width = 200;
	const int window_height = 150;//窗口大小
	int x = 0;
	int y = screen_rect.h / 2 - window_height / 2;//窗口初始位置
	SDL_Window* window = SDL_CreateWindow("Mouse_Cage", x, y,
		window_width, window_height, SDL_WINDOW_SHOWN);
	//创建窗口，窗口名字为Mouse_Cage，窗口位置为x,y，窗口大小为window_width,window_height，窗口状态为显示
	SDL_SetWindowMouseGrab(window, SDL_TRUE);//窗口控制鼠标
	//注意下方有SDL_DestroyWindow(window)函数，有创建就要有销毁，否则会出现内存泄漏


	/* 关于渲染器 */
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	//创建渲染器，渲染器绑定到窗口，-1表示使用默认驱动器，0表示使用默认渲染器
	

	/* 关于纹理 */
	SDL_Texture* texture = IMG_LoadTexture(renderer, "ISAAC/Characters/isaac1.png");
	//Surface和Texture的区别：Surface是CPU内存中的像素数据，Texture是GPU内存中的像素数据
	//Texture的优势：Texture可以直接在GPU上渲染，速度更快，Surface需要先转换成Texture才能在GPU上渲染
	//注意下方有SDL_DestroyTexture(texture)函数，有创建就要有销毁，否则会出现内存泄漏


	while (!isquit) {//循环,直到退出标志为真

		while (SDL_PollEvent(&event)) {//轮询事件,直到事件队列为空
			if (event.type == SDL_QUIT) {//如果遇到退出事件就关闭窗口
				isquit = true;
			}
			if (event.type == SDL_MOUSEBUTTONDOWN) {//如果遇到鼠标按下事件
				SDL_Log("Mouse button down");
			}
			if (event.type == SDL_MOUSEMOTION) {//如果遇到鼠标移动事件
				SDL_Log("Mouse position: %d,%d", event.motion.x, event.motion.y);
			}
			if (event.type == SDL_KEYDOWN) {//如果遇到键盘按下事件
				SDL_Log("You pressed: %s", SDL_GetKeyName(event.key.keysym.sym));
			}
			if (event.type == SDL_KEYUP) {//如果遇到键盘抬起事件
				SDL_Log("You released: %s", SDL_GetKeyName(event.key.keysym.sym));
			}
		}

		/* 渲染器的使用（画图）*/
		//注意后画的图案会覆盖先画的图案
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		//设置画笔颜色，RGB为0,255,0，透明度为255，数值范围为0-255，数值越大颜色越深
		//注意下方有SDL_DestroyRenderer(renderer)函数，有创建就要有销毁，否则会出现内存泄漏
		SDL_RenderClear(renderer);//清空渲染器，表现为设置窗口背景的颜色

		SDL_Rect fill_rect = { 10, 10, 150, 100 };//定义一个矩形,左上角坐标为10,10，宽为150，高为100
		SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);//设置画笔颜色
		SDL_RenderFillRect(renderer, &fill_rect);//画矩形,注意传递的是矩形的指针，而且画的是矩形的填充
		
		SDL_Rect rect = { 10, 10, 150, 100 };//定义一个矩形,左上角坐标为10,10，宽为150，高为100
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);//设置画笔颜色
		SDL_RenderDrawRect(renderer, &rect);//画矩形,注意传递的是矩形的指针，而且画的是矩形的边框

		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);//设置画笔颜色
		SDL_RenderDrawLine(renderer, 0, 0, 200, 200);//画线,起点坐标为0,0，终点坐标为200,200

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);//设置画笔颜色
		SDL_RenderDrawPoint(renderer, 50, 100);//画点,坐标为50,100

		SDL_RenderCopy(renderer, texture, NULL, NULL);//复制纹理到渲染器,注意传递的是矩形的指针,
		//注意：纹理的复制是在渲染器上进行的，而不是在窗口上进行的
		//四个参数分别是：渲染器、纹理，第三个NULL表示复制整个纹理，第四个NULL表示复制到整个渲染器

		
		
		SDL_RenderPresent(renderer);//刷新渲染器(将画布上的内容显示到窗口上)



		/* 窗口的运动 */
		x += 1;
		y = screen_rect.h / 2 - window_height / 2 + 200 * sin(4 * PI * x / screen_rect.w);//窗口运动轨迹函数
		if (x >= screen_rect.w) x = -window_width;//循环运动
		SDL_SetWindowPosition(window, x, y);//设置窗口位置
		SDL_Delay(2.5);//延时
		//每次位置更新的步长和延时时间决定窗口的运动速度和运动流畅度
	}


	SDL_DestroyTexture(texture);//销毁纹理
	SDL_DestroyRenderer(renderer);//销毁着色器
	SDL_DestroyWindow(window);//销毁窗口
	SDL_Quit();//退出SDL
	return 0;
}