//变量下划线，函数小驼峰，数组大驼峰，类和结构体全大写
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <ctime>
#include "main.h"
using namespace std;
#define PI 3.14159265358979323846

// 定义速度常量
const int PLAYER_SPEED = 5;
//枚举方向
enum Direction { UP, DOWN, LEFT, RIGHT };
Direction head_direction = DOWN; // 默认朝向下
// 添加方向变量
int bodyDirection = 0; // 默认静止
// 方向结构体
struct DIRECTION {
    double radian;
};


/* —————————— 声音 —————————— */
Mix_Music  *main_msuic = NULL;
void playMusic()
{
    main_msuic = Mix_LoadMUS("ISAAC/Sound/test.wav");
}


/* —————————— 角色 —————————— */

//角色类
class PLAYER {
public:
    //角色基本面板属性
    double x;//角色坐标
    double y;//角色坐标
    double HP;//角色血量
    double speed;//角色速度
    double damage;//角色伤害
    double tear;//角色射速
    double shootspeed;//角色弹速
    double range;//角色射程
    SDL_Rect bumpbox;//碰撞箱
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
    }
};



/* —————————— 子弹 —————————— */

//子弹类
class BULLET {
public:
    //子弹基本面板属性
    int x;//子弹坐标
    int y;//子弹坐标
    double speed;//子弹速度
    double damage;//子弹伤害
    double range;//子弹射程
    SDL_Rect bumpbox;//碰撞箱
    DIRECTION direction;//子弹飞行方向
    //子弹行为函数
    BULLET(int x, int y, double speed, double damage, double range, DIRECTION direction) {
        this->x = x;
        this->y = y;
        this->speed = speed;
        this->damage = damage;
        this->range = range;
        this->bumpbox.x = x;
        this->bumpbox.y = y;
        this->bumpbox.w = 30;
        this->bumpbox.h = 30;
        this->direction = direction;
    }
    void updatePosition() {
        bumpbox.x += speed * cos(direction.radian);
        bumpbox.y += speed * sin(direction.radian);
        x = bumpbox.x;
        y = bumpbox.y;
        range -= speed;
    }
    void move() {
        //子弹移动
    }
    void hit() {
        //子弹击中
    }
    void die() {
        //子弹消失
    }
};

vector<BULLET> Bullets;





/* —————————— 怪物 —————————— */
//基类怪物
class MONSTER {
public:
    //怪物基本面板属性
    int id;//怪物id
    char name[20];//怪物名字
    double x;//怪物坐标
    double y;//怪物坐标
    double HP;//怪物血量
    int move_type;//怪物移动方式
    double speed;//怪物速度
    int attack_type;//怪物攻击方式
    double damage;//怪物伤害
    SDL_Rect bumpbox;//碰撞箱
    //怪物行为函数
    void move() {
        switch (move_type) {
        case 1://直线移动
            break;
        case 2://随机移动
            break;
        case 3://追踪移动
            break;
        }
    }
    void attack() {
        switch (attack_type) {
        case 1://近战攻击
            break;
        case 2://远程攻击
            break;
        }
    }
    void die() {
        //怪物死亡
    }



};
//苍蝇怪
class FLY :public MONSTER {
public:
    FLY() {
        id = 1;
        strcpy_s(name, "FLY");
        x = 0;
        y = 0;
        HP = 10;
        move_type = 1;
        speed = 5;
        attack_type = 1;
        damage = 1;
    }
    FLY* createFly(int x, int y) {
        FLY* fly = new FLY();
        fly->x = x;
        fly->y = y;
        return fly;
    }
};







/* —————————— 动画 —————————— */

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
//人物行走正面动画
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
//人物行走右面动画
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
//人物行走左面动画
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
// 向上射击动画
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
// 向左射击动画
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
// 向下射击动画
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
// 向右射击动画
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

/* —————————————————————————————— */




// 定义按键状态数组，依次为W, A, S, D, UP, LEFT, DOWN, RIGHT
bool keyStates[8] = { false, false, false, false, false, false, false, false };


// 每帧处理按键事件和移动逻辑
void processInput(SDL_Event& event, bool& isquit, bool keyStates[8], Direction& head_direction,
    SDL_Rect& headrect, SDL_Rect& bodyrect, PLAYER& isaac) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isquit = true;
        }
        if (event.type == SDL_KEYDOWN) {
            DIRECTION direction;
            switch (event.key.keysym.sym) {
                //移动按键
            case SDLK_w: keyStates[0] = true; break;
            case SDLK_a: keyStates[1] = true; break;
            case SDLK_s: keyStates[2] = true; break;
            case SDLK_d: keyStates[3] = true; break;
                //攻击按键
            case SDLK_UP:
                keyStates[4] = true;
                head_direction = UP;
                direction = { PI * 3 / 2 };
                Bullets.push_back(BULLET(headrect.x + headrect.w / 2, headrect.y + headrect.h / 2 - 5,
                    isaac.shootspeed, isaac.damage, isaac.range, direction));
                break;
            case SDLK_LEFT:
                keyStates[5] = true;
                head_direction = LEFT;
                direction = { PI };
                Bullets.push_back(BULLET(headrect.x + headrect.w / 2 - 5, headrect.y + headrect.h / 2,
                    isaac.shootspeed, isaac.damage, isaac.range, direction));
                break;
            case SDLK_DOWN:
                keyStates[6] = true;
                head_direction = DOWN;
                direction = { PI / 2 };
                Bullets.push_back(BULLET(headrect.x + headrect.w / 2, headrect.y + headrect.h / 2 + 5,
                    isaac.shootspeed, isaac.damage, isaac.range, direction));
                break;
            case SDLK_RIGHT:
                keyStates[7] = true;
                head_direction = RIGHT;
                direction = { 0 };
                Bullets.push_back(BULLET(headrect.x + headrect.w / 2 + 5, headrect.y + headrect.h / 2,
                    isaac.shootspeed, isaac.damage, isaac.range, direction));
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


// 更新角色位置和方向
void updatePlayerPosition(SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[4], int window_width, int window_height, int& bodyDirection) {
    if (keyStates[0] && headrect.y > 0) {
        headrect.y -= PLAYER_SPEED;
        bodyrect.y -= PLAYER_SPEED;
        bodyDirection = 1; // 上
    }
    else if (keyStates[1] && headrect.x > 0) {
        headrect.x -= PLAYER_SPEED;
        bodyrect.x -= PLAYER_SPEED;
        bodyDirection = 3; // 左
    }
    else if (keyStates[2] && bodyrect.y < window_height - bodyrect.h) {
        headrect.y += PLAYER_SPEED;
        bodyrect.y += PLAYER_SPEED;
        bodyDirection = 2; // 下
    }
    else if (keyStates[3] && headrect.x < window_width - headrect.w) {
        headrect.x += PLAYER_SPEED;
        bodyrect.x += PLAYER_SPEED;
        bodyDirection = 4; // 右
    }
    else {
        bodyDirection = 0; // 静止
    }
}


// 更新角色动画
void updatePlayerMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BackMotions, vector<SDL_Texture*>& FrontMotions,
    vector<SDL_Texture*>& RightMotions, vector<SDL_Texture*>& LeftMotions, vector<SDL_Texture*>& BackHeadMotions,
    vector<SDL_Texture*>& FrontHeadMotions, vector<SDL_Texture*>& RightHeadMotions, vector<SDL_Texture*>& LeftHeadMotions,
    SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[8], int bodyDirection) {

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


// 更新子弹位置
void updateBullets(vector<BULLET>& bullets, int window_width, int window_height) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        it->updatePosition();
        // 如果子弹飞出窗口，则移除
        if (it->bumpbox.x < 0 || it->bumpbox.x > window_width ||
            it->bumpbox.y < 0 || it->bumpbox.y > window_height) {
            it = bullets.erase(it); // 删除并返回下一个迭代器
        }
        else {
            ++it;
        }
    }
}
// 子弹渲染
void renderBullets(SDL_Renderer* renderer, const vector<BULLET>& bullets, SDL_Texture* bulletTexture) {
    for (const auto& bullet : bullets) {
        SDL_RenderCopy(renderer, bulletTexture, NULL, &bullet.bumpbox);
    }
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

    //初始化音乐与音效
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }
    playMusic();
    if (main_msuic == NULL) {
        SDL_Log("Failed to load beat music! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }
    Mix_PlayMusic(main_msuic, -1);

    /* 创建纹理 */
    SDL_Texture* basement = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/basement.png");
    SDL_Texture* bulletTexture = IMG_LoadTexture(renderer, "ISAAC/Characters/bullet1.png");

    /* 初始化动画纹理数组 */
    vector<SDL_Texture*> BackMotions;
    for (int i = 1; i <= 8; ++i) {
        string filename = "ISAAC/Characters/Motions/BackMotion" + to_string(i) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture) {
            BackMotions.push_back(texture);
        }
        else {
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

    /* 初始化纹理位置 */
    //头部位
    SDL_Rect headrect;
    SDL_QueryTexture(BackHeadMotions[0], NULL, NULL, &headrect.w, &headrect.h);
    headrect.w *= 3;
    headrect.h *= 3;
    headrect.x = window_width / 2 - headrect.w / 2;
    headrect.y = window_height / 2 - headrect.h / 2 - 12;
    //身体位
    SDL_Rect bodyrect;
    SDL_QueryTexture(BackMotions[0], NULL, NULL, &bodyrect.w, &bodyrect.h);
    bodyrect.w *= 3;
    bodyrect.h *= 3;
    bodyrect.x = window_width / 2 - bodyrect.w / 2;
    bodyrect.y = window_height / 2 - bodyrect.h / 2 + 12;

    PLAYER isaac(bodyrect.x, bodyrect.y, headrect.w, headrect.h + bodyrect.h, 6, 2, 3.5, 3, 1, 6);

    while (!isquit) {

        // 处理输入
        processInput(event, isquit, keyStates, head_direction, headrect, bodyrect, isaac);

        // 更新子弹位置
        updateBullets(Bullets, window_width, window_height);

        // 渲染
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, basement, NULL, NULL);

        // 更新角色位置和动画
        updatePlayerPosition(headrect, bodyrect, keyStates, window_width, window_height, bodyDirection);
        updatePlayerMotion(renderer, BackMotions, FrontMotions, RightMotions, LeftMotions, BackHeadMotions,
            FrontHeadMotions, RightHeadMotions, LeftHeadMotions, headrect, bodyrect, keyStates, bodyDirection);

        // 渲染子弹
        renderBullets(renderer, Bullets, bulletTexture);

        //刷新画布     
        SDL_RenderPresent(renderer);

        // 控制帧率
        SDL_Delay(16); // 约 60 FPS
    }

    Mix_FreeMusic(main_msuic);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


