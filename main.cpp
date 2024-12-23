//�����»��ߣ�����С�շ壬������շ壬��ͽṹ��ȫ��д
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


//ö�ٷ���
enum Direction { UP, DOWN, LEFT, RIGHT };
Direction head_direction = DOWN; // Ĭ�ϳ�����
// ��ӷ������
int bodyDirection = 0; // Ĭ�Ͼ�ֹ
// ����ṹ��
struct DIRECTION {
    double radian;
};


/* �������������������� ��Ƶ �������������������� */
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
        SDL_Delay(16); // ����֡��
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&videoCodecContext);
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    SDL_DestroyTexture(texture);
}


/* �������������������� ���� �������������������� */
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


/* �������������������� ��ɫ �������������������� */
class PLAYER {
public:
    //��ɫ�����������
    double x;//��ɫ����
    double y;//��ɫ����
    double HP;//��ɫѪ��
    double speed;//��ɫ�ٶ�
    double damage;//��ɫ�˺�
    double tear;//��ɫ����
    double attack_interval;// ���������ʱ�䵥λΪ���룩
    double shootspeed;//��ɫ����
    double range;//��ɫ���
    SDL_Rect bumpbox;//��ײ��
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




/* �������������������� �ӵ� �������������������� */
//�ӵ���
class BULLET {
public:
    //�ӵ������������
    int x;//�ӵ�����
    int y;//�ӵ�����
    double speed;//�ӵ��ٶ�
    double damage;//�ӵ��˺�
    double range;//�ӵ����
    SDL_Rect bumpbox;//��ײ��
    DIRECTION direction;//�ӵ����з���
    bool isAlive = true;//�ӵ��Ƿ���
    bool isBursting = false;//�ӵ��Ƿ����ڱ���
    Uint32 burstStartTime = 0;//���Ѷ�����ʼʱ��

    //�ӵ���Ϊ����
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
        if (!isBursting) {
            bumpbox.x += speed * cos(direction.radian);
            bumpbox.y += speed * sin(direction.radian);
            x = bumpbox.x;
            y = bumpbox.y;
            range -= speed;
            if (range <= 0) {
                burst(); // ����̺ľ�����������
            }
        }
    }

    void update() {
        if (isAlive) {
            updatePosition();
        }
    }

    //�ӵ���ײ���
    bool isCollide(SDL_Rect rect) {
        if (bumpbox.x + bumpbox.w < rect.x || bumpbox.x > rect.x + rect.w ||
            bumpbox.y + bumpbox.h < rect.y || bumpbox.y > rect.y + rect.h) {
            return false;
        }
        return true;
    }

    //�ӵ�����
    void burst() {
        isBursting = true;
        burstStartTime = SDL_GetTicks();
    }

    //�ӵ�����
    void die() {
        isAlive = false;
    }
};
vector<BULLET> Bullets;





/* �������������������� ���� �������������������� */
//�������
class MONSTER {
public:
    //��������������
    int id;//����id
    char name[20];//��������
    double x;//��������
    double y;//��������
    double HP;//����Ѫ��
    int move_type;//�����ƶ���ʽ
    double speed;//�����ٶ�
    int attack_type;//���﹥����ʽ
    double damage;//�����˺�
    SDL_Rect bumpbox;//��ײ��
    //������Ϊ����
    void move() {
        switch (move_type) {
        case 1://ֱ���ƶ�
            break;
        case 2://����ƶ�
            break;
        case 3://׷���ƶ�
            break;
        }
    }
    void attack() {
        switch (attack_type) {
        case 1://��ս����
            break;
        case 2://Զ�̹���
            break;
        }
    }
    void die() {
        //��������
    }



};
//��Ӭ��
class FLY :public MONSTER {
public:
	int state = 0;//����״̬
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
vector<FLY> Flies;
//֩���
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
		attack_type = 1;
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
// �ӵ����Ѷ���
void bulletBurstMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BurstMotions, SDL_Rect& bulletrect) {
	static int currentFrame = 0;
	static Uint32 lastFrameTime = 0;
	Uint32 currentTime = SDL_GetTicks();
	// ÿ10ms�л�֡�����°���ʱ����
	if (currentTime - lastFrameTime >= 10 || currentFrame == 0) {
		currentFrame = (currentFrame + 1) % BurstMotions.size();
		lastFrameTime = currentTime;
	}
	SDL_RenderCopy(renderer, BurstMotions[currentFrame], NULL, &bulletrect);
}

/* ������������������������������������������������������������ */




// ���尴��״̬���飬����ΪW, A, S, D, UP, LEFT, DOWN, RIGHT
bool keyStates[8] = { false, false, false, false, false, false, false, false };


//ÿ֡�����ƶ��¼�
void processInput(SDL_Event& event, bool& isquit, bool keyStates[8]) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isquit = true;
        }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                // �ƶ�����
            case SDLK_w: keyStates[0] = true; break;
            case SDLK_a: keyStates[1] = true; break;
            case SDLK_s: keyStates[2] = true; break;
            case SDLK_d: keyStates[3] = true; break;
                // ��������
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
//��������¼�
void processShooting(bool keyStates[8], Direction& head_direction, SDL_Rect& headrect,
    PLAYER& isaac, Uint32& last_shoot_time, bool& is_attacking, Uint32& attack_start_time) {
    Uint32 current_time = SDL_GetTicks(); // ��ǰʱ��
    double attack_interval = 1000.0 / isaac.tear; // �������

    // ���������������������
    if (keyStates[4] && current_time - last_shoot_time >= attack_interval) { // ��
        head_direction = UP;
        Bullets.push_back(BULLET(headrect.x + headrect.w / 2 - 14, headrect.y + headrect.h / 2 - 15,
            isaac.shootspeed, isaac.damage, isaac.range, { PI * 3 / 2 }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time; // ��¼������ʼʱ��
    }
    if (keyStates[5] && current_time - last_shoot_time >= attack_interval) { // ��
        head_direction = LEFT;
        Bullets.push_back(BULLET(headrect.x + headrect.w / 2 - 32, headrect.y + headrect.h / 2 - 10,
            isaac.shootspeed, isaac.damage, isaac.range, { PI }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time;
    }
    if (keyStates[6] && current_time - last_shoot_time >= attack_interval) { // ��
        head_direction = DOWN;
        Bullets.push_back(BULLET(headrect.x + headrect.w / 2 - 14, headrect.y + headrect.h / 2,
            isaac.shootspeed, isaac.damage, isaac.range, { PI / 2 }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time;
    }
    if (keyStates[7] && current_time - last_shoot_time >= attack_interval) { // ��
        head_direction = RIGHT;
        Bullets.push_back(BULLET(headrect.x + headrect.w / 2 + 5, headrect.y + headrect.h / 2 - 10,
            isaac.shootspeed, isaac.damage, isaac.range, { 0 }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time;
    }

    // ��鶯���Ƿ񲥷����
    if (is_attacking && current_time - attack_start_time >= attack_interval) {
        is_attacking = false; // ����������ֹͣ����״̬
    }
}


// ���½�ɫλ�úͷ���
void updatePlayerPosition(SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[4], int window_width,
    int window_height, int& bodyDirection, PLAYER isaac) {
    if (keyStates[0] && headrect.y > 0) {
        headrect.y -= 2.5 * isaac.speed;
        bodyrect.y -= 2.5 * isaac.speed;
        bodyDirection = 1; // ��
    }
    else if (keyStates[1] && headrect.x > 0) {
        headrect.x -= 2.5 * isaac.speed;
        bodyrect.x -= 2.5 * isaac.speed;
        bodyDirection = 3; // ��
    }
    else if (keyStates[2] && bodyrect.y < window_height - bodyrect.h) {
        headrect.y += 2.5 * isaac.speed;
        bodyrect.y += 2.5 * isaac.speed;
        bodyDirection = 2; // ��
    }
    else if (keyStates[3] && headrect.x < window_width - headrect.w) {
        headrect.x += 2.5 * isaac.speed;
        bodyrect.x += 2.5 * isaac.speed;
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
    SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[8], int bodyDirection, bool& is_attacking) {

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


// �����ӵ�λ��
void updateBullets(vector<BULLET>& bullets, int window_width, int window_height, vector<SDL_Texture*>& BurstMotions) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        it->update();

        // ����ӵ��Ƿ�����ǽ�ڣ����ڱ߽磩�������������
        if (!it->isBursting && (it->bumpbox.x < 0 || it->bumpbox.x > window_width - it->bumpbox.w ||
            it->bumpbox.y < 0 || it->bumpbox.y > window_height - it->bumpbox.h)) {
            it->burst();
        }

        // �����������ӵ���������ײ��⣬ʾ���������£�
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

        // ����ӵ����ڲ��ű��Ѷ�������鶯���Ƿ����
        if (it->isBursting) {
            Uint32 currentTime = SDL_GetTicks();
            // ���豬�Ѷ�������ʱ��Ϊ `burstDuration`�����ݶ���֡����֡�������
            Uint32 burstDuration = BurstMotions.size() * 50; // ÿ֡50ms
            if (currentTime - it->burstStartTime >= burstDuration) {
                // ����������ɾ���ӵ�
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
// �ӵ���Ⱦ
void renderBullets(SDL_Renderer* renderer, const vector<BULLET>& bullets, SDL_Texture* bulletTexture,
                   vector<SDL_Texture*>& BurstMotions) {
    for (const auto& bullet : bullets) {
        if (bullet.isBursting) {
            // ���㵱ǰӦ��ʾ�ı��Ѷ���֡
            Uint32 currentTime = SDL_GetTicks();
            Uint32 elapsedTime = currentTime - bullet.burstStartTime;
            int frame = (elapsedTime / 100) % BurstMotions.size(); // ÿ֡100ms
            SDL_RenderCopy(renderer, BurstMotions[frame], NULL, &bullet.bumpbox);
        }
        else {
            SDL_RenderCopy(renderer, bulletTexture, NULL, &bullet.bumpbox);
        }
    }
}


//��Ⱦ����
void renderScene(SDL_Renderer* renderer, const vector<BULLET>& bullets, SDL_Texture* bulletTexture,
    SDL_Rect& headrect, SDL_Rect& bodyrect, vector<SDL_Texture*>& BackMotions, vector<SDL_Texture*>& FrontMotions,
    vector<SDL_Texture*>& RightMotions, vector<SDL_Texture*>& LeftMotions, vector<SDL_Texture*>& BackHeadMotions,
    vector<SDL_Texture*>& FrontHeadMotions, vector<SDL_Texture*>& RightHeadMotions, vector<SDL_Texture*>& LeftHeadMotions,
    const bool keyStates[8], int bodyDirection, bool& is_attacking, vector<SDL_Texture*>& BurstMotions) {
    // �ӵ�����ʱ����Ⱦ�ӵ�������Ⱦ��ɫ
    for (const auto& bullet : bullets) {
        if (bullet.direction.radian == 3 * PI / 2 && bullet.bumpbox.y + bullet.bumpbox.h < headrect.y + headrect.h) {
            renderBullets(renderer, { bullet }, bulletTexture, BurstMotions);
        }
    }
    // ���½�ɫ����
    updatePlayerMotion(renderer, BackMotions, FrontMotions, RightMotions, LeftMotions, BackHeadMotions,
        FrontHeadMotions, RightHeadMotions, LeftHeadMotions, headrect, bodyrect, keyStates, bodyDirection, is_attacking);
    // ����������ӵ���Ⱦ�������ڵ���
    for (const auto& bullet : bullets) {
        if (bullet.direction.radian != 3 * PI / 2 || bullet.bumpbox.y + bullet.bumpbox.h >= headrect.y + headrect.h) {
            renderBullets(renderer, { bullet }, bulletTexture, BurstMotions);
        }
    }
}








/* �������������������� ������ �������������������� */

int main(int, char**) {
    // ��ʼ��SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    //��ʼ�� SDL ��Ƶ��ϵͳ
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) {
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
    const int window_width = 1368;
    const int window_height = 768;
    int x = screen_rect.w / 2 - window_width / 2;
    int y = screen_rect.h / 2 - window_height / 2;
    SDL_Window* window = SDL_CreateWindow("The Binding of Isaac", x, y,
        window_width, window_height, SDL_WINDOW_SHOWN);

    // ������Ⱦ
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //��ʼ����������Ч
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }

    //���ſ�����Ƶ����Ƶ
    playOpeningVideoSound();
    if (opening_video_sound == NULL) {
        SDL_Log("Failed to load beat music! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }
    Mix_PlayMusic(opening_video_sound, -1);
    playVideo("ISAAC/video/openingvideo.mp4", renderer);
    Mix_FreeMusic(opening_video_sound);

    //��������
    playMainMusic();
    if (main_music == NULL) {
        SDL_Log("Failed to load beat music! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }
    Mix_PlayMusic(main_music, -1);

    /* �������� */
    SDL_Texture* basement = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/basement.png");
    SDL_Texture* bulletTexture = IMG_LoadTexture(renderer, "ISAAC/Characters/bullet.png");

    /* ��ʼ�������������� */
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


    Uint32 last_shoot_time = SDL_GetTicks(); // �ϴ����ʱ��
	bool is_attacking = false; // �Ƿ����ڹ���
	Uint32 attack_start_time = 0; // ������ʼʱ��
    PLAYER isaac(bodyrect.x, bodyrect.y, headrect.w, headrect.h + bodyrect.h, 6, 2, 3.5, 3, 7, 500);

    while (!isquit) {

		// �����¼�
        processInput(event, isquit, keyStates);

        // ÿ֡�������
        processShooting(keyStates, head_direction, headrect, isaac, last_shoot_time, is_attacking, attack_start_time);

        // �����ӵ�λ��
        updateBullets(Bullets, window_width, window_height, BurstMotions);

        // ��Ⱦ����
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, basement, NULL, NULL);

        // ���½�ɫλ��
        updatePlayerPosition(headrect, bodyrect, keyStates, window_width, window_height, bodyDirection,isaac);
        
        // ��Ⱦ����
		renderScene(renderer, Bullets, bulletTexture, headrect, bodyrect, BackMotions, FrontMotions,
			RightMotions, LeftMotions, BackHeadMotions, FrontHeadMotions, RightHeadMotions, LeftHeadMotions,
			keyStates, bodyDirection, is_attacking, BurstMotions);

        // ˢ�»���     
        SDL_RenderPresent(renderer);
        
        // ����֡��
        SDL_Delay(16); // Լ 60 FPS
    }

    Mix_FreeMusic(main_music);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}