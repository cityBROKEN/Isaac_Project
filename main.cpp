#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
using namespace std;
#define PI 3.14159265358979323846

// 定义速度常量
const int PLAYER_SPEED = 5;

// 定义按键状态数组，依次为W, A, S, D
bool keyStates[4] = { false, false, false, false };

// 每帧处理按键事件和移动逻辑
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

// 更新角色位置
void updatePlayerPosition(SDL_Rect& dstrect, const bool keyStates[4], int window_width, int window_height) {
    if (keyStates[0] && dstrect.y > 0)                     dstrect.y -= PLAYER_SPEED; // W - 上移
    if (keyStates[1] && dstrect.x > 0)                     dstrect.x -= PLAYER_SPEED; // A - 左移
    if (keyStates[2] && dstrect.y < window_height - dstrect.h) dstrect.y += PLAYER_SPEED; // S - 下移
    if (keyStates[3] && dstrect.x < window_width - dstrect.w)  dstrect.x += PLAYER_SPEED; // D - 右移
}

int main(int, char**) {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // 初始化事件队列
    bool isquit = false;
    SDL_Event event;

    // 获取屏幕大小
    SDL_Rect screen_rect;
    SDL_GetDisplayBounds(0, &screen_rect);

    // 创建窗口
    const int window_width = 1600;
    const int window_height = 900;
    int x = screen_rect.w / 2 - window_width / 2;
    int y = screen_rect.h / 2 - window_height / 2;
    SDL_Window* window = SDL_CreateWindow("Mouse_Cage", x, y,
        window_width, window_height, SDL_WINDOW_SHOWN);

    // 创建渲染
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 创建纹理
    SDL_Texture* isaac1 = IMG_LoadTexture(renderer, "ISAAC/Characters/isaac1.png");
    SDL_Texture* basement = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/basement.png");

    // 初始化纹理位置
    SDL_Rect dstrect;
    SDL_QueryTexture(isaac1, NULL, NULL, &dstrect.w, &dstrect.h);
    dstrect.w *= 4;
    dstrect.h *= 4;
    dstrect.x = window_width / 2 - dstrect.w / 2;
    dstrect.y = window_height / 2 - dstrect.h / 2;

    while (!isquit) {
        // 处理输入
        processInput(event, isquit, keyStates);

        // 更新角色位置
        updatePlayerPosition(dstrect, keyStates, window_width, window_height);

        // 渲染
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, basement, NULL, NULL);
        SDL_RenderCopy(renderer, isaac1, NULL, &dstrect);
        SDL_RenderPresent(renderer);

        // 控制帧率
        SDL_Delay(16); // 约 60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
