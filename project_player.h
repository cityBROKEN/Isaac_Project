#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
using namespace std;
#define PI 3.14159265358979323846


//枚举方向
enum Direction { UP, DOWN, LEFT, RIGHT };
Direction head_direction = DOWN; // 默认朝向下
// 添加方向变量
int bodyDirection = 0; // 默认静止
// 方向结构体
struct DIRECTION {
    double radian;
};
double distance(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}



/* —————————— 角色类定义 —————————— */
class PLAYER {
public:
    //角色基本面板属性
    int x;//角色坐标
    int y;//角色坐标
    int HP;//角色血量
    double speed;//角色速度
    double damage;//角色伤害
    double tear;//角色射速
    double attack_interval;// 攻击间隔（时间单位为毫秒）
    double shootspeed;//角色弹速
    double range;//角色射程
    SDL_Rect bumpbox;//碰撞箱
    // 无敌状态相关变量
    bool isInvincible;          // 是否处于无敌状态
    Uint32 invincibleStartTime; // 无敌开始时间
    Uint32 invincibleDuration;  // 无敌持续时间（毫秒）
    PLAYER(int x, int y, int w, int h, double HP, double speed, double damage, double tear, double shootspeed, double range) {
        this->x = x;
        this->y = y;
        this->HP = HP;
        this->speed = speed;
        this->damage = damage;
        this->tear = tear;
        this->shootspeed = shootspeed;
        this->range = range;
        this->bumpbox.x = x;
        this->bumpbox.y = y;
        this->bumpbox.w = w;
        this->bumpbox.h = h;
        this->attack_interval = 1000.0 / tear;
        this->isInvincible = false;
        this->invincibleStartTime = 0;
        this->invincibleDuration = 1000; // 无敌持续时间为1秒，可根据需要调整
    }
};










/* —————————— 角色相关动画 —————————— */

// 人物行走背面动画
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
// 人物行走正面动画
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
// 人物行走右面动画
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
// 人物行走左面动画
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
// 人物向上射击动画
void shootUpMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BackHeadMotions, SDL_Rect& headrect) {
    static int currentFrame = 0;
    static Uint32 lastFrameTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    // 每200ms切换帧，按下按键时重置
    if (currentTime - lastFrameTime >= 200 || currentFrame == 0) {
        currentFrame = (currentFrame + 1) % BackHeadMotions.size();
        lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, BackHeadMotions[currentFrame], NULL, &headrect);
}
// 人物向左射击动画
void shootLeftMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& LeftHeadMotions, SDL_Rect& headrect) {
    static int currentFrame = 0;
    static Uint32 lastFrameTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    // 每200ms切换帧，按下按键时重置
    if (currentTime - lastFrameTime >= 200 || currentFrame == 0) {
        currentFrame = (currentFrame + 1) % LeftHeadMotions.size();
        lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, LeftHeadMotions[currentFrame], NULL, &headrect);
}
// 人物向下射击动画
void shootDownMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FrontHeadMotions, SDL_Rect& headrect) {
    static int currentFrame = 0;
    static Uint32 lastFrameTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    // 每200ms切换帧，按下按键时重置
    if (currentTime - lastFrameTime >= 200 || currentFrame == 0) {
        currentFrame = (currentFrame + 1) % FrontHeadMotions.size();
        lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, FrontHeadMotions[currentFrame], NULL, &headrect);
}
// 人物向右射击动画
void shootRightMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& RightHeadMotions, SDL_Rect& headrect) {
    static int currentFrame = 0;
    static Uint32 lastFrameTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    // 每200ms切换帧，按下按键时重置
    if (currentTime - lastFrameTime >= 200 || currentFrame == 0) {
        currentFrame = (currentFrame + 1) % RightHeadMotions.size();
        lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, RightHeadMotions[currentFrame], NULL, &headrect);
}




/* —————————— 人物事件处理和动画渲染 —————————— */

// 定义按键状态数组，依次为W, A, S, D, UP, LEFT, DOWN, RIGHT
bool keyStates[8] = { false, false, false, false, false, false, false, false };

//每帧处理移动事件
void processInput(SDL_Event& event, bool& isquit, bool keyStates[8]) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isquit = true;
        }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                // 移动按键
            case SDLK_w: keyStates[0] = true; break;
            case SDLK_a: keyStates[1] = true; break;
            case SDLK_s: keyStates[2] = true; break;
            case SDLK_d: keyStates[3] = true; break;
                // 攻击按键
            case SDLK_UP: keyStates[4] = true; break;
            case SDLK_LEFT: keyStates[5] = true; break;
            case SDLK_DOWN: keyStates[6] = true; break;
            case SDLK_RIGHT: keyStates[7] = true; break;
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

// 更新角色位置
void updatePlayerPosition(SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[4], int window_width,
    int window_height, int& bodyDirection, PLAYER& isaac) {
    if (keyStates[0] && headrect.y > 0) {
        headrect.y -= 2.5 * isaac.speed;
        bodyrect.y -= 2.5 * isaac.speed;
        bodyDirection = 1; // 上
    }
    else if (keyStates[1] && headrect.x > 0) {
        headrect.x -= 2.5 * isaac.speed;
        bodyrect.x -= 2.5 * isaac.speed;
        bodyDirection = 3; // 左
    }
    else if (keyStates[2] && bodyrect.y < window_height - bodyrect.h) {
        headrect.y += 2.5 * isaac.speed;
        bodyrect.y += 2.5 * isaac.speed;
        bodyDirection = 2; // 下
    }
    else if (keyStates[3] && headrect.x < window_width - headrect.w) {
        headrect.x += 2.5 * isaac.speed;
        bodyrect.x += 2.5 * isaac.speed;
        bodyDirection = 4; // 右
    }
    else {
        bodyDirection = 0; // 静止
    }
    // 更新 isaac 的中心点坐标
    isaac.x = headrect.x + headrect.w / 2 - 45;
    isaac.y = headrect.y + (headrect.h + bodyrect.h) / 2 - 65;
    // 更新角色的碰撞箱
    isaac.bumpbox.x = headrect.x + 15;
    isaac.bumpbox.y = headrect.y + 10;
    isaac.bumpbox.w = headrect.w - 30;
    isaac.bumpbox.h = headrect.h - 10;
    // 在主循环或专门的更新函数中
    Uint32 currentTime = SDL_GetTicks();
    if (isaac.isInvincible && currentTime - isaac.invincibleStartTime >= isaac.invincibleDuration) {
        isaac.isInvincible = false; // 结束无敌状态
    }

}

// 更新角色动画
void updatePlayerMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BackMotions, vector<SDL_Texture*>& FrontMotions,
    vector<SDL_Texture*>& RightMotions, vector<SDL_Texture*>& LeftMotions, vector<SDL_Texture*>& BackHeadMotions,
    vector<SDL_Texture*>& FrontHeadMotions, vector<SDL_Texture*>& RightHeadMotions, vector<SDL_Texture*>& LeftHeadMotions,
    SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[8], int bodyDirection, bool& is_attacking, PLAYER& player) {
    // 在渲染角色之前，检查是否处于无敌状态
    if (player.isInvincible) {
        // 计算闪烁效果，例如每隔 100ms 切换一次可见性
        Uint32 currentTime = SDL_GetTicks();
        if ((currentTime / 100) % 2 == 0) {
            // 此时不渲染角色，实现闪烁效果
            return;
        }
    }
    // 先处理身体动画
    switch (bodyDirection) {
    case 1: backMotion(renderer, BackMotions, headrect, bodyrect); break;
    case 2: frontMotion(renderer, FrontMotions, headrect, bodyrect); break;
    case 3: leftMotion(renderer, LeftMotions, headrect, bodyrect); break;
    case 4: rightMotion(renderer, RightMotions, headrect, bodyrect); break;
    default: SDL_RenderCopy(renderer, FrontMotions[3], NULL, &bodyrect); break;
    }

    // 按键优先触发头部攻击动画
    if (keyStates[4]) {
        if (is_attacking) shootUpMotion(renderer, BackHeadMotions, headrect);
    }
    else if (keyStates[5]) {
        if (is_attacking) shootLeftMotion(renderer, LeftHeadMotions, headrect);
    }
    else if (keyStates[6]) {
        if (is_attacking) shootDownMotion(renderer, FrontHeadMotions, headrect);
    }
    else if (keyStates[7]) {
        if (is_attacking) shootRightMotion(renderer, RightHeadMotions, headrect);
    }
    else {
        // 默认渲染头部静止状态
        switch (bodyDirection) {
        case 1: SDL_RenderCopy(renderer, BackHeadMotions[0], NULL, &headrect); break;
        case 2: SDL_RenderCopy(renderer, FrontHeadMotions[0], NULL, &headrect); break;
        case 3: SDL_RenderCopy(renderer, LeftHeadMotions[0], NULL, &headrect); break;
        case 4: SDL_RenderCopy(renderer, RightHeadMotions[0], NULL, &headrect); break;
        default: SDL_RenderCopy(renderer, FrontHeadMotions[0], NULL, &headrect); break;
        }
    }
}

// 人物血量渲染
void renderPlayerHealth(SDL_Renderer* renderer, const PLAYER& player, SDL_Texture* full_heart, SDL_Texture* half_heart, SDL_Texture* empty_heart) {
    int maxHearts = 6; // 假设最大心数为6
    int heartWidth = 30;
    int heartHeight = 30;

    for (int i = 0; i < maxHearts; ++i) {
        SDL_Rect heartRect = { 10 + i * (heartWidth + 5), 10, heartWidth, heartHeight };
        if (player.HP >= (i + 1) * 2) {
            // 绘制满心
            SDL_RenderCopy(renderer, full_heart, NULL, &heartRect);
        }
        else if (player.HP == (i * 2) + 1) {
            // 绘制半心
            SDL_RenderCopy(renderer, half_heart, NULL, &heartRect);
        }
        else {
            // 绘制空心
            SDL_RenderCopy(renderer, empty_heart, NULL, &heartRect);
        }
    }
}



