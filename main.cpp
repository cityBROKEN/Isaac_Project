#include <SDL.h>
#include <iostream>
#include <cmath>
using namespace std;
#define PI 3.14159265358979323846

int main(int, char**) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}//��ʼ��SDL

	SDL_Rect screen_rect;
	SDL_GetDisplayBounds(0, &screen_rect);//��ȡ��Ļ��С
	

	const int window_width = 200;
	const int window_height = 150;//���ڴ�С
	int x = 0;
	int y = screen_rect.h / 2 - window_height / 2;//���ڳ�ʼλ��
	SDL_Window* window = SDL_CreateWindow("Mouse_Cage", x, y, 
		window_width, window_height, SDL_WINDOW_SHOWN);//��������
	SDL_SetWindowMouseGrab(window, SDL_TRUE);//�������
	
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);//������ɫ�� 

	bool isquit = false;
	SDL_Event event;//��ʼ���¼�����


	while (!isquit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				isquit = true;//��������˳��¼��͹رմ���
			}
		}
		SDL_Rect rect = { 10, 10, 150, 100 };//����һ������
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);//���û�����ɫ
		SDL_RenderDrawRect(renderer, &rect);//������
		SDL_RenderPresent(renderer);//ˢ�»���
		x += 1;
		y = screen_rect.h / 2 - window_height / 2 + 200 * sin(4 * PI * x / screen_rect.w);//�����˶��켣
		if (x >= screen_rect.w) x = -window_width;//ѭ���˶�
		SDL_SetWindowPosition(window, x, y);//���ô���λ��
		SDL_Delay(2.5);//��ʱ
	}
	
	SDL_DestroyRenderer(renderer);//������ɫ��
	SDL_DestroyWindow(window);//���ٴ���
	SDL_Quit();//�˳�SDL
	return 0;
}