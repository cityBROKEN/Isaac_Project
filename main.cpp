//�����»��ߣ�����С�շ壬������շ壬��ͽṹ��ȫ��д
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <ctime>
#include "main.h"
using namespace std;
#define PI 3.14159265358979323846

// �����ٶȳ���
const int PLAYER_SPEED = 5;
//ö�ٷ���
enum Direction { UP, DOWN, LEFT, RIGHT };
Direction head_direction = DOWN; // Ĭ�ϳ�����
// ��ӷ������
int bodyDirection = 0; // Ĭ�Ͼ�ֹ

/* �������������������� ���� �������������������� */




/* �������������������� ���� �������������������� */

// �������߱��涯��
void backMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BackMotions, SDL_Rect& headrect, SDL_Rect& bodyrect)
{
    static int currentFrame = 0;
    static Uint32 lastFrameTime = SDL_GetTicks();
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastFrameTime >= 100) {
        currentFrame = (currentFrame + 1) % 8;
        lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, BackMotions[currentFrame], NULL, &bodyrect);
}
//�����������涯��
void frontMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FrontMotions, SDL_Rect& headrect, SDL_Rect& bodyrect)
{
	static int currentFrame = 0;
	static Uint32 lastFrameTime = SDL_GetTicks();
	Uint32 currentTime = SDL_GetTicks();
	if (currentTime - lastFrameTime >= 100) {
		currentFrame = (currentFrame + 1) % 12;
		lastFrameTime = currentTime;
	}
	SDL_RenderCopy(renderer, FrontMotions[currentFrame], NULL, &bodyrect);
}
//�����������涯��
void rightMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& RightMotions, SDL_Rect& headrect, SDL_Rect& bodyrect)
{
	static int currentFrame = 0;
	static Uint32 lastFrameTime = SDL_GetTicks();
	Uint32 currentTime = SDL_GetTicks();
	if (currentTime - lastFrameTime >= 100) {
		currentFrame = (currentFrame + 1) % 10;
		lastFrameTime = currentTime;
	}
	SDL_RenderCopy(renderer, RightMotions[currentFrame], NULL, &bodyrect);
}
//�����������涯��
void leftMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& LeftMotions, SDL_Rect& headrect, SDL_Rect& bodyrect)
{
	static int currentFrame = 0;
	static Uint32 lastFrameTime = SDL_GetTicks();
	Uint32 currentTime = SDL_GetTicks();
	if (currentTime - lastFrameTime >= 100) {
		currentFrame = (currentFrame + 1) % 10;
		lastFrameTime = currentTime;
	}
	SDL_RenderCopy(renderer, LeftMotions[currentFrame], NULL, &bodyrect);
}
// �����������
void shootUpMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BackHeadMotions, SDL_Rect& headrect) {
    static int currentFrame = 0;
    static Uint32 lastFrameTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    // ÿ200ms�л�֡�����°���ʱ����
    if (currentTime - lastFrameTime >= 200 || currentFrame == 0) {
        currentFrame = (currentFrame + 1) % BackHeadMotions.size();
        lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, BackHeadMotions[currentFrame], NULL, &headrect);
}
// �����������
void shootLeftMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& LeftHeadMotions, SDL_Rect& headrect) {
    static int currentFrame = 0;
    static Uint32 lastFrameTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    // ÿ200ms�л�֡�����°���ʱ����
    if (currentTime - lastFrameTime >= 200 || currentFrame == 0) {
        currentFrame = (currentFrame + 1) % LeftHeadMotions.size();
        lastFrameTime = currentTime;
    }
	SDL_RenderCopy(renderer, LeftHeadMotions[currentFrame], NULL, &headrect);
}
// �����������
void shootDownMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FrontHeadMotions, SDL_Rect& headrect) {
    static int currentFrame = 0;
    static Uint32 lastFrameTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    // ÿ200ms�л�֡�����°���ʱ����
    if (currentTime - lastFrameTime >= 200 || currentFrame == 0) {
        currentFrame = (currentFrame + 1) % FrontHeadMotions.size();
        lastFrameTime = currentTime;
    }
	SDL_RenderCopy(renderer, FrontHeadMotions[currentFrame], NULL, &headrect);
}
// �����������
void shootRightMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& RightHeadMotions, SDL_Rect& headrect) {
    static int currentFrame = 0;
    static Uint32 lastFrameTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    // ÿ200ms�л�֡�����°���ʱ����
    if (currentTime - lastFrameTime >= 200 || currentFrame == 0) {
        currentFrame = (currentFrame + 1) % RightHeadMotions.size();
        lastFrameTime = currentTime;
    }
	SDL_RenderCopy(renderer, RightHeadMotions[currentFrame], NULL, &headrect);
}

/* ������������������������������������������������������������ */




// ���尴��״̬���飬����ΪW, A, S, D, UP, LEFT, DOWN, RIGHT
bool keyStates[8] = { false, false, false, false, false, false, false, false };


// ÿ֡�������¼����ƶ��߼�
void processInput(SDL_Event& event, bool& isquit, bool keyStates[8], Direction& head_direction) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isquit = true;
        }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_w: keyStates[0] = true; break;
            case SDLK_a: keyStates[1] = true; break;
            case SDLK_s: keyStates[2] = true; break;
            case SDLK_d: keyStates[3] = true; break;
            case SDLK_UP:
                keyStates[4] = true;
                head_direction = UP;
                break;
            case SDLK_LEFT:
                keyStates[5] = true;
                head_direction = LEFT;
                break;
            case SDLK_DOWN:
                keyStates[6] = true;
                head_direction = DOWN;
                break;
            case SDLK_RIGHT:
                keyStates[7] = true;
                head_direction = RIGHT;
                break;
            }
        }
        if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
            case SDLK_w: keyStates[0] = false; break;
            case SDLK_a: keyStates[1] = false; break;
            case SDLK_s: keyStates[2] = false; break;
            case SDLK_d: keyStates[3] = false; break;
            case SDLK_UP: keyStates[4] = false; break;
            case SDLK_LEFT: keyStates[5] = false; break;
            case SDLK_DOWN: keyStates[6] = false; break;
            case SDLK_RIGHT: keyStates[7] = false; break;
            }
        }
    }
}


// ���½�ɫλ�úͷ���
void updatePlayerPosition(SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[4], int window_width, int window_height, int& bodyDirection) {
    if (keyStates[0] && headrect.y > 0) {
        headrect.y -= PLAYER_SPEED;
        bodyrect.y -= PLAYER_SPEED;
        bodyDirection = 1; // ��
    }
    else if (keyStates[1] && headrect.x > 0) {
        headrect.x -= PLAYER_SPEED;
        bodyrect.x -= PLAYER_SPEED;
        bodyDirection = 3; // ��
    }
    else if (keyStates[2] && bodyrect.y < window_height - bodyrect.h) {
        headrect.y += PLAYER_SPEED;
        bodyrect.y += PLAYER_SPEED;
        bodyDirection = 2; // ��
    }
    else if (keyStates[3] && headrect.x < window_width - headrect.w) {
        headrect.x += PLAYER_SPEED;
        bodyrect.x += PLAYER_SPEED;
        bodyDirection = 4; // ��
    }
    else {
        bodyDirection = 0; // ��ֹ
    }
}


// ���½�ɫ����
void updatePlayerMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BackMotions, vector<SDL_Texture*>& FrontMotions,
    vector<SDL_Texture*>& RightMotions, vector<SDL_Texture*>& LeftMotions, vector<SDL_Texture*>& BackHeadMotions,
    vector<SDL_Texture*>& FrontHeadMotions, vector<SDL_Texture*>& RightHeadMotions, vector<SDL_Texture*>& LeftHeadMotions,
    SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[8], int bodyDirection) {

    // �ȴ������嶯��
    switch (bodyDirection) {
    case 1: backMotion(renderer, BackMotions, headrect, bodyrect); break;
    case 2: frontMotion(renderer, FrontMotions, headrect, bodyrect); break;
    case 3: leftMotion(renderer, LeftMotions, headrect, bodyrect); break;
    case 4: rightMotion(renderer, RightMotions, headrect, bodyrect); break;
    default: SDL_RenderCopy(renderer, FrontMotions[3], NULL, &bodyrect); break;
    }

    // �������ȴ���ͷ����������
    if (keyStates[4]) {
        shootUpMotion(renderer, BackHeadMotions, headrect);
    }
    else if (keyStates[5]) {
        shootLeftMotion(renderer, LeftHeadMotions, headrect);
    }
    else if (keyStates[6]) {
        shootDownMotion(renderer, FrontHeadMotions, headrect);
    }
    else if (keyStates[7]) {
        shootRightMotion(renderer, RightHeadMotions, headrect);
    }
    else {
        // Ĭ����Ⱦͷ����ֹ״̬
        switch (bodyDirection) {
        case 1: SDL_RenderCopy(renderer, BackHeadMotions[0], NULL, &headrect); break;
        case 2: SDL_RenderCopy(renderer, FrontHeadMotions[0], NULL, &headrect); break;
        case 3: SDL_RenderCopy(renderer, LeftHeadMotions[0], NULL, &headrect); break;
        case 4: SDL_RenderCopy(renderer, RightHeadMotions[0], NULL, &headrect); break;
        default: SDL_RenderCopy(renderer, FrontHeadMotions[0], NULL, &headrect); break;
        }
    }
}






int main(int, char**) {
    // ��ʼ��SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // ��ʼ���¼�����
    bool isquit = false;
    SDL_Event event;

    // ��ȡ��Ļ��С
    SDL_Rect screen_rect;
    SDL_GetDisplayBounds(0, &screen_rect);

    // ��������
    const int window_width = 1600;
    const int window_height = 900;
    int x = screen_rect.w / 2 - window_width / 2;
    int y = screen_rect.h / 2 - window_height / 2;
    SDL_Window* window = SDL_CreateWindow("Mouse_Cage", x, y,
        window_width, window_height, SDL_WINDOW_SHOWN);

    // ������Ⱦ
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    /* �������� */
    SDL_Texture* basement = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/basement.png");


    /* ��ʼ�������������� */
    vector<SDL_Texture*> BackMotions;
    for (int i = 1; i <= 8; ++i) {
        string filename = "ISAAC/Characters/Motions/BackMotion" + to_string(i) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture) {
            BackMotions.push_back(texture);
        } else {
            SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
        }
    }
    vector<SDL_Texture*> FrontMotions;
    for (int i = 1; i <= 12; ++i) {
        string filename = "ISAAC/Characters/Motions/FrontMotion" + to_string(i) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture) {
            FrontMotions.push_back(texture);
        }
        else {
            SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
        }
    }
    vector<SDL_Texture*> RightMotions;
    for (int i = 1; i <= 10; ++i) {
        string filename = "ISAAC/Characters/Motions/RightMotion" + to_string(i) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture) {
            RightMotions.push_back(texture);
        }
        else {
            SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
        }
    }
    vector<SDL_Texture*> LeftMotions;
    for (int i = 1; i <= 10; ++i) {
        string filename = "ISAAC/Characters/Motions/LeftMotion" + to_string(i) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture) {
            LeftMotions.push_back(texture);
        }
        else {
            SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
        }
    }
    vector<SDL_Texture*> BackHeadMotions;
    for (int i = 1; i <= 2; ++i) {
        string filename = "ISAAC/Characters/Motions/BackHeadMotion" + to_string(i) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture) {
            BackHeadMotions.push_back(texture);
        }
        else {
            SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
        }
    }
	vector<SDL_Texture*> FrontHeadMotions;
	for (int i = 1; i <= 2; ++i) {
		string filename = "ISAAC/Characters/Motions/FrontHeadMotion" + to_string(i) + ".png";
		SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
		if (texture) {
			FrontHeadMotions.push_back(texture);
		}
		else {
			SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
		}
	}
	vector<SDL_Texture*> RightHeadMotions;
	for (int i = 1; i <= 2; ++i) {
		string filename = "ISAAC/Characters/Motions/RightHeadMotion" + to_string(i) + ".png";
		SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
		if (texture) {
			RightHeadMotions.push_back(texture);
		}
		else {
			SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
		}
	}
	vector<SDL_Texture*> LeftHeadMotions;
	for (int i = 1; i <= 2; ++i) {
		string filename = "ISAAC/Characters/Motions/LeftHeadMotion" + to_string(i) + ".png";
		SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
		if (texture) {
			LeftHeadMotions.push_back(texture);
		}
		else {
			SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
		}
	}



    /* ��ʼ������λ�� */
    //ͷ��λ
	SDL_Rect headrect;
	SDL_QueryTexture(BackHeadMotions[0], NULL, NULL, &headrect.w, &headrect.h);
	headrect.w *= 3;
	headrect.h *= 3;
    headrect.x = window_width / 2 - headrect.w / 2;
	headrect.y = window_height / 2 - headrect.h / 2 - 12;
	//����λ
	SDL_Rect bodyrect;
	SDL_QueryTexture(BackMotions[0], NULL, NULL, &bodyrect.w, &bodyrect.h);
	bodyrect.w *= 3;
	bodyrect.h *= 3;
	bodyrect.x = window_width / 2 - bodyrect.w / 2;
    bodyrect.y = window_height / 2 - bodyrect.h / 2 + 12;





    while (!isquit) {

        // ��������
        processInput(event, isquit, keyStates, head_direction);

        // ��Ⱦ
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, basement, NULL, NULL);

        // ���½�ɫλ�úͶ���
        updatePlayerPosition(headrect, bodyrect, keyStates, window_width, window_height, bodyDirection);
        updatePlayerMotion(renderer, BackMotions, FrontMotions, RightMotions, LeftMotions,BackHeadMotions, 
            FrontHeadMotions, RightHeadMotions, LeftHeadMotions, headrect, bodyrect, keyStates, bodyDirection);


        //ˢ�»���     
        SDL_RenderPresent(renderer);

        // ����֡��
        SDL_Delay(16); // Լ 60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
