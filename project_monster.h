#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <random>
#include "project_player.h"
#include "project_bullet.h"
using namespace std;
#define PI 3.14159265358979323846

//初始化随机数
random_device rd;
mt19937 gen(rd());

/* —————————— 怪物类定义 —————————— */
//枚举状态
enum State { IDLE, ATTACK };
//基类怪物
class MONSTER {
public:
    friend class PLAYER;
    //怪物基本面板属性
    int id;//怪物id
    char name[20];//怪物名字
    double x;//怪物坐标
    double y;//怪物坐标
    double HP;//怪物血量
    int move_type;//怪物移动方式
    double speed;//怪物速度
    double damage;//怪物伤害
    SDL_Rect bumpbox;//碰撞箱
    State state;//怪物状态
    Uint32 lastMoveTime;  // 上次移动的时间
    Uint32 moveInterval;  // 移动间隔
    bool isAlive = true;//怪物是否存活
    //怪物行为函数
    void move(const PLAYER& player) { // 怪物移动
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastMoveTime >= moveInterval) { // 检查是否达到移动间隔
            switch (move_type) {
            case 1: // 追踪玩家移动，且速度不变
                if (x < player.x) {
                    x += speed;
                }
                else if (x > player.x) {
                    x -= speed;
                }
                if (y < player.y) {
                    y += speed;
                }
                else if (y > player.y) {
                    y -= speed;
                }
                break;
            case 2: // 追踪玩家移动，且越靠近玩家速度越慢
            {
                // 计算怪物与玩家的距离
                double dist = distance(x, y, player.x, player.y);
                // 为了防止除以零，设置一个最小距离
                if (dist < 1.0) {
                    dist = 1.0;
                }
                // 计算方向向量，并归一化
                double dx = (player.x - x) / dist;
                double dy = (player.y - y) / dist;
                // 计算移动速度，距离越大速度越快，距离越小速度越慢
                double moveSpeed = speed * (dist / 1000.0); // 100.0 是一个调节系数，可根据需要调整
                // 更新怪物位置
                x += dx * moveSpeed;
                y += dy * moveSpeed;
            }
            break;

            case 3: // 在一定范围内随机移动
                uniform_real_distribution<> dis(0.0, 1.0);
                double random_element = dis(gen);
                double random_radian = random_element * 2 * PI;
                x += 0.5 * speed * cos(random_radian);
                y += 0.5 * speed * sin(random_radian);
                break;
            }
            lastMoveTime = currentTime; // 更新上次移动时间
        }
    }
    void die() {//怪物死亡
        isAlive = false;
    }
};
//苍蝇怪
class FLY : public MONSTER {
public:
    friend class PLAYER;
    Uint32 lastShootTime; // 上次发射子弹的时间
    Uint32 shootInterval; // 攻击间隔
    int currentFrame;     // 当前动画帧
    Uint32 lastFrameTime; // 上次动画帧时间
    Uint32 attackStartTime; // 攻击开始时间
    bool isReadyToAttack; // 是否准备好再次攻击
    bool hasShot; // 是否已经发射过子弹
    SDL_Rect bumpbox;//碰撞箱
    FLY(int x, int y) {
        id = 1;
        strcpy_s(name, "FLY");
        this->x = x;
        this->y = y;
        HP = 10;
        move_type = 1;
        speed = 3;
        damage = 1;
        lastShootTime = 0;
        shootInterval = 2000; // 设置攻击间隔为2000毫秒（2秒）
        lastMoveTime = SDL_GetTicks(); // 初始化为当前时间
        moveInterval = 16 + rand() % 16; // 设置移动间隔为16-32毫秒之间的随机值
        currentFrame = 0;
        lastFrameTime = SDL_GetTicks();
        attackStartTime = 0;
        isReadyToAttack = true;
        hasShot = false;
        // 初始化碰撞箱
        this->bumpbox.w = 96; // 图像宽度
        this->bumpbox.h = 96; // 图像高度
        this->bumpbox.x = x + (96 - 40) / 2;
        this->bumpbox.y = y + (96 - 40) / 2;
    }
    FLY* createFly(int x, int y) {
        FLY* fly = new FLY(x, y);
        return fly;
    }
    void detectState(const PLAYER& player) { // 检测玩家是否在攻击范围内
        if (state == ATTACK) {
            // 如果正在攻击，不改变状态
            return;
        }
        if (distance(x, y, player.x, player.y) <= 400 && isReadyToAttack) {
            state = ATTACK;
        }
        else {
            state = IDLE;
        }
    }
    void updateMoveType(const PLAYER& player) { // 更新移动状态
        if (distance(x, y, player.x, player.y) <= 600)move_type = 2;
        else move_type = 3;
    }
    void shoot(const PLAYER& player) { // 发射子弹
        Uint32 currentTime = SDL_GetTicks();
        if (isReadyToAttack) {
            double angle = atan2(player.y - y, player.x - x);
            FlyBullets.push_back(BULLET(x + 35, y + 50, 5, damage, 300, { angle }));
            lastShootTime = currentTime; // 更新上次发射时间
            attackStartTime = currentTime; // 记录攻击开始时间
            isReadyToAttack = false; // 设置为不准备攻击状态
        }
    }
    void move(const PLAYER& player) { // 移动
        detectState(player); // 检测玩家是否在攻击范围内
        updateMoveType(player); // 更新移动状态
        MONSTER::move(player); // 调用基类的移动函数
        // 更新碰撞箱的位置
        this->bumpbox.x = static_cast<int>(this->x) + 8;
        this->bumpbox.y = static_cast<int>(this->y) + 8;
    }

};

vector<FLY> Flies;

//蜘蛛怪
class SPIDER :public MONSTER {
public:
    SPIDER() {
        id = 2;
        strcpy_s(name, "SPIDER");
        x = 0;
        y = 0;
        HP = 20;
        move_type = 2;
        speed = 3;
        damage = 2;
    }
    SPIDER* createSpider(int x, int y) {
        SPIDER* spider = new SPIDER();
        spider->x = x;
        spider->y = y;
        return spider;
    }
};
vector<SPIDER> Spiders;





/* —————————— 动画 —————————— */


// 苍蝇怪常态动画
void flyIdleMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyMotions, SDL_Rect& flyrect, FLY& fly) {
    Uint32 currentTime = SDL_GetTicks();
    // 每100ms切换帧
    if (currentTime - fly.lastFrameTime >= 100) {
        fly.currentFrame = (fly.currentFrame + 1) % 2;
        fly.lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, FlyMotions[fly.currentFrame], NULL, &flyrect);
}
// 苍蝇怪攻击动画
void flyAttackMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyMotions, SDL_Rect& flyrect, FLY& fly, const PLAYER& player) {
    Uint32 currentTime = SDL_GetTicks();
    // 每50ms切换帧
    if (currentTime - fly.lastFrameTime >= 40) {
        fly.currentFrame = (fly.currentFrame + 1) % 16;
        fly.lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, FlyMotions[fly.currentFrame], NULL, &flyrect);

    // 当动画帧为第9帧时发射子弹
    if (fly.currentFrame == 8 && !fly.hasShot) {
        fly.shoot(player);
        fly.hasShot = true; // 设置已发射标志，防止重复发射
    }

    // 检查动画是否播放完毕
    if (currentTime - fly.attackStartTime >= 440) { // 假设攻击动画持续800ms
        fly.state = IDLE; // 设置为常态
        fly.isReadyToAttack = false; // 设置为不准备攻击状态
        fly.hasShot = false; // 重置已发射标志
    }
}
// 苍蝇怪死亡动画
void flyDeadMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyDeadMotions, SDL_Rect& flyrect) {
    static int currentFrame = 0;
    static Uint32 lastFrameTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    // 每200ms切换帧，按下按键时重置
    if (currentTime - lastFrameTime >= 200 || currentFrame == 0) {
        currentFrame = (currentFrame + 1) % FlyDeadMotions.size();
        lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, FlyDeadMotions[currentFrame], NULL, &flyrect);
}







/* —————————— 状态更新和画面渲染 —————————— */

// 更新怪物位置在，删除死亡的怪物
void updateMonsters(vector<FLY>& flies, PLAYER& player, int window_width, int window_height) {
    for (auto it = flies.begin(); it != flies.end();) {
        if (!it->isAlive) {
            it = flies.erase(it);
        }
        else {
            it->detectState(player);
            it->updateMoveType(player);
            it->move(player);

            // 检查是否达到攻击间隔
            Uint32 currentTime = SDL_GetTicks();
            if (!it->isReadyToAttack && currentTime - it->lastShootTime >= it->shootInterval) {
                it->isReadyToAttack = true; // 设置为准备攻击状态
            }

            // 碰撞检测
            if (SDL_HasIntersection(&player.bumpbox, &it->bumpbox)) {
                if (!player.isInvincible) { // 如果不在无敌状态
                    player.HP -= it->damage;
                    player.isInvincible = true; // 开始无敌状态
                    player.invincibleStartTime = SDL_GetTicks(); // 记录无敌开始时间
                }
                // 可选：添加角色受伤的反馈，如播放受伤音效
            }

            ++it;
        }
    }
}