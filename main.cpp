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
#include <cstdlib>
#include <random>
#include <map>
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
//��ʼ�������
random_device rd;
mt19937 gen(rd());

//���ڴ�С
const int window_width = 1368;
const int window_height = 768;






/* �������������������� ��ɫ �������������������� */
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
	SDL_Rect spriteRect;//��ͼ����
    DIRECTION direction;//�ӵ����з���
    bool isAlive = true;//�ӵ��Ƿ���
    bool isBursting = false;//�ӵ��Ƿ����ڱ���
    Uint32 burstStartTime = 0;//���Ѷ�����ʼʱ��
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

        // ��ʼ����ͼ����
        this->spriteRect.w = 30; // ��ͼ���
        this->spriteRect.h = 30; // ��ͼ�߶�
        this->spriteRect.x = x;
        this->spriteRect.y = y;

        // ��ʼ����ײ���λ�úʹ�С
        this->bumpbox.w = 10; // ��ײ����
        this->bumpbox.h = 10; // ��ײ��߶�
        this->bumpbox.x = x + (this->spriteRect.w - this->bumpbox.w) / 2;
        this->bumpbox.y = y + (this->spriteRect.h - this->bumpbox.h) / 2;
    }

    //���������ӵ���Ϊ����

    //�ӵ�����λ��
    void updatePosition() {
        if (!isBursting) {
            double deltaX = speed * cos(direction.radian);
            double deltaY = speed * sin(direction.radian);

            // ����λ��
            x += deltaX;
            y += deltaY;

            // ������ͼ��λ��
            spriteRect.x = static_cast<int>(x);
            spriteRect.y = static_cast<int>(y);

            // ������ײ���λ��
            bumpbox.x = static_cast<int>(x) + (spriteRect.w - bumpbox.w) / 2;
            bumpbox.y = static_cast<int>(y) + (spriteRect.h - bumpbox.h) / 2;

            range -= speed;
            if (range <= 0) {
                burst();
            }
        }
    }

    //�ӵ�����
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
vector<BULLET> Bullets;//��ɫ�ӵ�����
vector<BULLET> FlyBullets;//��Ӭ���ӵ�����








/* �������������������� ���� �������������������� */
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
	SDL_Rect spriteRect;//��ͼ����
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
        lastMoveTime = SDL_GetTicks();
        moveInterval = 16 + rand() % 16;
        currentFrame = 0;
        lastFrameTime = SDL_GetTicks();
        attackStartTime = 0;
        isReadyToAttack = true;
        hasShot = false;

        // ��ʼ����ͼ����
        this->spriteRect.w = 96; // ��ͼ���
        this->spriteRect.h = 96; // ��ͼ�߶�
        this->spriteRect.x = x;
        this->spriteRect.y = y;

        // ��ʼ����ײ���λ�úʹ�С�����Ը�����Ҫ����
        this->bumpbox.w = 40; // ��ײ����
        this->bumpbox.h = 40; // ��ײ��߶�
        this->bumpbox.x = x + (this->spriteRect.w - this->bumpbox.w) / 2;
        this->bumpbox.y = y + (this->spriteRect.h - this->bumpbox.h) / 2;
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
        // ������ͼ��λ��
        this->spriteRect.x = static_cast<int>(this->x);
        this->spriteRect.y = static_cast<int>(this->y);

        // ������ײ���λ��
        this->bumpbox.x = static_cast<int>(this->x) + (this->spriteRect.w - this->bumpbox.w) / 2;
        this->bumpbox.y = static_cast<int>(this->y) + (this->spriteRect.h - this->bumpbox.h) / 2;
    }

};

vector<FLY> Flies;
FLY fly_standard(100, 100);

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
// �����ᵯ���Ѷ���
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
// ��Ӭ�ֳ�̬����
void flyIdleMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyMotions, vector<SDL_Texture*>& _FlyMotions, SDL_Rect& flyrect, FLY& fly , const PLAYER& player) {
    if (player.x >= fly.x) {
        Uint32 currentTime = SDL_GetTicks();
        // ÿ100ms�л�֡
        if (currentTime - fly.lastFrameTime >= 100) {
            fly.currentFrame = (fly.currentFrame + 1) % 2;
            fly.lastFrameTime = currentTime;
        }
        SDL_RenderCopy(renderer, FlyMotions[fly.currentFrame], NULL, &flyrect);
    }
	else {
		Uint32 currentTime = SDL_GetTicks();
		// ÿ100ms�л�֡
		if (currentTime - fly.lastFrameTime >= 100) {
			fly.currentFrame = (fly.currentFrame + 1) % 2;
			fly.lastFrameTime = currentTime;
		}
		SDL_RenderCopy(renderer, _FlyMotions[fly.currentFrame], NULL, &flyrect);
	}
}
// ��Ӭ�ֹ�������
void flyAttackMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyMotions, vector<SDL_Texture*>& _FlyMotions, SDL_Rect& flyrect, FLY& fly, const PLAYER& player) {
    if (player.x >= fly.x) {
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
    else {
        Uint32 currentTime = SDL_GetTicks();
        // ÿ50ms�л�֡
        if (currentTime - fly.lastFrameTime >= 40) {
            fly.currentFrame = (fly.currentFrame + 1) % 16;
            fly.lastFrameTime = currentTime;
        }
        SDL_RenderCopy(renderer, _FlyMotions[fly.currentFrame], NULL, &flyrect);
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

/* ������������������������������������������������������������ */










int room_number = 1;
mt19937 Xelem(rd()); // ʹ�� Mersenne Twister �㷨
mt19937 Yelem(rd()); // ʹ�� Mersenne Twister �㷨
uniform_int_distribution<> Xdis(window_width/2 - 550, window_width/2 + 550 ); // ���� 0 �� window_width - 96 ֮��������
uniform_int_distribution<> Ydis(window_height/2 - 275 , window_height + 275); // ���� 0 �� window_height - 96 ֮��������
//���ˢ�ֺ���
void generateMonster(int room_number ,PLAYER player, FLY fly) {
	for (int i = 0; i < room_number; i++) {
		int x = Xdis(Xelem);
		int y = Ydis(Yelem);
		//���x��y�Ƿ��������ײ����
		while (x + fly.bumpbox.w + 100 >= player.bumpbox.x && x - 100 <= player.bumpbox.x + player.bumpbox.w &&
            y - 100 <= player.bumpbox.y + player.bumpbox.h && y + fly.bumpbox.h + 100 >= player.bumpbox.y) {
			x = Xdis(Xelem);
			y = Ydis(Yelem);
		}
		FLY fly(x, y);
		Flies.push_back(fly);
	}
}   
// �л����� 
void switchRoom(SDL_Renderer* renderer, SDL_Texture* newRoomTexture, SDL_Rect& headrect, SDL_Rect& bodyrect, PLAYER player, FLY fly) {
    // ��Ⱦ�·��䱳��
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, newRoomTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
    // ���ý�ɫλ��
    headrect.x = window_width / 2 - headrect.w / 2;
    headrect.y = window_height / 2 - headrect.h / 2 + 200;
    bodyrect.x = window_width / 2 - bodyrect.w / 2;
    bodyrect.y = window_height / 2 - bodyrect.h / 2 + 225;
    // ��յ�ǰ�ӵ�
    Bullets.clear();
    FlyBullets.clear();
    // ��������µĹ���
	generateMonster(room_number, player, fly);
    room_number++;
}














/* �������������������� �¼�����ͻ�����Ⱦ �������������������� */

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
        Bullets.push_back(BULLET(isaac.x + 30, isaac.y, isaac.shootspeed, isaac.damage, isaac.range, { PI * 3 / 2 }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time; // ��¼������ʼʱ��
    }
    if (keyStates[5] && current_time - last_shoot_time >= attack_interval) { // ��
        head_direction = LEFT;
        Bullets.push_back(BULLET(isaac.x + 30, isaac.y, isaac.shootspeed, isaac.damage, isaac.range, { PI }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time;
    }
    if (keyStates[6] && current_time - last_shoot_time >= attack_interval) { // ��
        head_direction = DOWN;
        Bullets.push_back(BULLET(isaac.x + 30, isaac.y, isaac.shootspeed, isaac.damage, isaac.range, { PI / 2 }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time;
    }
    if (keyStates[7] && current_time - last_shoot_time >= attack_interval) { // ��
        head_direction = RIGHT;
        Bullets.push_back(BULLET(isaac.x + 30, isaac.y, isaac.shootspeed, isaac.damage, isaac.range, { 0 }));
        last_shoot_time = current_time;
        is_attacking = true;
        attack_start_time = current_time;
    }

    // ��鶯���Ƿ񲥷����
    if (is_attacking && current_time - attack_start_time >= attack_interval) {
        is_attacking = false; // ����������ֹͣ����״̬
    }
}



// ���½�ɫλ��
void updatePlayerPosition(SDL_Rect& headrect, SDL_Rect& bodyrect, const bool keyStates[4], int window_width,
    int window_height, int& bodyDirection, PLAYER& isaac) {
    if (keyStates[0] && headrect.y > 50) {
        headrect.y -= 2.5 * isaac.speed;
        bodyrect.y -= 2.5 * isaac.speed;
        bodyDirection = 1; // ��
    }
    else if (keyStates[1] && headrect.x > 120) {
        headrect.x -= 2.5 * isaac.speed;
        bodyrect.x -= 2.5 * isaac.speed;
        bodyDirection = 3; // ��
    }
    else if (keyStates[2] && bodyrect.y < window_height - bodyrect.h - 120) {
        headrect.y += 2.5 * isaac.speed;
        bodyrect.y += 2.5 * isaac.speed;
        bodyDirection = 2; // ��
    }
    else if (keyStates[3] && headrect.x < window_width - headrect.w - 130) {
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
// �� updateBullets �����У����һ��������������������ҵ��ӵ����ǹ�����ӵ�
void updateBullets(vector<BULLET>& bullets, int window_width, int window_height, vector<SDL_Texture*>& BurstMotions, bool isEnemyBullet, PLAYER& player) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        it->update();

        // ����ӵ��Ƿ�����ǽ�ڣ����ڱ߽磩�������������
        if (!it->isBursting && (it->bumpbox.x < 120 || it->bumpbox.x > window_width - it->bumpbox.w - 120 ||
            it->bumpbox.y < 100 || it->bumpbox.y > window_height - it->bumpbox.h - 100 )) {
            it->burst();
        }

        // ��ײ���
        if (!it->isBursting) {
            // ��������ӵ�
            if (isEnemyBullet) {
                if (it->isCollide(player.bumpbox)) {
                    if (!player.isInvincible) { // ��������޵�״̬
                        player.HP -= it->damage;
                        player.isInvincible = true; // ��ʼ�޵�״̬
                        player.invincibleStartTime = SDL_GetTicks();
                    }
                    it->burst();
                }
            }

            else {
                // ����ӵ�������Ƿ���й���
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

        // ����ӵ����ڲ��ű��Ѷ�������鶯���Ƿ����
        if (it->isBursting) {
            Uint32 currentTime = SDL_GetTicks();
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
// �����ӵ���Ⱦ
void renderBullets(SDL_Renderer* renderer, const vector<BULLET>& bullets, SDL_Texture* bulletTexture,
    vector<SDL_Texture*>& BurstMotions) {
    for (const auto& bullet : bullets) {
        if (bullet.isBursting) {
            // ���㵱ǰӦ��ʾ�ı��Ѷ���֡
            Uint32 currentTime = SDL_GetTicks();
            Uint32 elapsedTime = currentTime - bullet.burstStartTime;
            int frame = (elapsedTime / 100) % BurstMotions.size(); // ÿ֡100ms
            SDL_RenderCopy(renderer, BurstMotions[frame], NULL, &bullet.spriteRect);
        }
        else {
            SDL_RenderCopy(renderer, bulletTexture, NULL, &bullet.spriteRect);
        }
    }
}

// ����Ѫ����Ⱦ
void renderPlayerHealth(SDL_Renderer* renderer, const PLAYER& player, SDL_Texture* full_heart, SDL_Texture* half_heart, SDL_Texture* empty_heart) {
    int maxHearts = 6; // �����������Ϊ6
    int heartWidth = 30;
    int heartHeight = 30;

    for (int i = 0; i < maxHearts; ++i) {
        SDL_Rect heartRect = { 10 + i * (heartWidth + 5), 10, 1.5*heartWidth, 1.5*heartHeight };
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

    // ��ʼ�� SDL ttf
    if (TTF_Init() == -1) {
        SDL_Log("TTF_Init: %s\n", TTF_GetError());
        return 1;
    }

    // ��ʼ���¼�����
    bool isquit = false;
    SDL_Event event;

    // ��ȡ��Ļ��С
    SDL_Rect screen_rect;
    SDL_GetDisplayBounds(0, &screen_rect);

    // ��������
    int x = screen_rect.w / 2 - window_width / 2;
    int y = screen_rect.h / 2 - window_height / 2;
    SDL_Window* window = SDL_CreateWindow("The Binding of Isaac", x, y,
        window_width, window_height, SDL_WINDOW_SHOWN);

    // ������Ⱦ
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // ��������
    // // �����Ҫ���� ftn ���壬���԰����·�ʽ��
    //if (!loadFont("ISAAC/Fonts/terminus8.fnt", renderer)) {
       // SDL_Log("Failed to load font");
       //return 1;
    //}
    // �����Ҫ���� TTF ���壬���԰����·�ʽ��
      if (!loadFont("ISAAC/Fonts/inconsolata-bold.ttf", renderer, 24)) {
           SDL_Log("Failed to load font");
            return 1;
        }

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
    bool startGame = false;

    //��������
    playMainMusic();
    if (main_music == NULL) {
        SDL_Log("Failed to load beat music! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }
    Mix_PlayMusic(main_music, -1);

    /*��ʼ����*/

    // ������ʼ��������
    SDL_Texture* start_screen = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/start_screen.png");
    if (start_screen == NULL) {
        SDL_Log("Failed to load start screen texture: %s", IMG_GetError());
        return 1;
    }

    while (!startGame && !isquit) {
        // �����¼�
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isquit = true;
            }
            if (event.type == SDL_KEYDOWN) {
                // ���û����»س���ʱ����ʼ��Ϸ
                if (event.key.keysym.sym == SDLK_RETURN) {
                    startGame = true;
                }
            }
        }

        // ��Ⱦ��ʼ����
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, start_screen, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ����֡��

        // ��Ⱦ����ʱ��ָ����ɫ
        SDL_Color blackColor = { 0, 0, 0, 0 };
        renderText(renderer, "�� Enter ����ʼ��Ϸ", window_width / 2 - 150, window_height - 100, blackColor);

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ����֡��
    }



    /*�ͷſ�ʼ��������*/
    SDL_DestroyTexture(start_screen);



    /* ������������ */
    SDL_Texture* basement = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/basement.png");
    SDL_Texture* black = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/black.png");

    /* �������� */

    SDL_Texture* bullet_texture = IMG_LoadTexture(renderer, "ISAAC/Characters/bullet.png");
    SDL_Texture* enemy_bullet_texture = IMG_LoadTexture(renderer, "ISAAC/Monsters/enemy_bullet.png");
    SDL_Texture* full_heart = IMG_LoadTexture(renderer, "ISAAC/Characters/full_heart.png");
    SDL_Texture* half_heart = IMG_LoadTexture(renderer, "ISAAC/Characters/half_heart.png");
    SDL_Texture* empty_heart = IMG_LoadTexture(renderer, "ISAAC/Characters/empty_heart.png");

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
    PLAYER isaac(bodyrect.x, bodyrect.y, headrect.w, headrect.h, 12, 2, 3.5, 3, 7, 500); // ������ɫ








    Uint32 switch_start_time = 0; // �л����俪ʼʱ��
    bool switching_room = false; // �Ƿ������л�����
    bool black_screen = false; // �Ƿ��ں���״̬

    while (!isquit) {

        if (isaac.HP <= 0) {
            // �˳�����
            isquit = true;
            // ���߿��Բ�������������
        }

        // �����¼�
        processInput(event, isquit, keyStates);

        // ÿ֡�������
        processShooting(keyStates, head_direction, headrect, isaac, last_shoot_time, is_attacking, attack_start_time);

        // ��������ӵ�
        updateBullets(Bullets, window_width, window_height, BurstMotions, false, isaac);
        // ���¹����ӵ�
        updateBullets(FlyBullets, window_width, window_height, EnemyBurstMotions, true, isaac);


        // ���¹���λ��
        updateMonsters(Flies, isaac, window_width, window_height);

        // ��Ⱦ����
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, black, NULL, NULL);
        SDL_RenderCopy(renderer, basement, NULL, NULL);


        // ���½�ɫλ��
        updatePlayerPosition(headrect, bodyrect, keyStates, window_width, window_height, bodyDirection, isaac);


        // ��� bodyrect �Ƿ����ָ���ľ�������, ���Ҽ������Ƿ��������
        // ��ӵ�����Ϣ
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
           
        // ����Ƿ�ﵽ�ӳ�ʱ��
        if (switching_room && SDL_GetTicks() - switch_start_time >= 1000) { // �ӳ��� 5 ��
            switchRoom(renderer, basement, headrect, bodyrect,isaac, fly_standard);
            switching_room = false;
            black_screen = false;
        }

        // ��Ⱦ����
        SDL_RenderClear(renderer);
        if (black_screen) {
            SDL_RenderCopy(renderer, black, NULL, NULL);
        }
        else {
            SDL_RenderCopy(renderer, basement, NULL, NULL);
        }

        


        /* ���������������������������� ��Ⱦ���� ����������������������������*/

        if (!black_screen) {
            // ����Ѫ��
            renderPlayerHealth(renderer, isaac, full_heart, half_heart, empty_heart);
            // �ӵ�����ʱ����Ⱦ�ӵ�������Ⱦ��ɫ
            for (const auto& bullet : Bullets) {
                if (bullet.direction.radian == 3 * PI / 2 && bullet.bumpbox.y + bullet.bumpbox.h < headrect.y + headrect.h) {
                    renderBullets(renderer, { bullet }, bullet_texture, BurstMotions);
                }
            }


            //����������������������������ײ�����
            /*
            // ���û�����ɫΪ��ɫ
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            // ���ƽ�ɫ����ײ��
            SDL_RenderDrawRect(renderer, &player.bumpbox);
            // ���ƹ������ײ��
            for (const auto& fly : Flies) {
                SDL_RenderDrawRect(renderer, &fly.bumpbox);
            }
            */



            // ���½�ɫ����
            updatePlayerMotion(renderer, BackMotions, FrontMotions, RightMotions, LeftMotions, BackHeadMotions,
                FrontHeadMotions, RightHeadMotions, LeftHeadMotions, headrect, bodyrect, keyStates, bodyDirection, is_attacking, isaac);
            // ����������ӵ���Ⱦ�������ڵ���
            for (const auto& bullet : Bullets) {
                if (bullet.direction.radian != 3 * PI / 2 || bullet.bumpbox.y + bullet.bumpbox.h >= headrect.y + headrect.h) {
                    renderBullets(renderer, { bullet }, bullet_texture, BurstMotions);
                }
            }
            // ��Ⱦ��Ӭ��
            for (auto& fly : Flies) {
                if (fly.state == ATTACK) {
                    flyAttackMotion(renderer, FlyMotions, _FlyMotions, fly.spriteRect, fly, isaac);
                }
                else {
                    flyIdleMotion(renderer, FlyMotions, _FlyMotions, fly.spriteRect, fly, isaac);
                }
            }
            // ��Ⱦ�����ӵ�
            renderBullets(renderer, FlyBullets, enemy_bullet_texture, EnemyBurstMotions);

        }

        /* ������������������������������������������������������������ */





        // ˢ�»���     
        SDL_RenderPresent(renderer);

        // ����֡��
        SDL_Delay(16); // Լ 60 FPS
    }





    // ������Դ
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
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}



