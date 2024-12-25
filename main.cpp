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
#include <cstdlib>
#include <random>
#include <map>
#include <algorithm>
#include < fstream >
#include "font.h"
#include "media.h"
#ifdef __cplusplus
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#endif
#ifdef __cplusplus
}
#endif
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
//初始化随机数
random_device rd;
mt19937 gen(rd());
//窗口大小
const int window_width = 1368;
const int window_height = 768;
//游戏状态
bool isPause = false;

/* —————————— 障碍物 —————————— */
class OBSTACLE {
public:
    int x, y; // 障碍物坐标
    int w, h; // 障碍物宽度和高度
    SDL_Rect rect; // 障碍物的矩形区域
    SDL_Texture* texture; // 障碍物的纹理
	SDL_Rect collisionRect; // 障碍物的碰撞箱
    OBSTACLE(int x, int y, int w, int h, SDL_Texture* texture) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
        this->rect = { x, y, w, h };
        this->texture = texture;
        // 初始化碰撞箱，使其比实际障碍物小一些
        int collisionMargin = 20; // 碰撞箱缩小的边距
        this->collisionRect = { x + collisionMargin, y + collisionMargin, w - 2 * collisionMargin, h - 2 * collisionMargin };
    
    }
};
vector<OBSTACLE> Obstacles; // 障碍物容器



/* —————————— 角色 —————————— */
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
	bool isAlive = true;//角色是否存活
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
		this->isAlive = true;
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
	SDL_Rect spriteRect;//贴图矩形
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
        this->direction = direction;
        this->isAlive = true;
        this->isBursting = false;
        this->burstStartTime = 0;

        // 初始化贴图矩形
        this->spriteRect.w = 30; // 贴图宽度
        this->spriteRect.h = 30; // 贴图高度
        this->spriteRect.x = x;
        this->spriteRect.y = y;

        // 初始化碰撞箱的位置和大小
        this->bumpbox.w = 10; // 碰撞箱宽度
        this->bumpbox.h = 10; // 碰撞箱高度
        this->bumpbox.x = x + (this->spriteRect.w - this->bumpbox.w) / 2;
        this->bumpbox.y = y + (this->spriteRect.h - this->bumpbox.h) / 2;
    }

    //————子弹行为函数

    //子弹更新位置
    void updatePosition() {
        if (!isBursting) {
            double deltaX = speed * cos(direction.radian);
            double deltaY = speed * sin(direction.radian);

            // 更新位置
            x += deltaX;
            y += deltaY;

            // 更新贴图的位置
            spriteRect.x = static_cast<int>(x);
            spriteRect.y = static_cast<int>(y);

            // 更新碰撞箱的位置
            bumpbox.x = static_cast<int>(x) + (spriteRect.w - bumpbox.w) / 2;
            bumpbox.y = static_cast<int>(y) + (spriteRect.h - bumpbox.h) / 2;

            range -= speed;
            if (range <= 0) {
                burst();
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
        //生成随机数1或2
        uniform_int_distribution<int> disw(1, 2); // 定义随机数范围为[1, 7]
        int random_number = disw(gen); // 生成随机数
		if (random_number == 1) {
            playShootSound1();
            Mix_PlayChannel(-1, shoot_sound1, 0);

        }
		else {
            playShootSound2();
            Mix_PlayChannel(-1, shoot_sound2, 0);
        }
	}

    //子弹死亡
    void die() {
        isAlive = false;
    }
};
vector<BULLET> Bullets;//角色子弹容器
vector<BULLET> FlyBullets;//苍蝇怪子弹容器



/* —————————— 随机数 —————————— */
int randomNumberOneToSeven() {
    std::random_device rd;  // 创建随机数种子
    std::mt19937 gen(rd()); // 生成随机数引擎，使用梅森旋转算法
    std::uniform_int_distribution<> dis(1, 7); // 定义随机数范围为[1, 7]

    int random_number = dis(gen); // 生成随机数
	return random_number;
}



/* —————————— 怪物 —————————— */
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
    double vx; // x轴速度分量
    double vy; // y轴速度分量
    double damage;//怪物伤害
    SDL_Rect bumpbox;//碰撞箱
    SDL_Rect spriteRect;//贴图矩形
    State state;//怪物状态
    Uint32 lastMoveTime;  // 上次移动的时间
    Uint32 moveInterval;  // 移动间隔
    bool isAlive = true;//怪物是否存活
	bool isDying = false; // 是否正在死亡
    Uint32 currentFrame;     // 当前动画帧
    Uint32 lastFrameTime; // 上次动画帧时间
    Uint32 deathStartTime = 0;      // 记录死亡动画开始时间
    Uint32 deathDuration = 550;     // 死亡动画持续时间（毫秒）
    // 怪物行为函数
	// 怪物移动
    void move(const PLAYER& player) {
        // 计算怪物与玩家的距离和方向
        double dx = player.x - x;
        double dy = player.y - y;
        double dist = sqrt(dx * dx + dy * dy);

        // 归一化方向向量
        double dirX = dx / dist;
        double dirY = dy / dist;

        // 根据移动类型调整怪物的速度
        switch (move_type) {
        case 1:
            // 追踪玩家，速度不变
            vx = dirX * speed;
            vy = dirY * speed;
            break;
        case 2:
            // 越靠近玩家速度越慢
            if (dist > 0) {
                double moveSpeed = speed * (dist / 2500.0); // 调整 500.0 为合适的比例
                vx = dirX * moveSpeed;
                vy = dirY * moveSpeed;
            }
            break;
        case 3:
            // 随机移动
        {
            uniform_real_distribution<> dis(-1.0, 1.0);
            vx += dis(gen) * speed * 0.1;
            vy += dis(gen) * speed * 0.1;
            // 限制速度
            double maxSpeed = speed;
            double currentSpeed = sqrt(vx * vx + vy * vy);
            if (currentSpeed > maxSpeed) {
                vx = (vx / currentSpeed) * maxSpeed;
                vy = (vy / currentSpeed) * maxSpeed;
            }
        }
        break;
        case 4:
			//45度直线移动，速度不变，碰到边界反弹
			vx = speed;
			vy = speed;
			break;
        }

        // 更新位置
        x += vx;
        y += vy;

        // 应用摩擦力，逐渐减速
        vx *= 0.9;
        vy *= 0.9;

        // 更新碰撞箱和贴图的位置
        bumpbox.x = static_cast<int>(x) + (spriteRect.w - bumpbox.w) / 2;
        bumpbox.y = static_cast<int>(y) + (spriteRect.h - bumpbox.h) / 2;
        spriteRect.x = static_cast<int>(x);
        spriteRect.y = static_cast<int>(y);

        // 边界检测，防止怪物移出屏幕（可选）
        if (x < 120) {
            x = 120;
            vx = -vx * 0.5; // 碰撞反弹效果
        }
        else if (x > window_width - bumpbox.w - 120) {
            x = window_width - bumpbox.w - 120;
            vx = -vx * 0.5;
        }
        if (y < 100) {
            y = 100;
            vy = -vy * 0.5;
        }
        else if (y > window_height - bumpbox.h - 100) {
            y = window_height - bumpbox.h - 100;
            vy = -vy * 0.5;
        }
    }

    // 更新位置的方法
    void updatePosition() {
        // 更新位置
        x += vx;
        y += vy;

        // 应用摩擦力，逐渐减速
        vx *= 0.9; // 摩擦系数，可根据需要调整
        vy *= 0.9;

        // 边界检测，防止怪物移出屏幕
        if (x < 120) {
            x = 120;
            vx = 0;
        }
        else if (x > window_width - bumpbox.w - 120) {
            x = window_width - bumpbox.w - 120;
            vx = 0;
        }
        if (y < 100) {
            y = 100;
            vy = 0;
        }
        else if (y > window_height - bumpbox.h - 100) {
            y = window_height - bumpbox.h - 100;
            vy = 0;
        }

        // 更新碰撞箱和贴图的位置
        bumpbox.x = static_cast<int>(x) + (spriteRect.w - bumpbox.w) / 2;
        bumpbox.y = static_cast<int>(y) + (spriteRect.h - bumpbox.h) / 2;
        spriteRect.x = static_cast<int>(x);
        spriteRect.y = static_cast<int>(y);
    }
    void die() {
        if (!isDying) {
            isDying = true;
            deathStartTime = SDL_GetTicks(); // 记录死亡动画开始时间
            vx = 0; // 停止移动
            vy = 0;
			// 播放死亡音效
            playMonsterDeath();
			Mix_PlayChannel(-1, monster_death, 0);
        }
    }

};

//苍蝇怪
class FLY : public MONSTER {
public:
    friend class PLAYER;
    Uint32 lastShootTime; // 上次发射子弹的时间
    Uint32 shootInterval; // 攻击间隔
    Uint32 attackStartTime; // 攻击开始时间
    bool isReadyToAttack; // 是否准备好再次攻击
    bool hasShot; // 是否已经发射过子弹    

	
    FLY(int x, int y) {
        id = 1;
        strcpy_s(name, "FLY");
        this->x = x;
        this->y = y;
        HP = 10;
        move_type = 1;
        speed = 3;
        vx = 0;
        vy = 0;
        damage = 1;
        lastShootTime = 0;
        shootInterval = 2000; // 设置攻击间隔为2000毫秒（2秒）
        lastMoveTime = SDL_GetTicks();
        moveInterval = 16 + rand() % 16;
        currentFrame = 0;
        lastFrameTime = SDL_GetTicks();
        attackStartTime = 0;
        isReadyToAttack = true;
        hasShot = false;
		state = IDLE;
		isAlive = true;
		isDying = false;
		deathStartTime = 0;
		deathDuration = 550;

        // 初始化贴图矩形
        this->spriteRect.w = 96; // 贴图宽度
        this->spriteRect.h = 96; // 贴图高度
        this->spriteRect.x = x;
        this->spriteRect.y = y;

        // 初始化碰撞箱的位置和大小，可以根据需要调整
        this->bumpbox.w = 40; // 碰撞箱宽度
        this->bumpbox.h = 40; // 碰撞箱高度
        this->bumpbox.x = x + (this->spriteRect.w - this->bumpbox.w) / 2;
        this->bumpbox.y = y + (this->spriteRect.h - this->bumpbox.h) / 2;
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
        // 更新贴图的位置
        this->spriteRect.x = static_cast<int>(this->x);
        this->spriteRect.y = static_cast<int>(this->y);
        // 更新碰撞箱的位置
        this->bumpbox.x = static_cast<int>(this->x) + (this->spriteRect.w - this->bumpbox.w) / 2;
        this->bumpbox.y = static_cast<int>(this->y) + (this->spriteRect.h - this->bumpbox.h) / 2;
    }

    void updateDeathAnimation() {
        // 根据死亡时间计算当前帧（假设死亡动画有 11 帧，每帧持续 deathDuration / 11 毫秒）
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsed = currentTime - deathStartTime;
        int totalFrames = 11;
        int frameDuration = deathDuration / totalFrames;
        currentFrame = elapsed / frameDuration;
        if (currentFrame >= totalFrames) {
            currentFrame = totalFrames - 1;
        }
    }

};

vector<FLY> Flies;
FLY fly_standard(100, 100);

//大苍蝇怪
class BIGFLY : public MONSTER {
public:
    friend class PLAYER;
    BIGFLY(int x, int y) {
        id = 2;
        strcpy_s(name, "BIGFLY");
        this->x = x;
        this->y = y;
        HP = 20;
        move_type = 4;
        speed = 4;
        vx = 0;
        vy = 0;
        damage = 2;
        lastMoveTime = SDL_GetTicks();
        moveInterval = 16;
        currentFrame = 0;
        lastFrameTime = SDL_GetTicks();
        state = IDLE;
        isAlive = true;
        isDying = false;
        deathStartTime = 0;
        deathDuration = 550;
        // 初始化贴图矩形
        this->spriteRect.w = 90; // 贴图宽度
        this->spriteRect.h = 90; // 贴图高度
        this->spriteRect.x = x;
        this->spriteRect.y = y;
        // 初始化碰撞箱的位置和大小，可以根据需要调整
        this->bumpbox.w = 50; // 碰撞箱宽度
        this->bumpbox.h = 50; // 碰撞箱高度
        this->bumpbox.x = x + (this->spriteRect.w - this->bumpbox.w) / 2;
        this->bumpbox.y = y + (this->spriteRect.h - this->bumpbox.h) / 2;
        // 45度直线移动，速度不变，碰到边界反弹
        vx = speed * cos(PI / 4);
        vy = speed * sin(PI / 4);
    }
	BIGFLY* createBigFly(int x, int y) {
		BIGFLY* bigfly = new BIGFLY(x, y);
		return bigfly;
	}
    void move() {

		// 更新位置
        x += vx;
        y += vy;

        // 边界检测，防止怪物移出屏幕，碰到边界反弹
		if (x < 120 || x > window_width - bumpbox.w - 120) {//碰到左右边界
			vx = -vx;
        }
        if (y < 100 || y > window_height - bumpbox.h - 100) {
			vy = -vy;
        }

        // 更新碰撞箱和贴图的位置
        bumpbox.x = static_cast<int>(x) + (spriteRect.w - bumpbox.w) / 2;
        bumpbox.y = static_cast<int>(y) + (spriteRect.h - bumpbox.h) / 2;
        spriteRect.x = static_cast<int>(x);
        spriteRect.y = static_cast<int>(y);
    }

    void die() {
        if (!isDying) {
            isDying = true;
            deathStartTime = SDL_GetTicks(); // 记录死亡动画开始时间
            vx = 0; // 停止移动
            vy = 0;
            // 死亡后向六个方向发射子弹
            double angle = 0;
            for (int i = 0; i < 6; i++) {
                // 修改这里，将子弹添加到 FlyBullets 而不是 Bullets
                FlyBullets.push_back(BULLET(x + 45, y + 45, 5, damage, 300, { angle }));
                angle += PI / 3;
            }
            // 播放死亡音效
            playMonsterDeath();
            Mix_PlayChannel(-1, monster_death, 0);
        }
    }

	void updateDeathAnimation() {
		// 根据死亡时间计算当前帧（假设死亡动画有 11 帧，每帧持续 deathDuration / 11 毫秒）
		Uint32 currentTime = SDL_GetTicks();
		Uint32 elapsed = currentTime - deathStartTime;
		int totalFrames = 11;
		int frameDuration = deathDuration / totalFrames;
		currentFrame = elapsed / frameDuration;
		if (currentFrame >= totalFrames) {
			currentFrame = totalFrames - 1;
		}
	}
};

vector<BIGFLY> BigFlies;
BIGFLY bigfly_standard(100, 100);






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
// 苍蝇怪常态动画
void flyIdleMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyMotions, vector<SDL_Texture*>& _FlyMotions, SDL_Rect& flyrect, FLY& fly , const PLAYER& player) {
    if (player.x >= fly.x) {
        Uint32 currentTime = SDL_GetTicks();
        // 每100ms切换帧
        if (currentTime - fly.lastFrameTime >= 100) {
            fly.currentFrame = (fly.currentFrame + 1) % 2;
            fly.lastFrameTime = currentTime;
        }
        SDL_RenderCopy(renderer, FlyMotions[fly.currentFrame], NULL, &flyrect);
    }
	else {
		Uint32 currentTime = SDL_GetTicks();
		// 每100ms切换帧
		if (currentTime - fly.lastFrameTime >= 100) {
			fly.currentFrame = (fly.currentFrame + 1) % 2;
			fly.lastFrameTime = currentTime;
		}
		SDL_RenderCopy(renderer, _FlyMotions[fly.currentFrame], NULL, &flyrect);
	}
}
// 苍蝇怪攻击动画
void flyAttackMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyMotions, vector<SDL_Texture*>& _FlyMotions, SDL_Rect& flyrect, FLY& fly, const PLAYER& player) {
    if (player.x >= fly.x) {
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
    else {
        Uint32 currentTime = SDL_GetTicks();
        // 每50ms切换帧
        if (currentTime - fly.lastFrameTime >= 40) {
            fly.currentFrame = (fly.currentFrame + 1) % 16;
            fly.lastFrameTime = currentTime;
        }
        SDL_RenderCopy(renderer, _FlyMotions[fly.currentFrame], NULL, &flyrect);
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
}
// 苍蝇怪死亡动画
void flyDeadMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyDeadMotions, SDL_Rect& flyrect, FLY& fly) {
    // 确保帧数不超过动画总帧数
    int totalFrames = FlyDeadMotions.size();
    if (fly.currentFrame >= totalFrames) {
        fly.currentFrame = totalFrames - 1;
    }
    SDL_RenderCopy(renderer, FlyDeadMotions[fly.currentFrame], NULL, &flyrect);
}
// 大苍蝇怪常态动画
void bigFlyMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BigFlyMotions, SDL_Rect& bigflyrect, BIGFLY& bigfly) {
	Uint32 currentTime = SDL_GetTicks();
	// 每80ms切换帧
	if (currentTime - bigfly.lastFrameTime >= 80) {
		bigfly.currentFrame = (bigfly.currentFrame + 1) % 2;
		bigfly.lastFrameTime = currentTime;
	}
	SDL_RenderCopy(renderer, BigFlyMotions[bigfly.currentFrame], NULL, &bigflyrect);
}
// 大苍蝇怪死亡动画
void bigFlyDeadMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BigFlyDeadMotions, SDL_Rect& bigflyrect, BIGFLY& bigfly) {
	// 确保帧数不超过动画总帧数
	int totalFrames = BigFlyDeadMotions.size();
	if (bigfly.currentFrame >= totalFrames) {
		bigfly.currentFrame = totalFrames - 1;
	}
	SDL_RenderCopy(renderer, BigFlyDeadMotions[bigfly.currentFrame], NULL, &bigflyrect);
}

/* —————————————————————————————— */










int room_number = 1;
// 随机刷怪函数
void generateMonsters(int room_number, PLAYER& player) {
    uniform_int_distribution<int> Xdis(150, window_width - 150);
    uniform_int_distribution<int> Ydis(120, window_height - 120);
    uniform_real_distribution<double> monsterTypeDis(0.0, 1.0); // 用于随机决定怪物类型

    for (int i = 0; i < room_number; i++) {
        int x = Xdis(gen);
        int y = Ydis(gen);

        // 确保生成的怪物不在玩家附近
        while (abs(x - player.x) < 200 && abs(y - player.y) < 200) {
            x = Xdis(gen);
            y = Ydis(gen);
        }

        double monsterType = monsterTypeDis(gen); // 生成0到1之间的随机数

        if (monsterType < 0.7) {
            // 70%的概率生成苍蝇怪
            FLY fly(x, y);
            Flies.push_back(fly);
        }
        else {
            // 30%的概率生成大苍蝇怪
            BIGFLY bigfly(x, y);
            BigFlies.push_back(bigfly);
        }
    }
}
// 切换房间 
void switchRoom(SDL_Renderer* renderer, SDL_Texture* newRoomTexture, SDL_Rect& headrect, SDL_Rect& bodyrect, PLAYER player, FLY fly) {
    // 渲染新房间背景
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, newRoomTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
    // 重置角色位置
    headrect.x = window_width / 2 - headrect.w / 2;
    headrect.y = window_height / 2 - headrect.h / 2 + 180;
    bodyrect.x = window_width / 2 - bodyrect.w / 2;
    bodyrect.y = window_height / 2 - bodyrect.h / 2 + 205;
    // 清空当前子弹
    Bullets.clear();
    FlyBullets.clear();
    // 清除障碍物
    Obstacles.clear();
}
// 生成障碍物
void generateObstacles(int numObstacles, SDL_Texture* obstacleTexture) {
    uniform_int_distribution<> Xdis(150, window_width - 250);
    uniform_int_distribution<> Ydis(120, window_height - 170);
    // 可以根据纹理的实际尺寸设置固定的宽高
    int obstacleWidth = 100; // 纹理宽度
    int obstacleHeight = 100; // 纹理高度
    for (int i = 0; i < numObstacles; ++i) {
        int x = Xdis(gen);
        int y = Ydis(gen);
        Obstacles.push_back(OBSTACLE(x, y, obstacleWidth, obstacleHeight, obstacleTexture));
        //SDL_Log("Obstacle generated at position (%d, %d)", x, y); // 输出障碍物位置
    }
}













/* —————————— 事件处理和画面渲染 —————————— */

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
				// 暂停按键
            case SDLK_ESCAPE: isPause = true; break;
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



// 更新角色位置
void updatePlayerPosition(SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[4], int window_width,
    int window_height, int& bodyDirection, PLAYER& isaac) {
    // 保存原始位置
    int originalX = headrect.x;
    int originalY = headrect.y;
    int originalBodyX = bodyrect.x;
    int originalBodyY = bodyrect.y;

    // 尝试更新位置
    if (keyStates[0] && headrect.y > 50) {
        headrect.y -= 2.5 * isaac.speed;
        bodyrect.y -= 2.5 * isaac.speed;
        bodyDirection = 1; // 上
    }
    else if (keyStates[1] && headrect.x > 120) {
        headrect.x -= 2.5 * isaac.speed;
        bodyrect.x -= 2.5 * isaac.speed;
        bodyDirection = 3; // 左
    }
    else if (keyStates[2] && bodyrect.y < window_height - bodyrect.h - 130) {
        headrect.y += 2.5 * isaac.speed;
        bodyrect.y += 2.5 * isaac.speed;
        bodyDirection = 2; // 下
    }
    else if (keyStates[3] && headrect.x < window_width - headrect.w - 130) {
        headrect.x += 2.5 * isaac.speed;
        bodyrect.x += 2.5 * isaac.speed;
        bodyDirection = 4; // 右
    }
    else {
        bodyDirection = 0; // 静止
    }

    // 更新角色的碰撞箱
    isaac.x = headrect.x + headrect.w / 2 - 45;
    isaac.y = headrect.y + (headrect.h + bodyrect.h) / 2 - 65;
    isaac.bumpbox.x = headrect.x + 15;
    isaac.bumpbox.y = headrect.y + 10;
    isaac.bumpbox.w = headrect.w - 30;
    isaac.bumpbox.h = headrect.h - 10;

    // 检测与障碍物的碰撞
    bool collided = false;
    for (const auto& obstacle : Obstacles) {
        if (SDL_HasIntersection(&isaac.bumpbox, &obstacle.collisionRect)) {
            collided = true;
            break;
        }
    }

    // 如果发生碰撞，恢复原始位置
    if (collided) {
        headrect.x = originalX;
        headrect.y = originalY;
        bodyrect.x = originalBodyX;
        bodyrect.y = originalBodyY;

        isaac.x = headrect.x + headrect.w / 2 - 45;
        isaac.y = headrect.y + (headrect.h + bodyrect.h) / 2 - 65;
        isaac.bumpbox.x = headrect.x + 15;
        isaac.bumpbox.y = headrect.y + 10;
    }

    // 更新无敌状态的计时
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
// 在 updateBullets 函数中，添加一个参数，用于区分是玩家的子弹还是怪物的子弹
void updateBullets(vector<BULLET>& bullets, int window_width, int window_height, vector<SDL_Texture*>& BurstMotions, bool isEnemyBullet, PLAYER& player) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        it->update();

        // 检查子弹是否碰到墙壁或障碍物，如果碰到则爆裂
        if (!it->isBursting) {
            // 检查墙壁
            if (it->bumpbox.x < 120 || it->bumpbox.x > window_width - it->bumpbox.w - 120 ||
                it->bumpbox.y < 90 || it->bumpbox.y > window_height - it->bumpbox.h - 100) {
                it->burst();
            }

            // 检查障碍物
            for (const auto& obstacle : Obstacles) {
                if (SDL_HasIntersection(&it->bumpbox, &obstacle.collisionRect)) {
                    it->burst();
                    break;
                }
            }
        }

        // 碰撞检测
        if (!it->isBursting) {
            if (isEnemyBullet) {
                // 处理敌人子弹
                if (it->isCollide(player.bumpbox)) {
                    if (!player.isInvincible) { // 如果不在无敌状态
                        player.HP -= it->damage;
						playHurtGrunt();
						Mix_PlayChannel(-1, hurt_grunt, 0);//播放受伤音效
                        player.isInvincible = true; // 开始无敌状态
                        player.invincibleStartTime = SDL_GetTicks();
                    }
                    it->burst();
                }
            }
            else {
                // 玩家子弹，检测是否击中小苍蝇怪物
                for (auto& fly : Flies) {
                    if (it->isCollide(fly.bumpbox)) {
                        it->burst();
                        fly.HP -= it->damage;
                        if (fly.HP <= 0) {
                            fly.die();
                        }
                        // 计算击退效果
                        double knockbackStrength = 5.0; // 击退力度，可根据需要调整
                        fly.vx += knockbackStrength * cos(it->direction.radian);
                        fly.vy += knockbackStrength * sin(it->direction.radian);
                        break;
                    }
                }
                // 玩家子弹，检测是否击中大苍蝇怪物
                for (auto& bigFly : BigFlies) {
                    if (it->isCollide(bigFly.bumpbox)) {
                        it->burst();
                        bigFly.HP -= it->damage;
                        if (bigFly.HP <= 0) {
                            bigFly.die();
                        }
                        // 计算击退效果
                        double knockbackStrength = 3.0; // 大苍蝇怪物较重，击退力度可适当调小
                        bigFly.vx += knockbackStrength * cos(it->direction.radian);
                        bigFly.vy += knockbackStrength * sin(it->direction.radian);
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
            int frame = (elapsedTime / 50) % BurstMotions.size(); // 每帧50ms
            SDL_RenderCopy(renderer, BurstMotions[frame], NULL, &bullet.spriteRect);
        }
        else {
            SDL_RenderCopy(renderer, bulletTexture, NULL, &bullet.spriteRect);
        }
    }
}

// 人物血量渲染
void renderPlayerHealth(SDL_Renderer* renderer, const PLAYER& player, SDL_Texture* full_heart, SDL_Texture* half_heart, SDL_Texture* empty_heart) {
    int maxHearts = 6; // 假设最大心数为6
    int heartWidth = 30;
    int heartHeight = 30;

    for (int i = 0; i < maxHearts; ++i) {
        SDL_Rect heartRect = { 10 + i * (heartWidth + 5), 10, 1.5*heartWidth, 1.5*heartHeight };
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




// 更新怪物位置和状态，删除死亡的怪物
void updateMonsters(vector<FLY>& flies, vector<BIGFLY>& bigFlies, PLAYER& player, int window_width, int window_height) {
    // 处理小苍蝇怪物
    for (auto it = flies.begin(); it != flies.end();) {
        if (it->isDying) {
            // 检查死亡动画是否播放完毕
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - it->deathStartTime >= it->deathDuration) {
                // 死亡动画播放完毕，从容器中移除
                it = flies.erase(it);
                continue;
            }
            else {
                // 更新死亡动画帧
                it->updateDeathAnimation();
                ++it;
                continue;
            }
        }
        if (!it->isAlive) {
            it = flies.erase(it);
        }
        else {
            it->detectState(player);
            it->updateMoveType(player);
            it->move(player);

            // 更新位置，应用击退效果
            it->updatePosition();

            // 检查是否达到攻击间隔
            Uint32 currentTime = SDL_GetTicks();
            if (!it->isReadyToAttack && currentTime - it->lastShootTime >= it->shootInterval) {
                it->isReadyToAttack = true; // 设置为准备攻击状态
            }

            // 碰撞检测（小苍蝇怪与玩家）
            if (SDL_HasIntersection(&player.bumpbox, &it->bumpbox)) {
                if (!player.isInvincible) { // 如果不在无敌状态
                    player.HP -= it->damage;
                    playHurtGrunt();//播放受伤音效
                    Mix_PlayChannel(-1, hurt_grunt, 0);
                    player.isInvincible = true; // 开始无敌状态
                    player.invincibleStartTime = SDL_GetTicks(); // 记录无敌开始时间
                }
            }

            ++it;
        }
    }

    // 处理大苍蝇怪物
    for (auto it = bigFlies.begin(); it != bigFlies.end();) {
        if (it->isDying) {
            // 检查死亡动画是否播放完毕
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - it->deathStartTime >= it->deathDuration) {
                // 死亡动画播放完毕，从容器中移除
                it = bigFlies.erase(it);
                continue;
            }
            else {
                // 更新死亡动画帧
                it->updateDeathAnimation();
                ++it;
                continue;
            }
        }
        if (!it->isAlive) {
            it = bigFlies.erase(it);
        }
        else {
            // 更新大苍蝇怪物的位置
            it->move();

            // 碰撞检测（大苍蝇怪与玩家）
            if (SDL_HasIntersection(&player.bumpbox, &it->bumpbox)) {
                if (!player.isInvincible) { // 如果不在无敌状态
                    player.HP -= it->damage;
                    playHurtGrunt();//播放受伤音效
                    Mix_PlayChannel(-1, hurt_grunt, 0);
                    player.isInvincible = true; // 开始无敌状态
                    player.invincibleStartTime = SDL_GetTicks(); // 记录无敌开始时间
                }
            }

            ++it;
        }
    }
}












// 结束界面函数
void renderGameOverScreen(SDL_Renderer* renderer, TTF_Font* font, int window_width, int window_height, SDL_Texture* endmenu) {
    //设置背景为endmenu
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, endmenu, NULL, NULL);

    // 渲染 "Game Over" 文字
	SDL_Color blackColor = { 0, 0, 0 };
    SDL_Surface* gameOverSurface = TTF_RenderText_Solid(font, "Game Over", blackColor);
    SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
    SDL_Rect gameOverRect;
    gameOverRect.w = gameOverSurface->w*4;
    gameOverRect.h = gameOverSurface->h*4;
    gameOverRect.x = (window_width - gameOverRect.w) / 2;
    gameOverRect.y = (window_height - gameOverRect.h) / 2 - 100;
    SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);

    // 渲染 "The number of rooms you passed: " 文字
    string roomPassed = "The number of rooms you passed: " + to_string(room_number - 1);
    SDL_Surface* roomPassedSurface = TTF_RenderText_Solid(font, roomPassed.c_str(), blackColor);
    SDL_Texture* roomPassedTexture = SDL_CreateTextureFromSurface(renderer, roomPassedSurface);
    SDL_Rect roomPassedRect;
    roomPassedRect.w = roomPassedSurface->w;
    roomPassedRect.h = roomPassedSurface->h;
    roomPassedRect.x = (window_width - roomPassedRect.w) / 2;
    roomPassedRect.y = (window_height - roomPassedRect.h) / 2;
    SDL_RenderCopy(renderer, roomPassedTexture, NULL, &roomPassedRect);

    // 渲染 "Press Enter to Restart or Esc to Exit" 文字
    SDL_Surface* exitSurface = TTF_RenderText_Solid(font, "Press Enter to Restart or Esc to Exit", blackColor);
    SDL_Texture* exitTexture = SDL_CreateTextureFromSurface(renderer, exitSurface);
    SDL_Rect exitRect;
    exitRect.w = exitSurface->w;
    exitRect.h = exitSurface->h;
    exitRect.x = (window_width - exitRect.w) / 2;
    exitRect.y = (window_height - exitRect.h) / 2 + 50;
    SDL_RenderCopy(renderer, exitTexture, NULL, &exitRect);

    // 更新屏幕
    SDL_RenderPresent(renderer);

    // 释放资源
    SDL_FreeSurface(gameOverSurface);
    SDL_DestroyTexture(gameOverTexture);
    SDL_FreeSurface(exitSurface);
    SDL_DestroyTexture(exitTexture);
}
//暂停界面函数
void renderPauseScreen(SDL_Renderer* renderer, TTF_Font* font, int window_width, int window_height, SDL_Texture* pausemenu) {
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, pausemenu, NULL, NULL);
	// 更新屏幕
	SDL_RenderPresent(renderer);
}













/* —————————— 主函数 —————————— */

int main(int, char**) {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    //初始化 SDL 音频子系统
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // 初始化 SDL ttf
    if (TTF_Init() == -1) {
        SDL_Log("TTF_Init: %s\n", TTF_GetError());
        return 1;
    }

    // 初始化事件队列
    bool isquit = false;
    SDL_Event event;

    // 获取屏幕大小
    SDL_Rect screen_rect;
    SDL_GetDisplayBounds(0, &screen_rect);

    // 创建窗口
    int x = screen_rect.w / 2 - window_width / 2;
    int y = screen_rect.h / 2 - window_height / 2;
    SDL_Window* window = SDL_CreateWindow("The Binding of Isaac", x, y,
        window_width, window_height, SDL_WINDOW_SHOWN);

    // 创建渲染
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 加载字体
    // // 如果需要加载 ftn 字体，可以按以下方式：
    //if (!loadFont("ISAAC/Fonts/terminus8.fnt", renderer)) {
       // SDL_Log("Failed to load font");
       //return 1;
    //}
    // 如果需要加载 TTF 字体，可以按以下方式：
      if (!loadFont("ISAAC/Fonts/inconsolata-bold.ttf", renderer, 24)) {
           SDL_Log("Failed to load font");
            return 1;
        }

    //初始化音乐与音效
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }
   
	modifyVolume();//调整音量

    //播放开场视频和音频
    playOpeningVideoSound();
    if (opening_video_sound == NULL) {
        SDL_Log("Failed to load beat music! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }
    Mix_PlayMusic(opening_video_sound, -1);
    playVideo("ISAAC/video/openingvideo.mp4", renderer);
    Mix_FreeMusic(opening_video_sound);
    bool startGame = false;




    /*开始界面*/
    //播放音乐
    playTitleScreenSound();
    if (title_screen_sound == NULL) {
        SDL_Log("Failed to load beat music! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }
    Mix_PlayMusic(title_screen_sound, -1);


    // 创建开始界面纹理
    SDL_Texture* start_screen = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/start_screen.png");
    if (start_screen == NULL) {
        SDL_Log("Failed to load start screen texture: %s", IMG_GetError());
        return 1;
    }

    while (!startGame && !isquit) {
        // 处理事件
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isquit = true;
            }
            if (event.type == SDL_KEYDOWN) {
                // 当用户按下回车键时，开始游戏
                if (event.key.keysym.sym == SDLK_RETURN) {
                    startGame = true;
                }
            }
        }

        // 清屏
        SDL_RenderClear(renderer);

        // 渲染开始界面
        SDL_RenderCopy(renderer, start_screen, NULL, NULL);

        SDL_Delay(16); // 控制帧率

        // 渲染文字时，指定颜色
        SDL_Color blackColor = { 0, 0, 0, 0 };
        renderText(renderer, "Press Enter To Start", window_width / 2 , window_height /2, blackColor);



        SDL_RenderPresent(renderer);

        SDL_Delay(16); // 控制帧率
    }


    /*释放开始界面纹理*/
    SDL_DestroyTexture(start_screen);


	// 播放主音乐
    playMainMusic();
    if (main_music == NULL) {
        SDL_Log("Failed to load beat music! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }
    Mix_PlayMusic(main_music, -1);




    /* 创建房间纹理 */
    SDL_Texture* basement = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/basement.png");
    SDL_Texture* black = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/black.png");

    /* 创建纹理 */
    SDL_Texture* bullet_texture = IMG_LoadTexture(renderer, "ISAAC/Characters/bullet.png");
    SDL_Texture* enemy_bullet_texture = IMG_LoadTexture(renderer, "ISAAC/Monsters/enemy_bullet.png");
    SDL_Texture* full_heart = IMG_LoadTexture(renderer, "ISAAC/Characters/full_heart.png");
    SDL_Texture* half_heart = IMG_LoadTexture(renderer, "ISAAC/Characters/half_heart.png");
    SDL_Texture* empty_heart = IMG_LoadTexture(renderer, "ISAAC/Characters/empty_heart.png");
    SDL_Texture* obstacleTexture = IMG_LoadTexture(renderer, "ISAAC/Obstacle/stone.png");
	SDL_Texture* end_menu = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/endmenu.png");
	SDL_Texture* pause_menu = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/pausescreen.png");

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
    vector<SDL_Texture*> BurstMotions;
    for (int i = 1; i <= 15; ++i) {
        string filename = "ISAAC/Characters/Bulletatlas/BurstMotion" + to_string(i) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture) {
            BurstMotions.push_back(texture);
        }
        else {
            SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
        }
    }
    vector<SDL_Texture*> EnemyBurstMotions;
    for (int i = 1; i <= 16; ++i) {
        string filename = "ISAAC/Monsters/EnemyBurstMotion" + to_string(i) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture) {
            EnemyBurstMotions.push_back(texture);
        }
        else {
            SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
        }
    }
    vector<SDL_Texture*> FlyMotions;
    for (int i = 1; i <= 16; ++i) {
        string filename = "ISAAC/Monsters/FLY/FlyMotion" + to_string(i) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture) {
            FlyMotions.push_back(texture);
        }
        else {
            SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
        }
    }
    vector<SDL_Texture*> _FlyMotions;
    for (int i = 1; i <= 16; ++i) {
        string filename = "ISAAC/Monsters/FLY/_FlyMotion" + to_string(i) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture) {
            _FlyMotions.push_back(texture);
        }
        else {
            SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
        }
    }
    vector<SDL_Texture*> FlyDeadMotions;
    for (int i = 1; i <= 11; ++i) {
        string filename = "ISAAC/Monsters/FLY/FlyDeadMotion" + to_string(i) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture) {
            FlyDeadMotions.push_back(texture);
        }
        else {
            SDL_Log("Failed to load texture: %s, Error: %s", filename.c_str(), IMG_GetError());
        }
    }
    vector<SDL_Texture*> BigFlyMotions;
    for (int i = 1; i <= 2; ++i) {
        string filename = "ISAAC/Monsters/BigFly/BigFlyMotion" + to_string(i) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture) {
            BigFlyMotions.push_back(texture);
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

    Uint32 last_shoot_time = SDL_GetTicks(); // 上次射击时间
    bool is_attacking = false; // 是否正在攻击
    Uint32 attack_start_time = 0; // 攻击开始时间
    PLAYER isaac(bodyrect.x, bodyrect.y, headrect.w, headrect.h, 12, 2, 3.5, 3, 7, 500); // 创建角色


  


  


    Uint32 switch_start_time = 0; // 切换房间开始时间
    bool switching_room = false; // 是否正在切换房间
    bool black_screen = false; // 是否处于黑屏状态
	int isClear = 1;// 是否清理完怪物

    while (!isquit) {

		if (isPause) {
			// 暂停音乐
			Mix_PauseMusic();
			// 显示暂停界面
			renderPauseScreen(renderer, ttfFont, window_width, window_height,pause_menu);
			while (isPause) {
				while (SDL_PollEvent(&event)) {
					if (event.type == SDL_QUIT) {
						isquit = true;
                        isPause = false;
					}
					if (event.type == SDL_KEYDOWN) {
						if (event.key.keysym.sym == SDLK_ESCAPE) {
							// 继续游戏
							Mix_ResumeMusic();
                            isPause = false;
						}
					}
				}
			}
		}









        /*结束界面*/
        if (isaac.HP <= 0) {
			// 播放死亡音效
			playDeathGrunt();
            Mix_PlayChannel(-1, death_grunt, 0);
            // 播放死亡界面音乐
            playEndingSound();
            Mix_PlayMusic(ending_sound, -1);
            // 显示结束界面
            renderGameOverScreen(renderer, ttfFont, window_width, window_height,end_menu);
            // 等待用户按下回车键重新开始或按下ESC键退出游戏
            bool gameOver = true;
            while (gameOver) {
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        isquit = true;
                        gameOver = false;
                    }
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_RETURN) {
							playMainMusic();
							Mix_PlayMusic(main_music, -1);
                            // 重新开始游戏
                            isaac.HP = 12; // 重置角色血量
                            isaac.x = window_width / 2; // 重置角色位置
                            isaac.y = window_height / 2;
                            headrect.x = window_width / 2 - headrect.w / 2;
                            headrect.y = window_height / 2 - headrect.h / 2 - 12;
                            bodyrect.x = window_width / 2 - bodyrect.w / 2;
                            bodyrect.y = window_height / 2 - bodyrect.h / 2 + 12;
                            Bullets.clear(); // 清空子弹
                            FlyBullets.clear(); // 清空怪物子弹
                            Flies.clear(); // 清空苍蝇怪
							BigFlies.clear(); // 清空大苍蝇怪
                            room_number = 0; // 重置房间号
							isClear = 1; // 重置是否清理完怪物
                            generateMonsters(room_number, isaac); // 重新生成怪物
                            gameOver = false;
                        }
                        if (event.key.keysym.sym == SDLK_ESCAPE) {
                            isquit = true;
                            gameOver = false;
                        }
                    }
                }
            }
            continue;
        }

        // 处理事件
        processInput(event, isquit, keyStates);

        // 每帧处理射击
        processShooting(keyStates, head_direction, headrect, isaac, last_shoot_time, is_attacking, attack_start_time);

        // 更新玩家子弹
        updateBullets(Bullets, window_width, window_height, BurstMotions, false, isaac);
        // 更新怪物子弹
        updateBullets(FlyBullets, window_width, window_height, EnemyBurstMotions, true, isaac);


        // 更新怪物位置和状态
        updateMonsters(Flies, BigFlies, isaac, window_width, window_height);


        // 渲染背景
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, black, NULL, NULL);
        SDL_RenderCopy(renderer, basement, NULL, NULL);


        // 更新角色位置
        updatePlayerPosition(headrect, bodyrect, keyStates, window_width, window_height, bodyDirection, isaac);


        // 检测 bodyrect 是否进入指定的矩形区域, 并且检测怪物是否清理完毕
        // 添加调试信息
        if ((headrect.x >= window_width / 2 - 50) && (headrect.x+headrect.w <= window_width/2 + 50 ) &&
            (headrect.y >= 0) && (headrect.y <= 50) && Flies.empty()) {
            //SDL_Log("Player entered the target area.");
            if (!switching_room) {
                switchRoom(renderer, black, headrect, bodyrect, isaac, fly_standard);
                switch_start_time = SDL_GetTicks();
                switching_room = true;
                black_screen = true;
            }
        }
           
        // 检查是否达到延迟时间
        if (switching_room && SDL_GetTicks() - switch_start_time >= 1000) { // 延长到 1 秒
            switchRoom(renderer, basement, headrect, bodyrect,isaac, fly_standard);
            switching_room = false;
            black_screen = false;
			int obstacle_number = randomNumberOneToSeven(); // 随机生成1-7个障碍物
            generateObstacles(obstacle_number, obstacleTexture); // 生成1个障碍物
            // 随机生成新的怪物
            generateMonsters(room_number, isaac);
			room_number++;
        }

        

        // 渲染背景
        SDL_RenderClear(renderer);
        if (black_screen) {
            SDL_RenderCopy(renderer, black, NULL, NULL);
        }
        else {
            SDL_RenderCopy(renderer, basement, NULL, NULL);
        }

        // 渲染障碍物
        for (const auto& obstacle : Obstacles) {
            SDL_RenderCopy(renderer, obstacle.texture, NULL, &obstacle.rect);
        }





		// 检查是否清理完怪物，若是则播放胜利音乐
		if (Flies.empty() && BigFlies.empty()) {
            if (isClear == room_number) {
                playClearMusic();
                Mix_PlayChannel(-1, clear_music, 0);
				isClear++;
            }
		}





        /* —————————————— 渲染画面 ——————————————*/

        if (!black_screen) {
            // 绘制血量
            renderPlayerHealth(renderer, isaac, full_heart, half_heart, empty_heart);
            // 子弹向上时先渲染子弹，再渲染角色
            for (const auto& bullet : Bullets) {
                if (bullet.direction.radian == 3 * PI / 2 && bullet.bumpbox.y + bullet.bumpbox.h < headrect.y + headrect.h) {
                    renderBullets(renderer, { bullet }, bullet_texture, BurstMotions);
                }
            }


            //—————————————碰撞箱绘制
            /*
            // 设置绘制颜色为红色
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            // 绘制角色的碰撞箱
            SDL_RenderDrawRect(renderer, &isaac.bumpbox);
            // 绘制苍蝇怪的碰撞箱
            for (const auto& fly : Flies) {
                SDL_RenderDrawRect(renderer, &fly.bumpbox);
            }
			// 绘制大苍蝇怪的碰撞箱
			for (const auto& bigFly : BigFlies) {
				SDL_RenderDrawRect(renderer, &bigFly.bumpbox);
			}
            // 绘制子弹的碰撞箱
            for (const auto& bullet : Bullets) {
                SDL_RenderDrawRect(renderer, &bullet.bumpbox);
            }
            // 绘制障碍物的碰撞箱
            for (const auto& obstacle : Obstacles) {
                SDL_RenderDrawRect(renderer, &obstacle.collisionRect);
            }
            */
			





            // 更新角色动画
            updatePlayerMotion(renderer, BackMotions, FrontMotions, RightMotions, LeftMotions, BackHeadMotions,
                FrontHeadMotions, RightHeadMotions, LeftHeadMotions, headrect, bodyrect, keyStates, bodyDirection, is_attacking, isaac);
            // 其他方向的子弹渲染（不受遮挡）
            for (const auto& bullet : Bullets) {
                if (bullet.direction.radian != 3 * PI / 2 || bullet.bumpbox.y + bullet.bumpbox.h >= headrect.y + headrect.h) {
                    renderBullets(renderer, { bullet }, bullet_texture, BurstMotions);
                }
            }
            // 渲染苍蝇怪
            for (auto& fly : Flies) {
                if (fly.isDying) {
                    // 渲染死亡动画
                    flyDeadMotion(renderer, FlyDeadMotions, fly.spriteRect, fly);
                }
                else if (fly.state == ATTACK) {
                    flyAttackMotion(renderer, FlyMotions, _FlyMotions, fly.spriteRect, fly, isaac);
                }
                else {
                    flyIdleMotion(renderer, FlyMotions, _FlyMotions, fly.spriteRect, fly, isaac);
                }
            }
            // 渲染大苍蝇怪
            for (auto& bigFly : BigFlies) {
				// 渲染死亡动画
                if (bigFly.isDying) {
                    bigFlyDeadMotion(renderer, FlyDeadMotions,bigFly.spriteRect, bigFly);
				}
                else {
                    bigFlyMotion(renderer, BigFlyMotions, bigFly.spriteRect, bigFly);
                }
            }

            // 渲染玩家子弹
            renderBullets(renderer, Bullets, bullet_texture, BurstMotions);
            // 渲染敌方子弹
            renderBullets(renderer, FlyBullets, enemy_bullet_texture, EnemyBurstMotions);


        }

        /* —————————————————————————————— */





        // 刷新画布     
        SDL_RenderPresent(renderer);

        // 控制帧率
        SDL_Delay(16); // 约 60 FPS
    }





    // 清理资源
    if (ttfFont) {
        TTF_CloseFont(ttfFont);
        ttfFont = nullptr;
    }
    if (fontTexture) {
        SDL_DestroyTexture(fontTexture);
        fontTexture = nullptr;
        Characters.clear();
    }
    TTF_Quit();
    Mix_FreeMusic(main_music);
	Mix_FreeMusic(opening_video_sound);
	Mix_FreeChunk(death_grunt);
	Mix_FreeChunk(hurt_grunt);
    Mix_CloseAudio();
    SDL_DestroyTexture(obstacleTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}



