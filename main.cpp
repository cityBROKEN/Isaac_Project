#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
using namespace std;
#define PI 3.14159265358979323846

// �����ٶȳ���
const int PLAYER_SPEED = 5;

// ���尴��״̬���飬����ΪW, A, S, D
bool keyStates[4] = { false, false, false, false };

// ÿ֡�������¼����ƶ��߼�
void processInput(SDL_Event& event, bool& isquit, bool keyStates[4]) {
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
            }
        }
        if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
            case SDLK_w: keyStates[0] = false; break;
            case SDLK_a: keyStates[1] = false; break;
            case SDLK_s: keyStates[2] = false; break;
            case SDLK_d: keyStates[3] = false; break;
            }
        }
    }
}

// ���½�ɫλ��
void updatePlayerPosition(SDL_Rect& dstrect, const bool keyStates[4], int window_width, int window_height) {
    if (keyStates[0] && dstrect.y > 0)                     dstrect.y -= PLAYER_SPEED; // W - ����
    if (keyStates[1] && dstrect.x > 0)                     dstrect.x -= PLAYER_SPEED; // A - ����
    if (keyStates[2] && dstrect.y < window_height - dstrect.h) dstrect.y += PLAYER_SPEED; // S - ����
    if (keyStates[3] && dstrect.x < window_width - dstrect.w)  dstrect.x += PLAYER_SPEED; // D - ����
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

    // ��������
    SDL_Texture* isaac1 = IMG_LoadTexture(renderer, "ISAAC/Characters/isaac1.png");
    SDL_Texture* basement = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/basement.png");

    // ��ʼ������λ��
    SDL_Rect dstrect;
    SDL_QueryTexture(isaac1, NULL, NULL, &dstrect.w, &dstrect.h);
    dstrect.w *= 4;
    dstrect.h *= 4;
    dstrect.x = window_width / 2 - dstrect.w / 2;
    dstrect.y = window_height / 2 - dstrect.h / 2;

    while (!isquit) {
        // ��������
        processInput(event, isquit, keyStates);

        // ���½�ɫλ��
        updatePlayerPosition(dstrect, keyStates, window_width, window_height);

        // ��Ⱦ
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, basement, NULL, NULL);
        SDL_RenderCopy(renderer, isaac1, NULL, &dstrect);
        SDL_RenderPresent(renderer);

        // ����֡��
        SDL_Delay(16); // Լ 60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
