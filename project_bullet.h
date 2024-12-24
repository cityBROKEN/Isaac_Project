#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <random>
#include "project_monster.h"
using namespace std;
#define PI 3.14159265358979323846




/* —————————— 子弹类定义 —————————— */
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
    bool isAlive = true;//子弹是否存活
    bool isBursting = false;//子弹是否正在爆裂
    Uint32 burstStartTime = 0;//爆裂动画开始时间
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

    //————子弹行为函数

    //子弹更新位置
    void updatePosition() {
        if (!isBursting) {
            bumpbox.x += speed * cos(direction.radian);
            bumpbox.y += speed * sin(direction.radian);
            x = bumpbox.x;
            y = bumpbox.y;
            range -= speed;
            if (range <= 0) {
                burst(); // 当射程耗尽，触发爆裂
            }
        }
    }

    //子弹更新
    void update() {
        if (isAlive) {
            updatePosition();
        }
    }

    //子弹碰撞检测
    bool isCollide(SDL_Rect rect) {
        if (bumpbox.x + bumpbox.w < rect.x || bumpbox.x > rect.x + rect.w ||
            bumpbox.y + bumpbox.h < rect.y || bumpbox.y > rect.y + rect.h) {
            return false;
        }
        return true;
    }

    //子弹爆裂
    void burst() {
        isBursting = true;
        burstStartTime = SDL_GetTicks();
    }

    //子弹死亡
    void die() {
        isAlive = false;
    }
};
vector<BULLET> Bullets;//角色子弹容器
vector<BULLET> FlyBullets;//苍蝇怪子弹容器




// 人物泪弹爆裂动画
void bulletBurstMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BurstMotions, SDL_Rect& bulletrect) {
    static int currentFrame = 0;
    static Uint32 lastFrameTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    // 每10ms切换帧，按下按键时重置
    if (currentTime - lastFrameTime >= 10 || currentFrame == 0) {
        currentFrame = (currentFrame + 1) % BurstMotions.size();
        lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, BurstMotions[currentFrame], NULL, &bulletrect);
}





/* —————————— 子弹事件处理和动画渲染 —————————— */

//处理射击事件
void processShooting(bool keyStates[8], Direction& head_direction, SDL_Rect& headrect,
    PLAYER& isaac, Uint32& last_shoot_time, bool& is_attacking, Uint32& attack_start_time) {
    Uint32 current_time = SDL_GetTicks(); // 当前时间
    double attack_interval = 1000.0 / isaac.tear; // 攻击间隔

    // 处理射击并触发攻击动画
    if (keyStates[4] && current_time - last_shoot_time >= attack_interval) { // 上
        head_direction = UP;
        Bullets.push_back(BULLET(isaac.x + 30, isaac.y, isaac.shootspeed, isaac.damage, isaac.range, { PI * 3 / 2 }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time; // 记录攻击开始时间
    }
    if (keyStates[5] && current_time - last_shoot_time >= attack_interval) { // 左
        head_direction = LEFT;
        Bullets.push_back(BULLET(isaac.x + 30, isaac.y, isaac.shootspeed, isaac.damage, isaac.range, { PI }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time;
    }
    if (keyStates[6] && current_time - last_shoot_time >= attack_interval) { // 下
        head_direction = DOWN;
        Bullets.push_back(BULLET(isaac.x + 30, isaac.y, isaac.shootspeed, isaac.damage, isaac.range, { PI / 2 }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time;
    }
    if (keyStates[7] && current_time - last_shoot_time >= attack_interval) { // 右
        head_direction = RIGHT;
        Bullets.push_back(BULLET(isaac.x + 30, isaac.y, isaac.shootspeed, isaac.damage, isaac.range, { 0 }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time;
    }

    // 检查动画是否播放完毕
    if (is_attacking && current_time - attack_start_time >= attack_interval) {
        is_attacking = false; // 动画结束后停止攻击状态
    }
}

// 更新子弹位置，添加一个参数，用于区分是玩家的子弹还是怪物的子弹
void updateBullets(vector<BULLET>& bullets, int window_width, int window_height, vector<SDL_Texture*>& BurstMotions, bool isEnemyBullet, PLAYER& player, vector<FLY>& Flies) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        it->update();

        // 检查子弹是否碰到墙壁（窗口边界），如果碰到则爆裂
        if (!it->isBursting && (it->bumpbox.x < 0 || it->bumpbox.x > window_width - it->bumpbox.w ||
            it->bumpbox.y < 0 || it->bumpbox.y > window_height - it->bumpbox.h)) {
            it->burst();
        }

        // 碰撞检测
        if (!it->isBursting) {
            // 处理敌人子弹
            if (isEnemyBullet) {
                if (it->isCollide(player.bumpbox)) {
                    if (!player.isInvincible) { // 如果不在无敌状态
                        player.HP -= it->damage;
                        player.isInvincible = true; // 开始无敌状态
                        player.invincibleStartTime = SDL_GetTicks();
                    }
                    it->burst();
                }
            }

            else {
                // 玩家子弹，检测是否击中怪物
                for (auto& fly : Flies) {
                    if (it->isCollide(fly.bumpbox)) {
                        it->burst();
                        fly.HP -= it->damage;
                        if (fly.HP <= 0) {
                            fly.die();
                        }
                        break;
                    }
                }
            }
        }

        // 如果子弹正在播放爆裂动画，检查动画是否结束
        if (it->isBursting) {
            Uint32 currentTime = SDL_GetTicks();
            Uint32 burstDuration = BurstMotions.size() * 50; // 每帧50ms
            if (currentTime - it->burstStartTime >= burstDuration) {
                // 动画结束，删除子弹
                it = bullets.erase(it);
                continue;
            }
        }

        if (!it->isAlive) {
            it = bullets.erase(it);
        }
        else {
            ++it;
        }
    }
}

// 人物子弹渲染
void renderBullets(SDL_Renderer* renderer, const vector<BULLET>& bullets, SDL_Texture* bulletTexture,
    vector<SDL_Texture*>& BurstMotions) {
    for (const auto& bullet : bullets) {
        if (bullet.isBursting) {
            // 计算当前应显示的爆裂动画帧
            Uint32 currentTime = SDL_GetTicks();
            Uint32 elapsedTime = currentTime - bullet.burstStartTime;
            int frame = (elapsedTime / 100) % BurstMotions.size(); // 每帧100ms
            SDL_RenderCopy(renderer, BurstMotions[frame], NULL, &bullet.bumpbox);
        }
        else {
            SDL_RenderCopy(renderer, bulletTexture, NULL, &bullet.bumpbox);
        }
    }
}


