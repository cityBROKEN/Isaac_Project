//����ļ���һ������SDL�Ľ�ѧ�Դ��룬����չʾSDL�Ļ���ʹ�÷������������ڵĴ�������Ⱦ���Ĵ������¼�����ѯ�����ڵ��ƶ������ڵĿ��Ƶȡ�

#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
using namespace std;
#define PI 3.14159265358979323846

int main(int, char**) {

	//��ʼ��SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());//�����ʼ��ʧ�ܾ����������Ϣ
		return 1;
	}
	//ע���·���SDL_Quit()�������д�����Ҫ�����٣����������ڴ�й©

	bool isquit = false;//��ʼ���˳���־
	SDL_Event event;//��ʼ���¼����У����ڽ����¼������˳��¼�������¼���

	SDL_Rect screen_rect;//������Ķ���
	SDL_GetDisplayBounds(0, &screen_rect);//��ȡ��Ļ��С


	/* ���ڴ��� */
	const int window_width = 200;
	const int window_height = 150;//���ڴ�С
	int x = 0;
	int y = screen_rect.h / 2 - window_height / 2;//���ڳ�ʼλ��
	SDL_Window* window = SDL_CreateWindow("Mouse_Cage", x, y,
		window_width, window_height, SDL_WINDOW_SHOWN);
	//�������ڣ���������ΪMouse_Cage������λ��Ϊx,y�����ڴ�СΪwindow_width,window_height������״̬Ϊ��ʾ
	SDL_SetWindowMouseGrab(window, SDL_TRUE);//���ڿ������
	//ע���·���SDL_DestroyWindow(window)�������д�����Ҫ�����٣����������ڴ�й©


	/* ������Ⱦ�� */
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	//������Ⱦ������Ⱦ���󶨵����ڣ�-1��ʾʹ��Ĭ����������0��ʾʹ��Ĭ����Ⱦ��
	

	/* �������� */
	SDL_Texture* texture = IMG_LoadTexture(renderer, "ISAAC/Characters/isaac1.png");
	//Surface��Texture������Surface��CPU�ڴ��е��������ݣ�Texture��GPU�ڴ��е���������
	//Texture�����ƣ�Texture����ֱ����GPU����Ⱦ���ٶȸ��죬Surface��Ҫ��ת����Texture������GPU����Ⱦ
	//ע���·���SDL_DestroyTexture(texture)�������д�����Ҫ�����٣����������ڴ�й©


	while (!isquit) {//ѭ��,ֱ���˳���־Ϊ��

		while (SDL_PollEvent(&event)) {//��ѯ�¼�,ֱ���¼�����Ϊ��
			if (event.type == SDL_QUIT) {//��������˳��¼��͹رմ���
				isquit = true;
			}
			if (event.type == SDL_MOUSEBUTTONDOWN) {//���������갴���¼�
				SDL_Log("Mouse button down");
			}
			if (event.type == SDL_MOUSEMOTION) {//�����������ƶ��¼�
				SDL_Log("Mouse position: %d,%d", event.motion.x, event.motion.y);
			}
			if (event.type == SDL_KEYDOWN) {//����������̰����¼�
				SDL_Log("You pressed: %s", SDL_GetKeyName(event.key.keysym.sym));
			}
			if (event.type == SDL_KEYUP) {//�����������̧���¼�
				SDL_Log("You released: %s", SDL_GetKeyName(event.key.keysym.sym));
			}
		}

		/* ��Ⱦ����ʹ�ã���ͼ��*/
		//ע��󻭵�ͼ���Ḳ���Ȼ���ͼ��
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		//���û�����ɫ��RGBΪ0,255,0��͸����Ϊ255����ֵ��ΧΪ0-255����ֵԽ����ɫԽ��
		//ע���·���SDL_DestroyRenderer(renderer)�������д�����Ҫ�����٣����������ڴ�й©
		SDL_RenderClear(renderer);//�����Ⱦ��������Ϊ���ô��ڱ�������ɫ

		SDL_Rect fill_rect = { 10, 10, 150, 100 };//����һ������,���Ͻ�����Ϊ10,10����Ϊ150����Ϊ100
		SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);//���û�����ɫ
		SDL_RenderFillRect(renderer, &fill_rect);//������,ע�⴫�ݵ��Ǿ��ε�ָ�룬���һ����Ǿ��ε����
		
		SDL_Rect rect = { 10, 10, 150, 100 };//����һ������,���Ͻ�����Ϊ10,10����Ϊ150����Ϊ100
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);//���û�����ɫ
		SDL_RenderDrawRect(renderer, &rect);//������,ע�⴫�ݵ��Ǿ��ε�ָ�룬���һ����Ǿ��εı߿�

		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);//���û�����ɫ
		SDL_RenderDrawLine(renderer, 0, 0, 200, 200);//����,�������Ϊ0,0���յ�����Ϊ200,200

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);//���û�����ɫ
		SDL_RenderDrawPoint(renderer, 50, 100);//����,����Ϊ50,100

		SDL_RenderCopy(renderer, texture, NULL, NULL);//����������Ⱦ��,ע�⴫�ݵ��Ǿ��ε�ָ��,
		//ע�⣺����ĸ���������Ⱦ���Ͻ��еģ��������ڴ����Ͻ��е�
		//�ĸ������ֱ��ǣ���Ⱦ��������������NULL��ʾ���������������ĸ�NULL��ʾ���Ƶ�������Ⱦ��

		
		
		SDL_RenderPresent(renderer);//ˢ����Ⱦ��(�������ϵ�������ʾ��������)



		/* ���ڵ��˶� */
		x += 1;
		y = screen_rect.h / 2 - window_height / 2 + 200 * sin(4 * PI * x / screen_rect.w);//�����˶��켣����
		if (x >= screen_rect.w) x = -window_width;//ѭ���˶�
		SDL_SetWindowPosition(window, x, y);//���ô���λ��
		SDL_Delay(2.5);//��ʱ
		//ÿ��λ�ø��µĲ�������ʱʱ��������ڵ��˶��ٶȺ��˶�������
	}


	SDL_DestroyTexture(texture);//��������
	SDL_DestroyRenderer(renderer);//������ɫ��
	SDL_DestroyWindow(window);//���ٴ���
	SDL_Quit();//�˳�SDL
	return 0;
}