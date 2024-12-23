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
#include "main.h"
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



/* —————————— 视频 —————————— */
void playVideo(const char* videoPath, SDL_Renderer* renderer) {
    avformat_network_init();

    AVFormatContext* formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, videoPath, NULL, NULL) != 0) {
        SDL_Log("Failed to open video file: %s", videoPath);
        return;
    }

    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        SDL_Log("Failed to retrieve stream info from video file: %s", videoPath);
        return;
    }

    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        SDL_Log("Failed to find video stream in file: %s", videoPath);
        return;
    }

    AVCodecParameters* videoCodecParameters = formatContext->streams[videoStreamIndex]->codecpar;
    const AVCodec* videoCodec = avcodec_find_decoder(videoCodecParameters->codec_id);
    if (!videoCodec) {
        SDL_Log("Failed to find codec for video file: %s", videoPath);
        return;
    }

    AVCodecContext* videoCodecContext = avcodec_alloc_context3(videoCodec);
    if (avcodec_parameters_to_context(videoCodecContext, videoCodecParameters) < 0) {
        SDL_Log("Failed to copy codec parameters to codec context for video file: %s", videoPath);
        return;
    }

    if (avcodec_open2(videoCodecContext, videoCodec, NULL) < 0) {
        SDL_Log("Failed to open codec for video file: %s", videoPath);
        return;
    }

    AVFrame* frame = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();
    struct SwsContext* swsContext = sws_getContext(
        videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,
        videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, NULL, NULL, NULL);

    SDL_Texture* texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING,
        videoCodecContext->width, videoCodecContext->height);

    SDL_Rect videoRect;
    videoRect.x = 0;
    videoRect.y = 0;
    videoRect.w = videoCodecContext->width;
    videoRect.h = videoCodecContext->height;

    bool isPlaying = true;
    SDL_Event event;
    while (isPlaying && av_read_frame(formatContext, packet) >= 0) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isPlaying = false;
            }
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                isPlaying = false;
            }
        }

        if (packet->stream_index == videoStreamIndex) {
            if (avcodec_send_packet(videoCodecContext, packet) >= 0) {
                while (avcodec_receive_frame(videoCodecContext, frame) >= 0) {
                    uint8_t* data[3] = { frame->data[0], frame->data[1], frame->data[2] };
                    int linesize[3] = { frame->linesize[0], frame->linesize[1], frame->linesize[2] };
                    SDL_UpdateYUVTexture(texture, &videoRect, data[0], linesize[0], data[1], linesize[1], data[2], linesize[2]);

                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, texture, NULL, &videoRect);
                    SDL_RenderPresent(renderer);
                }
            }
        }
        av_packet_unref(packet);
        SDL_Delay(16); // 控制帧率
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&videoCodecContext);
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    SDL_DestroyTexture(texture);
}



/* —————————— 声音 —————————— */
Mix_Music* main_music = NULL;
Mix_Music* opening_video_sound = NULL;
void playOpeningVideoSound()
{
    opening_video_sound = Mix_LoadMUS("ISAAC/Sounds/openingvideo.wav");
}
void playMainMusic()
{
    main_music = Mix_LoadMUS("ISAAC/Sounds/burning basement.mp3");
}



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
    double damage;//怪物伤害
    SDL_Rect bumpbox;//碰撞箱
	State state;//怪物状态
	bool isAlive = true;//怪物是否存活
    //怪物行为函数
	void move(const PLAYER& player) {//怪物移动
        switch (move_type) {
        case 1://追踪玩家移动，且速度不变
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
        case 2://追踪玩家移动，且越接近玩家速度越慢
            if (x < player.x) {
                x += speed / distance(x, y, player.x, player.y);
            }
            else if (x > player.x) {
                x -= speed / distance(x, y, player.x, player.y);
            }
            if (y < player.y) {
                y += speed / distance(x, y, player.x, player.y);
            }
            else if (y > player.y) {
                y -= speed / distance(x, y, player.x, player.y);
            }
            break;
        case 3://在一定范围内随机移动
            srand((unsigned)time(NULL));
            int random = rand() % 4;
            switch (random) {
            case 0:
                x += speed;
                break;
            case 1:
                x -= speed;
                break;
            case 2:
                y += speed;
                break;
            case 3:
                y -= speed;
                break;
            }
            break;
        }
	}//怪物移动
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
    Uint32 lastMoveTime;  // 上次移动的时间
    Uint32 moveInterval;  // 移动间隔
    int currentFrame;     // 当前动画帧
    Uint32 lastFrameTime; // 上次动画帧时间
    Uint32 attackStartTime; // 攻击开始时间
    bool isReadyToAttack; // 是否准备好再次攻击

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
    }
    FLY* createFly(int x, int y) {
        FLY* fly = new FLY(x, y);
        return fly;
    }
    void detectState(const PLAYER& player) { // 检测玩家是否在攻击范围内
        if (distance(x, y, player.x, player.y) <= 400 && isReadyToAttack && isReadyToAttack == true) {
            state = ATTACK;
        }
        else {
            state = IDLE;
        }
    }
    void updateMoveType(const PLAYER& player) { // 更新移动状态
        if (distance(x, y, player.x, player.y) <= 400)move_type = 2;
        else move_type = 2; 
    }
    void shoot(const PLAYER& player) { // 发射子弹
        Uint32 currentTime = SDL_GetTicks();
        if (isReadyToAttack) {
            double angle = atan2(player.y - y, player.x - x);
            FlyBullets.push_back(BULLET(x, y, 5, damage, 300, { angle }));
            lastShootTime = currentTime; // 更新上次发射时间
            attackStartTime = currentTime; // 记录攻击开始时间
            isReadyToAttack = false; // 设置为不准备攻击状态
        }
    }
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
            case 2: // 追踪玩家移动，且越接近玩家速度越慢
                if (x < player.x) {
                    x += speed / distance(x, y, player.x, player.y);
                }
                else if (x > player.x) {
                    x -= speed / distance(x, y, player.x, player.y);
                }
                if (y < player.y) {
                    y += speed / distance(x, y, player.x, player.y);
                }
                else if (y > player.y) {
                    y -= speed / distance(x, y, player.x, player.y);
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
void flyAttackMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyMotions, SDL_Rect& flyrect, FLY& fly) {
    Uint32 currentTime = SDL_GetTicks();
    // 每50ms切换帧
    if (currentTime - fly.lastFrameTime >= 50) {
        fly.currentFrame = (fly.currentFrame + 1) % 16;
        fly.lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, FlyMotions[fly.currentFrame], NULL, &flyrect);

    // 检查动画是否播放完毕
    if (currentTime - fly.attackStartTime >= 800) { // 假设攻击动画持续800ms
        fly.state = IDLE; // 设置为常态
        fly.lastShootTime = currentTime; // 更新上次发射时间
        fly.isReadyToAttack = false; // 设置为不准备攻击状态
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

/* —————————————————————————————— */


/* 切换房间 */
void switchRoom(SDL_Renderer* renderer, SDL_Texture* newRoomTexture, SDL_Rect& headrect, SDL_Rect& bodyrect) {
    // 渲染新房间背景
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, newRoomTexture, NULL, NULL);
    SDL_RenderPresent(renderer);

    // 重置角色位置
    headrect.x = window_width / 2 - headrect.w / 2;
    headrect.y = window_height / 2 - headrect.h / 2 - 12;
    bodyrect.x = window_width / 2 - bodyrect.w / 2;
    bodyrect.y = window_height / 2 - bodyrect.h / 2 + 12;

    // 清空当前子弹
    Bullets.clear();
    FlyBullets.clear();

    // 随机生成新的怪物
    Flies.clear();
    srand(time(NULL));
    for (int i = 0; i < 10; i++) {
        FLY fly(rand() % window_width, rand() % window_height);
        Flies.push_back(fly);
    }
}




/* —————————— 事件处理和画面渲染 —————————— */

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
//处理射击事件
void processShooting(bool keyStates[8], Direction& head_direction, SDL_Rect& headrect,
    PLAYER& isaac, Uint32& last_shoot_time, bool& is_attacking, Uint32& attack_start_time) {
    Uint32 current_time = SDL_GetTicks(); // 当前时间
    double attack_interval = 1000.0 / isaac.tear; // 攻击间隔

    // 处理射击并触发攻击动画
    if (keyStates[4] && current_time - last_shoot_time >= attack_interval) { // 上
        head_direction = UP;
        Bullets.push_back(BULLET(isaac.x, isaac.y - 30, isaac.shootspeed, isaac.damage, isaac.range, { PI * 3 / 2 }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time; // 记录攻击开始时间
    }
    if (keyStates[5] && current_time - last_shoot_time >= attack_interval) { // 左
        head_direction = LEFT;
        Bullets.push_back(BULLET(isaac.x, isaac.y - 30, isaac.shootspeed, isaac.damage, isaac.range, { PI }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time;
    }
    if (keyStates[6] && current_time - last_shoot_time >= attack_interval) { // 下
        head_direction = DOWN;
        Bullets.push_back(BULLET(isaac.x, isaac.y - 30, isaac.shootspeed, isaac.damage, isaac.range, { PI / 2 }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time;
    }
    if (keyStates[7] && current_time - last_shoot_time >= attack_interval) { // 右
        head_direction = RIGHT;
        Bullets.push_back(BULLET(isaac.x, isaac.y - 30, isaac.shootspeed, isaac.damage, isaac.range, { 0 }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time;
    }

    // 检查动画是否播放完毕
    if (is_attacking && current_time - attack_start_time >= attack_interval) {
        is_attacking = false; // 动画结束后停止攻击状态
    }
}




// 更新角色位置和方向
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
    isaac.x = headrect.x + headrect.w / 2 - 14;
    isaac.y = bodyrect.y + bodyrect.h / 2;
}
// 更新角色动画
void updatePlayerMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BackMotions, vector<SDL_Texture*>& FrontMotions,
    vector<SDL_Texture*>& RightMotions, vector<SDL_Texture*>& LeftMotions, vector<SDL_Texture*>& BackHeadMotions,
    vector<SDL_Texture*>& FrontHeadMotions, vector<SDL_Texture*>& RightHeadMotions, vector<SDL_Texture*>& LeftHeadMotions,
    SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[8], int bodyDirection, bool& is_attacking) {

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


// 更新人物子弹位置
void updateBullets(vector<BULLET>& bullets, int window_width, int window_height, vector<SDL_Texture*>& BurstMotions) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        it->update();

        // 检查子弹是否碰到墙壁（窗口边界），如果碰到则爆裂
        if (!it->isBursting && (it->bumpbox.x < 0 || it->bumpbox.x > window_width - it->bumpbox.w ||
            it->bumpbox.y < 0 || it->bumpbox.y > window_height - it->bumpbox.h)) {
            it->burst();
        }

        // 这里可以添加子弹与怪物的碰撞检测，示例代码如下：
        /*
        for (auto& monster : monsters) {
            if (it->isCollide(monster.bumpbox)) {
                it->burst();
                monster.HP -= it->damage;
                if (monster.HP <= 0) {
                    monster.die();
                }
                break;
            }
        }
        */

        // 如果子弹正在播放爆裂动画，检查动画是否结束
        if (it->isBursting) {
            Uint32 currentTime = SDL_GetTicks();
            // 假设爆裂动画持续时间为 `burstDuration`，根据动画帧数和帧间隔计算
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


// 更新怪物位置
void updateMonsters(vector<FLY>& flies, const PLAYER& player, int window_width, int window_height) {
    for (auto& fly : flies) {
        fly.detectState(player);
        fly.updateMoveType(player);
        fly.move(player);
        if (fly.state == ATTACK) {
            fly.shoot(player);
        }
        // 检查是否达到攻击间隔
        Uint32 currentTime = SDL_GetTicks();
        if (!fly.isReadyToAttack && currentTime - fly.lastShootTime >= fly.shootInterval) {
            fly.isReadyToAttack = true; // 设置为准备攻击状态
        }
    }
}









//渲染画面
void renderScene(SDL_Renderer* renderer, const vector<BULLET>& bullets, SDL_Texture* bulletTexture,
    SDL_Rect& headrect, SDL_Rect& bodyrect, vector<SDL_Texture*>& BackMotions, vector<SDL_Texture*>& FrontMotions,
    vector<SDL_Texture*>& RightMotions, vector<SDL_Texture*>& LeftMotions, vector<SDL_Texture*>& BackHeadMotions,
    vector<SDL_Texture*>& FrontHeadMotions, vector<SDL_Texture*>& RightHeadMotions, vector<SDL_Texture*>& LeftHeadMotions,
    const bool keyStates[8], int bodyDirection, bool& is_attacking, vector<SDL_Texture*>& BurstMotions, const vector<FLY>& flies,
    vector<SDL_Texture*>& FlyMotions, SDL_Texture* enemy_bullet_texture, vector<SDL_Texture*>& EnemyBurstMotions) {
    // 子弹向上时先渲染子弹，再渲染角色
    for (const auto& bullet : bullets) {
        if (bullet.direction.radian == 3 * PI / 2 && bullet.bumpbox.y + bullet.bumpbox.h < headrect.y + headrect.h) {
            renderBullets(renderer, { bullet }, bulletTexture, BurstMotions);
        }
    }
    // 更新角色动画
    updatePlayerMotion(renderer, BackMotions, FrontMotions, RightMotions, LeftMotions, BackHeadMotions,
        FrontHeadMotions, RightHeadMotions, LeftHeadMotions, headrect, bodyrect, keyStates, bodyDirection, is_attacking);
    // 其他方向的子弹渲染（不受遮挡）
    for (const auto& bullet : bullets) {
        if (bullet.direction.radian != 3 * PI / 2 || bullet.bumpbox.y + bullet.bumpbox.h >= headrect.y + headrect.h) {
            renderBullets(renderer, { bullet }, bulletTexture, BurstMotions);
        }
    }
    // 渲染苍蝇怪
    for (auto& fly : flies) {
        SDL_Rect flyrect = { static_cast<int>(fly.x), static_cast<int>(fly.y), 96, 96 };
        if (fly.state == ATTACK) {
            flyAttackMotion(renderer, FlyMotions, flyrect, const_cast<FLY&>(fly));
        }
        else {
            flyIdleMotion(renderer, FlyMotions, flyrect, const_cast<FLY&>(fly));
        }
    }
    // 渲染怪物子弹
    renderBullets(renderer, FlyBullets, enemy_bullet_texture, EnemyBurstMotions);
}





/* —————————————————————————————— */









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

    //初始化音乐与音效
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }

    //播放开场视频和音频
    playOpeningVideoSound();
    if (opening_video_sound == NULL) {
        SDL_Log("Failed to load beat music! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }
    Mix_PlayMusic(opening_video_sound, -1);
    playVideo("ISAAC/video/openingvideo.mp4", renderer);
    Mix_FreeMusic(opening_video_sound);

    //播放音乐
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
    PLAYER isaac(bodyrect.x, bodyrect.y, headrect.w, headrect.h + bodyrect.h, 6, 2, 3.5, 3, 7, 500); // 创建角色

    //随机生成苍蝇怪
    srand(time(NULL));
    for (int i = 0; i < 10; i++) {
        FLY fly(rand() % window_width, rand() % window_height);
        Flies.push_back(fly);
    }

    Uint32 switch_start_time = 0; // 切换房间开始时间
    bool switching_room = false; // 是否正在切换房间
    bool black_screen = false; // 是否处于黑屏状态

    while (!isquit) {
        // 处理事件
        processInput(event, isquit, keyStates);

        // 每帧处理射击
        processShooting(keyStates, head_direction, headrect, isaac, last_shoot_time, is_attacking, attack_start_time);

        // 更新子弹位置
        updateBullets(Bullets, window_width, window_height, BurstMotions);
        updateBullets(FlyBullets, window_width, window_height, EnemyBurstMotions); // 更新怪物子弹位置

        // 更新怪物位置
        updateMonsters(Flies, isaac, window_width, window_height);

        // 渲染背景
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, black, NULL, NULL);
        SDL_RenderCopy(renderer, basement, NULL, NULL);


        // 更新角色位置
        updatePlayerPosition(headrect, bodyrect, keyStates, window_width, window_height, bodyDirection, isaac);

        // 检测 bodyrect 是否进入右下角 100x100 像素区域
        if (bodyrect.x + bodyrect.w > window_width - 100 && bodyrect.y + bodyrect.h > window_height - 100) {
            if (!switching_room) {
                switchRoom(renderer, black, headrect, bodyrect);
                switch_start_time = SDL_GetTicks();
                switching_room = true;
                black_screen = true;
            }
        }

        // 检查是否达到延迟时间
        if (switching_room && SDL_GetTicks() - switch_start_time >= 2000) { // 延长到 5 秒
            switchRoom(renderer, basement, headrect, bodyrect);
            switching_room = false;
            black_screen = false;
        }

        // 渲染背景
        SDL_RenderClear(renderer);
        if (black_screen) {
            SDL_RenderCopy(renderer, black, NULL, NULL);
        }
        else {
            SDL_RenderCopy(renderer, basement, NULL, NULL);
        }

        // 渲染画面
        if (!black_screen) {
            renderScene(renderer, Bullets, bullet_texture, headrect, bodyrect, BackMotions, FrontMotions,
                RightMotions, LeftMotions, BackHeadMotions, FrontHeadMotions, RightHeadMotions, LeftHeadMotions,
                keyStates, bodyDirection, is_attacking, BurstMotions, Flies, FlyMotions, enemy_bullet_texture, EnemyBurstMotions);
        }

        // 刷新画布     
        SDL_RenderPresent(renderer);

        // 控制帧率
        SDL_Delay(16); // 约 60 FPS
    }

    Mix_FreeMusic(main_music);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}



