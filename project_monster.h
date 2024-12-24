#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <random>
#include "project_player.h"
#include "project_bullet.h"
using namespace std;
#define PI 3.14159265358979323846

//��ʼ�������
random_device rd;
mt19937 gen(rd());

/* �������������������� �����ඨ�� �������������������� */
//ö��״̬
enum State { IDLE, ATTACK };
//�������
class MONSTER {
public:
    friend class PLAYER;
    //��������������
    int id;//����id
    char name[20];//��������
    double x;//��������
    double y;//��������
    double HP;//����Ѫ��
    int move_type;//�����ƶ���ʽ
    double speed;//�����ٶ�
    double damage;//�����˺�
    SDL_Rect bumpbox;//��ײ��
    State state;//����״̬
    Uint32 lastMoveTime;  // �ϴ��ƶ���ʱ��
    Uint32 moveInterval;  // �ƶ����
    bool isAlive = true;//�����Ƿ���
    //������Ϊ����
    void move(const PLAYER& player) { // �����ƶ�
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastMoveTime >= moveInterval) { // ����Ƿ�ﵽ�ƶ����
            switch (move_type) {
            case 1: // ׷������ƶ������ٶȲ���
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
            case 2: // ׷������ƶ�����Խ��������ٶ�Խ��
            {
                // �����������ҵľ���
                double dist = distance(x, y, player.x, player.y);
                // Ϊ�˷�ֹ�����㣬����һ����С����
                if (dist < 1.0) {
                    dist = 1.0;
                }
                // ���㷽������������һ��
                double dx = (player.x - x) / dist;
                double dy = (player.y - y) / dist;
                // �����ƶ��ٶȣ�����Խ���ٶ�Խ�죬����ԽС�ٶ�Խ��
                double moveSpeed = speed * (dist / 1000.0); // 100.0 ��һ������ϵ�����ɸ�����Ҫ����
                // ���¹���λ��
                x += dx * moveSpeed;
                y += dy * moveSpeed;
            }
            break;

            case 3: // ��һ����Χ������ƶ�
                uniform_real_distribution<> dis(0.0, 1.0);
                double random_element = dis(gen);
                double random_radian = random_element * 2 * PI;
                x += 0.5 * speed * cos(random_radian);
                y += 0.5 * speed * sin(random_radian);
                break;
            }
            lastMoveTime = currentTime; // �����ϴ��ƶ�ʱ��
        }
    }
    void die() {//��������
        isAlive = false;
    }
};
//��Ӭ��
class FLY : public MONSTER {
public:
    friend class PLAYER;
    Uint32 lastShootTime; // �ϴη����ӵ���ʱ��
    Uint32 shootInterval; // �������
    int currentFrame;     // ��ǰ����֡
    Uint32 lastFrameTime; // �ϴζ���֡ʱ��
    Uint32 attackStartTime; // ������ʼʱ��
    bool isReadyToAttack; // �Ƿ�׼�����ٴι���
    bool hasShot; // �Ƿ��Ѿ�������ӵ�
    SDL_Rect bumpbox;//��ײ��
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
        shootInterval = 2000; // ���ù������Ϊ2000���루2�룩
        lastMoveTime = SDL_GetTicks(); // ��ʼ��Ϊ��ǰʱ��
        moveInterval = 16 + rand() % 16; // �����ƶ����Ϊ16-32����֮������ֵ
        currentFrame = 0;
        lastFrameTime = SDL_GetTicks();
        attackStartTime = 0;
        isReadyToAttack = true;
        hasShot = false;
        // ��ʼ����ײ��
        this->bumpbox.w = 96; // ͼ����
        this->bumpbox.h = 96; // ͼ��߶�
        this->bumpbox.x = x + (96 - 40) / 2;
        this->bumpbox.y = y + (96 - 40) / 2;
    }
    FLY* createFly(int x, int y) {
        FLY* fly = new FLY(x, y);
        return fly;
    }
    void detectState(const PLAYER& player) { // �������Ƿ��ڹ�����Χ��
        if (state == ATTACK) {
            // ������ڹ��������ı�״̬
            return;
        }
        if (distance(x, y, player.x, player.y) <= 400 && isReadyToAttack) {
            state = ATTACK;
        }
        else {
            state = IDLE;
        }
    }
    void updateMoveType(const PLAYER& player) { // �����ƶ�״̬
        if (distance(x, y, player.x, player.y) <= 600)move_type = 2;
        else move_type = 3;
    }
    void shoot(const PLAYER& player) { // �����ӵ�
        Uint32 currentTime = SDL_GetTicks();
        if (isReadyToAttack) {
            double angle = atan2(player.y - y, player.x - x);
            FlyBullets.push_back(BULLET(x + 35, y + 50, 5, damage, 300, { angle }));
            lastShootTime = currentTime; // �����ϴη���ʱ��
            attackStartTime = currentTime; // ��¼������ʼʱ��
            isReadyToAttack = false; // ����Ϊ��׼������״̬
        }
    }
    void move(const PLAYER& player) { // �ƶ�
        detectState(player); // �������Ƿ��ڹ�����Χ��
        updateMoveType(player); // �����ƶ�״̬
        MONSTER::move(player); // ���û�����ƶ�����
        // ������ײ���λ��
        this->bumpbox.x = static_cast<int>(this->x) + 8;
        this->bumpbox.y = static_cast<int>(this->y) + 8;
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


// ��Ӭ�ֳ�̬����
void flyIdleMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyMotions, SDL_Rect& flyrect, FLY& fly) {
    Uint32 currentTime = SDL_GetTicks();
    // ÿ100ms�л�֡
    if (currentTime - fly.lastFrameTime >= 100) {
        fly.currentFrame = (fly.currentFrame + 1) % 2;
        fly.lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, FlyMotions[fly.currentFrame], NULL, &flyrect);
}
// ��Ӭ�ֹ�������
void flyAttackMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyMotions, SDL_Rect& flyrect, FLY& fly, const PLAYER& player) {
    Uint32 currentTime = SDL_GetTicks();
    // ÿ50ms�л�֡
    if (currentTime - fly.lastFrameTime >= 40) {
        fly.currentFrame = (fly.currentFrame + 1) % 16;
        fly.lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, FlyMotions[fly.currentFrame], NULL, &flyrect);

    // ������֡Ϊ��9֡ʱ�����ӵ�
    if (fly.currentFrame == 8 && !fly.hasShot) {
        fly.shoot(player);
        fly.hasShot = true; // �����ѷ����־����ֹ�ظ�����
    }

    // ��鶯���Ƿ񲥷����
    if (currentTime - fly.attackStartTime >= 440) { // ���蹥����������800ms
        fly.state = IDLE; // ����Ϊ��̬
        fly.isReadyToAttack = false; // ����Ϊ��׼������״̬
        fly.hasShot = false; // �����ѷ����־
    }
}
// ��Ӭ����������
void flyDeadMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyDeadMotions, SDL_Rect& flyrect) {
    static int currentFrame = 0;
    static Uint32 lastFrameTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    // ÿ200ms�л�֡�����°���ʱ����
    if (currentTime - lastFrameTime >= 200 || currentFrame == 0) {
        currentFrame = (currentFrame + 1) % FlyDeadMotions.size();
        lastFrameTime = currentTime;
    }
    SDL_RenderCopy(renderer, FlyDeadMotions[currentFrame], NULL, &flyrect);
}







/* �������������������� ״̬���ºͻ�����Ⱦ �������������������� */

// ���¹���λ���ڣ�ɾ�������Ĺ���
void updateMonsters(vector<FLY>& flies, PLAYER& player, int window_width, int window_height) {
    for (auto it = flies.begin(); it != flies.end();) {
        if (!it->isAlive) {
            it = flies.erase(it);
        }
        else {
            it->detectState(player);
            it->updateMoveType(player);
            it->move(player);

            // ����Ƿ�ﵽ�������
            Uint32 currentTime = SDL_GetTicks();
            if (!it->isReadyToAttack && currentTime - it->lastShootTime >= it->shootInterval) {
                it->isReadyToAttack = true; // ����Ϊ׼������״̬
            }

            // ��ײ���
            if (SDL_HasIntersection(&player.bumpbox, &it->bumpbox)) {
                if (!player.isInvincible) { // ��������޵�״̬
                    player.HP -= it->damage;
                    player.isInvincible = true; // ��ʼ�޵�״̬
                    player.invincibleStartTime = SDL_GetTicks(); // ��¼�޵п�ʼʱ��
                }
                // ��ѡ����ӽ�ɫ���˵ķ������粥��������Ч
            }

            ++it;
        }
    }
}