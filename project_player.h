#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
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
double distance(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}



/* �������������������� ��ɫ�ඨ�� �������������������� */
class PLAYER {
public:
    //��ɫ�����������
    int x;//��ɫ����
    int y;//��ɫ����
    int HP;//��ɫѪ��
    double speed;//��ɫ�ٶ�
    double damage;//��ɫ�˺�
    double tear;//��ɫ����
    double attack_interval;// ���������ʱ�䵥λΪ���룩
    double shootspeed;//��ɫ����
    double range;//��ɫ���
    SDL_Rect bumpbox;//��ײ��
    // �޵�״̬��ر���
    bool isInvincible;          // �Ƿ����޵�״̬
    Uint32 invincibleStartTime; // �޵п�ʼʱ��
    Uint32 invincibleDuration;  // �޵г���ʱ�䣨���룩
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
        this->invincibleDuration = 1000; // �޵г���ʱ��Ϊ1�룬�ɸ�����Ҫ����
    }
};










/* �������������������� ��ɫ��ض��� �������������������� */

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
// �����������涯��
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
// �����������涯��
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
// �����������涯��
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
// ���������������
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
// ���������������
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
// ���������������
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
// ���������������
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




/* �������������������� �����¼�����Ͷ�����Ⱦ �������������������� */

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

// ���½�ɫλ��
void updatePlayerPosition(SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[4], int window_width,
    int window_height, int& bodyDirection, PLAYER& isaac) {
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
    // ���� isaac �����ĵ�����
    isaac.x = headrect.x + headrect.w / 2 - 45;
    isaac.y = headrect.y + (headrect.h + bodyrect.h) / 2 - 65;
    // ���½�ɫ����ײ��
    isaac.bumpbox.x = headrect.x + 15;
    isaac.bumpbox.y = headrect.y + 10;
    isaac.bumpbox.w = headrect.w - 30;
    isaac.bumpbox.h = headrect.h - 10;
    // ����ѭ����ר�ŵĸ��º�����
    Uint32 currentTime = SDL_GetTicks();
    if (isaac.isInvincible && currentTime - isaac.invincibleStartTime >= isaac.invincibleDuration) {
        isaac.isInvincible = false; // �����޵�״̬
    }

}

// ���½�ɫ����
void updatePlayerMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BackMotions, vector<SDL_Texture*>& FrontMotions,
    vector<SDL_Texture*>& RightMotions, vector<SDL_Texture*>& LeftMotions, vector<SDL_Texture*>& BackHeadMotions,
    vector<SDL_Texture*>& FrontHeadMotions, vector<SDL_Texture*>& RightHeadMotions, vector<SDL_Texture*>& LeftHeadMotions,
    SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[8], int bodyDirection, bool& is_attacking, PLAYER& player) {
    // ����Ⱦ��ɫ֮ǰ������Ƿ����޵�״̬
    if (player.isInvincible) {
        // ������˸Ч��������ÿ�� 100ms �л�һ�οɼ���
        Uint32 currentTime = SDL_GetTicks();
        if ((currentTime / 100) % 2 == 0) {
            // ��ʱ����Ⱦ��ɫ��ʵ����˸Ч��
            return;
        }
    }
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

// ����Ѫ����Ⱦ
void renderPlayerHealth(SDL_Renderer* renderer, const PLAYER& player, SDL_Texture* full_heart, SDL_Texture* half_heart, SDL_Texture* empty_heart) {
    int maxHearts = 6; // �����������Ϊ6
    int heartWidth = 30;
    int heartHeight = 30;

    for (int i = 0; i < maxHearts; ++i) {
        SDL_Rect heartRect = { 10 + i * (heartWidth + 5), 10, heartWidth, heartHeight };
        if (player.HP >= (i + 1) * 2) {
            // ��������
            SDL_RenderCopy(renderer, full_heart, NULL, &heartRect);
        }
        else if (player.HP == (i * 2) + 1) {
            // ���ư���
            SDL_RenderCopy(renderer, half_heart, NULL, &heartRect);
        }
        else {
            // ���ƿ���
            SDL_RenderCopy(renderer, empty_heart, NULL, &heartRect);
        }
    }
}



