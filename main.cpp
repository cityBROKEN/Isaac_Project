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
//��Ϸ״̬
bool isPause = false;

/* �������������������� �ϰ��� �������������������� */
class OBSTACLE {
public:
    int x, y; // �ϰ�������
    int w, h; // �ϰ����Ⱥ͸߶�
    SDL_Rect rect; // �ϰ���ľ�������
    SDL_Texture* texture; // �ϰ��������
	SDL_Rect collisionRect; // �ϰ������ײ��
    OBSTACLE(int x, int y, int w, int h, SDL_Texture* texture) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
        this->rect = { x, y, w, h };
        this->texture = texture;
        // ��ʼ����ײ�䣬ʹ���ʵ���ϰ���СһЩ
        int collisionMargin = 20; // ��ײ����С�ı߾�
        this->collisionRect = { x + collisionMargin, y + collisionMargin, w - 2 * collisionMargin, h - 2 * collisionMargin };
    
    }
};
vector<OBSTACLE> Obstacles; // �ϰ�������



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
	bool isAlive = true;//��ɫ�Ƿ���
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
		this->isAlive = true;
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
        //���������1��2
        uniform_int_distribution<int> disw(1, 2); // �����������ΧΪ[1, 7]
        int random_number = disw(gen); // ���������
		if (random_number == 1) {
            playShootSound1();
            Mix_PlayChannel(-1, shoot_sound1, 0);

        }
		else {
            playShootSound2();
            Mix_PlayChannel(-1, shoot_sound2, 0);
        }
	}

    //�ӵ�����
    void die() {
        isAlive = false;
    }
};
vector<BULLET> Bullets;//��ɫ�ӵ�����
vector<BULLET> FlyBullets;//��Ӭ���ӵ�����



/* �������������������� ����� �������������������� */
int randomNumberOneToSeven() {
    std::random_device rd;  // �������������
    std::mt19937 gen(rd()); // ������������棬ʹ��÷ɭ��ת�㷨
    std::uniform_int_distribution<> dis(1, 7); // �����������ΧΪ[1, 7]

    int random_number = dis(gen); // ���������
	return random_number;
}



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
    double vx; // x���ٶȷ���
    double vy; // y���ٶȷ���
    double damage;//�����˺�
    SDL_Rect bumpbox;//��ײ��
    SDL_Rect spriteRect;//��ͼ����
    State state;//����״̬
    Uint32 lastMoveTime;  // �ϴ��ƶ���ʱ��
    Uint32 moveInterval;  // �ƶ����
    bool isAlive = true;//�����Ƿ���
	bool isDying = false; // �Ƿ���������
    Uint32 currentFrame;     // ��ǰ����֡
    Uint32 lastFrameTime; // �ϴζ���֡ʱ��
    Uint32 deathStartTime = 0;      // ��¼����������ʼʱ��
    Uint32 deathDuration = 550;     // ������������ʱ�䣨���룩
    // ������Ϊ����
	// �����ƶ�
    void move(const PLAYER& player) {
        // �����������ҵľ���ͷ���
        double dx = player.x - x;
        double dy = player.y - y;
        double dist = sqrt(dx * dx + dy * dy);

        // ��һ����������
        double dirX = dx / dist;
        double dirY = dy / dist;

        // �����ƶ����͵���������ٶ�
        switch (move_type) {
        case 1:
            // ׷����ң��ٶȲ���
            vx = dirX * speed;
            vy = dirY * speed;
            break;
        case 2:
            // Խ��������ٶ�Խ��
            if (dist > 0) {
                double moveSpeed = speed * (dist / 2500.0); // ���� 500.0 Ϊ���ʵı���
                vx = dirX * moveSpeed;
                vy = dirY * moveSpeed;
            }
            break;
        case 3:
            // ����ƶ�
        {
            uniform_real_distribution<> dis(-1.0, 1.0);
            vx += dis(gen) * speed * 0.1;
            vy += dis(gen) * speed * 0.1;
            // �����ٶ�
            double maxSpeed = speed;
            double currentSpeed = sqrt(vx * vx + vy * vy);
            if (currentSpeed > maxSpeed) {
                vx = (vx / currentSpeed) * maxSpeed;
                vy = (vy / currentSpeed) * maxSpeed;
            }
        }
        break;
        case 4:
			//45��ֱ���ƶ����ٶȲ��䣬�����߽練��
			vx = speed;
			vy = speed;
			break;
        }

        // ����λ��
        x += vx;
        y += vy;

        // Ӧ��Ħ�������𽥼���
        vx *= 0.9;
        vy *= 0.9;

        // ������ײ�����ͼ��λ��
        bumpbox.x = static_cast<int>(x) + (spriteRect.w - bumpbox.w) / 2;
        bumpbox.y = static_cast<int>(y) + (spriteRect.h - bumpbox.h) / 2;
        spriteRect.x = static_cast<int>(x);
        spriteRect.y = static_cast<int>(y);

        // �߽��⣬��ֹ�����Ƴ���Ļ����ѡ��
        if (x < 120) {
            x = 120;
            vx = -vx * 0.5; // ��ײ����Ч��
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

    // ����λ�õķ���
    void updatePosition() {
        // ����λ��
        x += vx;
        y += vy;

        // Ӧ��Ħ�������𽥼���
        vx *= 0.9; // Ħ��ϵ�����ɸ�����Ҫ����
        vy *= 0.9;

        // �߽��⣬��ֹ�����Ƴ���Ļ
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

        // ������ײ�����ͼ��λ��
        bumpbox.x = static_cast<int>(x) + (spriteRect.w - bumpbox.w) / 2;
        bumpbox.y = static_cast<int>(y) + (spriteRect.h - bumpbox.h) / 2;
        spriteRect.x = static_cast<int>(x);
        spriteRect.y = static_cast<int>(y);
    }
    void die() {
        if (!isDying) {
            isDying = true;
            deathStartTime = SDL_GetTicks(); // ��¼����������ʼʱ��
            vx = 0; // ֹͣ�ƶ�
            vy = 0;
			// ����������Ч
            playMonsterDeath();
			Mix_PlayChannel(-1, monster_death, 0);
        }
    }

};

//��Ӭ��
class FLY : public MONSTER {
public:
    friend class PLAYER;
    Uint32 lastShootTime; // �ϴη����ӵ���ʱ��
    Uint32 shootInterval; // �������
    Uint32 attackStartTime; // ������ʼʱ��
    bool isReadyToAttack; // �Ƿ�׼�����ٴι���
    bool hasShot; // �Ƿ��Ѿ�������ӵ�    

	
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
        shootInterval = 2000; // ���ù������Ϊ2000���루2�룩
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

    void updateDeathAnimation() {
        // ��������ʱ����㵱ǰ֡���������������� 11 ֡��ÿ֡���� deathDuration / 11 ���룩
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

//���Ӭ��
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
        // ��ʼ����ͼ����
        this->spriteRect.w = 90; // ��ͼ���
        this->spriteRect.h = 90; // ��ͼ�߶�
        this->spriteRect.x = x;
        this->spriteRect.y = y;
        // ��ʼ����ײ���λ�úʹ�С�����Ը�����Ҫ����
        this->bumpbox.w = 50; // ��ײ����
        this->bumpbox.h = 50; // ��ײ��߶�
        this->bumpbox.x = x + (this->spriteRect.w - this->bumpbox.w) / 2;
        this->bumpbox.y = y + (this->spriteRect.h - this->bumpbox.h) / 2;
        // 45��ֱ���ƶ����ٶȲ��䣬�����߽練��
        vx = speed * cos(PI / 4);
        vy = speed * sin(PI / 4);
    }
	BIGFLY* createBigFly(int x, int y) {
		BIGFLY* bigfly = new BIGFLY(x, y);
		return bigfly;
	}
    void move() {

		// ����λ��
        x += vx;
        y += vy;

        // �߽��⣬��ֹ�����Ƴ���Ļ�������߽練��
		if (x < 120 || x > window_width - bumpbox.w - 120) {//�������ұ߽�
			vx = -vx;
        }
        if (y < 100 || y > window_height - bumpbox.h - 100) {
			vy = -vy;
        }

        // ������ײ�����ͼ��λ��
        bumpbox.x = static_cast<int>(x) + (spriteRect.w - bumpbox.w) / 2;
        bumpbox.y = static_cast<int>(y) + (spriteRect.h - bumpbox.h) / 2;
        spriteRect.x = static_cast<int>(x);
        spriteRect.y = static_cast<int>(y);
    }

    void die() {
        if (!isDying) {
            isDying = true;
            deathStartTime = SDL_GetTicks(); // ��¼����������ʼʱ��
            vx = 0; // ֹͣ�ƶ�
            vy = 0;
            // �������������������ӵ�
            double angle = 0;
            for (int i = 0; i < 6; i++) {
                // �޸�������ӵ���ӵ� FlyBullets ������ Bullets
                FlyBullets.push_back(BULLET(x + 45, y + 45, 5, damage, 300, { angle }));
                angle += PI / 3;
            }
            // ����������Ч
            playMonsterDeath();
            Mix_PlayChannel(-1, monster_death, 0);
        }
    }

	void updateDeathAnimation() {
		// ��������ʱ����㵱ǰ֡���������������� 11 ֡��ÿ֡���� deathDuration / 11 ���룩
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
void flyDeadMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& FlyDeadMotions, SDL_Rect& flyrect, FLY& fly) {
    // ȷ��֡��������������֡��
    int totalFrames = FlyDeadMotions.size();
    if (fly.currentFrame >= totalFrames) {
        fly.currentFrame = totalFrames - 1;
    }
    SDL_RenderCopy(renderer, FlyDeadMotions[fly.currentFrame], NULL, &flyrect);
}
// ���Ӭ�ֳ�̬����
void bigFlyMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BigFlyMotions, SDL_Rect& bigflyrect, BIGFLY& bigfly) {
	Uint32 currentTime = SDL_GetTicks();
	// ÿ80ms�л�֡
	if (currentTime - bigfly.lastFrameTime >= 80) {
		bigfly.currentFrame = (bigfly.currentFrame + 1) % 2;
		bigfly.lastFrameTime = currentTime;
	}
	SDL_RenderCopy(renderer, BigFlyMotions[bigfly.currentFrame], NULL, &bigflyrect);
}
// ���Ӭ����������
void bigFlyDeadMotion(SDL_Renderer* renderer, vector<SDL_Texture*>& BigFlyDeadMotions, SDL_Rect& bigflyrect, BIGFLY& bigfly) {
	// ȷ��֡��������������֡��
	int totalFrames = BigFlyDeadMotions.size();
	if (bigfly.currentFrame >= totalFrames) {
		bigfly.currentFrame = totalFrames - 1;
	}
	SDL_RenderCopy(renderer, BigFlyDeadMotions[bigfly.currentFrame], NULL, &bigflyrect);
}

/* ������������������������������������������������������������ */










int room_number = 1;
// ���ˢ�ֺ���
void generateMonsters(int room_number, PLAYER& player) {
    uniform_int_distribution<int> Xdis(150, window_width - 150);
    uniform_int_distribution<int> Ydis(120, window_height - 120);
    uniform_real_distribution<double> monsterTypeDis(0.0, 1.0); // �������������������

    for (int i = 0; i < room_number; i++) {
        int x = Xdis(gen);
        int y = Ydis(gen);

        // ȷ�����ɵĹ��ﲻ����Ҹ���
        while (abs(x - player.x) < 200 && abs(y - player.y) < 200) {
            x = Xdis(gen);
            y = Ydis(gen);
        }

        double monsterType = monsterTypeDis(gen); // ����0��1֮��������

        if (monsterType < 0.7) {
            // 70%�ĸ������ɲ�Ӭ��
            FLY fly(x, y);
            Flies.push_back(fly);
        }
        else {
            // 30%�ĸ������ɴ��Ӭ��
            BIGFLY bigfly(x, y);
            BigFlies.push_back(bigfly);
        }
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
    headrect.y = window_height / 2 - headrect.h / 2 + 180;
    bodyrect.x = window_width / 2 - bodyrect.w / 2;
    bodyrect.y = window_height / 2 - bodyrect.h / 2 + 205;
    // ��յ�ǰ�ӵ�
    Bullets.clear();
    FlyBullets.clear();
    // ����ϰ���
    Obstacles.clear();
}
// �����ϰ���
void generateObstacles(int numObstacles, SDL_Texture* obstacleTexture) {
    uniform_int_distribution<> Xdis(150, window_width - 250);
    uniform_int_distribution<> Ydis(120, window_height - 170);
    // ���Ը��������ʵ�ʳߴ����ù̶��Ŀ��
    int obstacleWidth = 100; // ������
    int obstacleHeight = 100; // ����߶�
    for (int i = 0; i < numObstacles; ++i) {
        int x = Xdis(gen);
        int y = Ydis(gen);
        Obstacles.push_back(OBSTACLE(x, y, obstacleWidth, obstacleHeight, obstacleTexture));
        //SDL_Log("Obstacle generated at position (%d, %d)", x, y); // ����ϰ���λ��
    }
}













/* �������������������� �¼�����ͻ�����Ⱦ �������������������� */

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
				// ��ͣ����
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
    // ����ԭʼλ��
    int originalX = headrect.x;
    int originalY = headrect.y;
    int originalBodyX = bodyrect.x;
    int originalBodyY = bodyrect.y;

    // ���Ը���λ��
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
    else if (keyStates[2] && bodyrect.y < window_height - bodyrect.h - 130) {
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

    // ���½�ɫ����ײ��
    isaac.x = headrect.x + headrect.w / 2 - 45;
    isaac.y = headrect.y + (headrect.h + bodyrect.h) / 2 - 65;
    isaac.bumpbox.x = headrect.x + 15;
    isaac.bumpbox.y = headrect.y + 10;
    isaac.bumpbox.w = headrect.w - 30;
    isaac.bumpbox.h = headrect.h - 10;

    // ������ϰ������ײ
    bool collided = false;
    for (const auto& obstacle : Obstacles) {
        if (SDL_HasIntersection(&isaac.bumpbox, &obstacle.collisionRect)) {
            collided = true;
            break;
        }
    }

    // ���������ײ���ָ�ԭʼλ��
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

    // �����޵�״̬�ļ�ʱ
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

        // ����ӵ��Ƿ�����ǽ�ڻ��ϰ�������������
        if (!it->isBursting) {
            // ���ǽ��
            if (it->bumpbox.x < 120 || it->bumpbox.x > window_width - it->bumpbox.w - 120 ||
                it->bumpbox.y < 90 || it->bumpbox.y > window_height - it->bumpbox.h - 100) {
                it->burst();
            }

            // ����ϰ���
            for (const auto& obstacle : Obstacles) {
                if (SDL_HasIntersection(&it->bumpbox, &obstacle.collisionRect)) {
                    it->burst();
                    break;
                }
            }
        }

        // ��ײ���
        if (!it->isBursting) {
            if (isEnemyBullet) {
                // ��������ӵ�
                if (it->isCollide(player.bumpbox)) {
                    if (!player.isInvincible) { // ��������޵�״̬
                        player.HP -= it->damage;
						playHurtGrunt();
						Mix_PlayChannel(-1, hurt_grunt, 0);//����������Ч
                        player.isInvincible = true; // ��ʼ�޵�״̬
                        player.invincibleStartTime = SDL_GetTicks();
                    }
                    it->burst();
                }
            }
            else {
                // ����ӵ�������Ƿ����С��Ӭ����
                for (auto& fly : Flies) {
                    if (it->isCollide(fly.bumpbox)) {
                        it->burst();
                        fly.HP -= it->damage;
                        if (fly.HP <= 0) {
                            fly.die();
                        }
                        // �������Ч��
                        double knockbackStrength = 5.0; // �������ȣ��ɸ�����Ҫ����
                        fly.vx += knockbackStrength * cos(it->direction.radian);
                        fly.vy += knockbackStrength * sin(it->direction.radian);
                        break;
                    }
                }
                // ����ӵ�������Ƿ���д��Ӭ����
                for (auto& bigFly : BigFlies) {
                    if (it->isCollide(bigFly.bumpbox)) {
                        it->burst();
                        bigFly.HP -= it->damage;
                        if (bigFly.HP <= 0) {
                            bigFly.die();
                        }
                        // �������Ч��
                        double knockbackStrength = 3.0; // ���Ӭ������أ��������ȿ��ʵ���С
                        bigFly.vx += knockbackStrength * cos(it->direction.radian);
                        bigFly.vy += knockbackStrength * sin(it->direction.radian);
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
            int frame = (elapsedTime / 50) % BurstMotions.size(); // ÿ֡50ms
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




// ���¹���λ�ú�״̬��ɾ�������Ĺ���
void updateMonsters(vector<FLY>& flies, vector<BIGFLY>& bigFlies, PLAYER& player, int window_width, int window_height) {
    // ����С��Ӭ����
    for (auto it = flies.begin(); it != flies.end();) {
        if (it->isDying) {
            // ������������Ƿ񲥷����
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - it->deathStartTime >= it->deathDuration) {
                // ��������������ϣ����������Ƴ�
                it = flies.erase(it);
                continue;
            }
            else {
                // ������������֡
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

            // ����λ�ã�Ӧ�û���Ч��
            it->updatePosition();

            // ����Ƿ�ﵽ�������
            Uint32 currentTime = SDL_GetTicks();
            if (!it->isReadyToAttack && currentTime - it->lastShootTime >= it->shootInterval) {
                it->isReadyToAttack = true; // ����Ϊ׼������״̬
            }

            // ��ײ��⣨С��Ӭ������ң�
            if (SDL_HasIntersection(&player.bumpbox, &it->bumpbox)) {
                if (!player.isInvincible) { // ��������޵�״̬
                    player.HP -= it->damage;
                    playHurtGrunt();//����������Ч
                    Mix_PlayChannel(-1, hurt_grunt, 0);
                    player.isInvincible = true; // ��ʼ�޵�״̬
                    player.invincibleStartTime = SDL_GetTicks(); // ��¼�޵п�ʼʱ��
                }
            }

            ++it;
        }
    }

    // ������Ӭ����
    for (auto it = bigFlies.begin(); it != bigFlies.end();) {
        if (it->isDying) {
            // ������������Ƿ񲥷����
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - it->deathStartTime >= it->deathDuration) {
                // ��������������ϣ����������Ƴ�
                it = bigFlies.erase(it);
                continue;
            }
            else {
                // ������������֡
                it->updateDeathAnimation();
                ++it;
                continue;
            }
        }
        if (!it->isAlive) {
            it = bigFlies.erase(it);
        }
        else {
            // ���´��Ӭ�����λ��
            it->move();

            // ��ײ��⣨���Ӭ������ң�
            if (SDL_HasIntersection(&player.bumpbox, &it->bumpbox)) {
                if (!player.isInvincible) { // ��������޵�״̬
                    player.HP -= it->damage;
                    playHurtGrunt();//����������Ч
                    Mix_PlayChannel(-1, hurt_grunt, 0);
                    player.isInvincible = true; // ��ʼ�޵�״̬
                    player.invincibleStartTime = SDL_GetTicks(); // ��¼�޵п�ʼʱ��
                }
            }

            ++it;
        }
    }
}












// �������溯��
void renderGameOverScreen(SDL_Renderer* renderer, TTF_Font* font, int window_width, int window_height, SDL_Texture* endmenu) {
    //���ñ���Ϊendmenu
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, endmenu, NULL, NULL);

    // ��Ⱦ "Game Over" ����
	SDL_Color blackColor = { 0, 0, 0 };
    SDL_Surface* gameOverSurface = TTF_RenderText_Solid(font, "Game Over", blackColor);
    SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
    SDL_Rect gameOverRect;
    gameOverRect.w = gameOverSurface->w*4;
    gameOverRect.h = gameOverSurface->h*4;
    gameOverRect.x = (window_width - gameOverRect.w) / 2;
    gameOverRect.y = (window_height - gameOverRect.h) / 2 - 100;
    SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);

    // ��Ⱦ "The number of rooms you passed: " ����
    string roomPassed = "The number of rooms you passed: " + to_string(room_number - 1);
    SDL_Surface* roomPassedSurface = TTF_RenderText_Solid(font, roomPassed.c_str(), blackColor);
    SDL_Texture* roomPassedTexture = SDL_CreateTextureFromSurface(renderer, roomPassedSurface);
    SDL_Rect roomPassedRect;
    roomPassedRect.w = roomPassedSurface->w;
    roomPassedRect.h = roomPassedSurface->h;
    roomPassedRect.x = (window_width - roomPassedRect.w) / 2;
    roomPassedRect.y = (window_height - roomPassedRect.h) / 2;
    SDL_RenderCopy(renderer, roomPassedTexture, NULL, &roomPassedRect);

    // ��Ⱦ "Press Enter to Restart or Esc to Exit" ����
    SDL_Surface* exitSurface = TTF_RenderText_Solid(font, "Press Enter to Restart or Esc to Exit", blackColor);
    SDL_Texture* exitTexture = SDL_CreateTextureFromSurface(renderer, exitSurface);
    SDL_Rect exitRect;
    exitRect.w = exitSurface->w;
    exitRect.h = exitSurface->h;
    exitRect.x = (window_width - exitRect.w) / 2;
    exitRect.y = (window_height - exitRect.h) / 2 + 50;
    SDL_RenderCopy(renderer, exitTexture, NULL, &exitRect);

    // ������Ļ
    SDL_RenderPresent(renderer);

    // �ͷ���Դ
    SDL_FreeSurface(gameOverSurface);
    SDL_DestroyTexture(gameOverTexture);
    SDL_FreeSurface(exitSurface);
    SDL_DestroyTexture(exitTexture);
}
//��ͣ���溯��
void renderPauseScreen(SDL_Renderer* renderer, TTF_Font* font, int window_width, int window_height, SDL_Texture* pausemenu) {
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, pausemenu, NULL, NULL);
	// ������Ļ
	SDL_RenderPresent(renderer);
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
   
	modifyVolume();//��������

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




    /*��ʼ����*/
    //��������
    playTitleScreenSound();
    if (title_screen_sound == NULL) {
        SDL_Log("Failed to load beat music! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }
    Mix_PlayMusic(title_screen_sound, -1);


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

        // ����
        SDL_RenderClear(renderer);

        // ��Ⱦ��ʼ����
        SDL_RenderCopy(renderer, start_screen, NULL, NULL);

        SDL_Delay(16); // ����֡��

        // ��Ⱦ����ʱ��ָ����ɫ
        SDL_Color blackColor = { 0, 0, 0, 0 };
        renderText(renderer, "Press Enter To Start", window_width / 2 , window_height /2, blackColor);



        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ����֡��
    }


    /*�ͷſ�ʼ��������*/
    SDL_DestroyTexture(start_screen);


	// ����������
    playMainMusic();
    if (main_music == NULL) {
        SDL_Log("Failed to load beat music! SDL_mixer Error: %s", Mix_GetError());
        return 1;
    }
    Mix_PlayMusic(main_music, -1);




    /* ������������ */
    SDL_Texture* basement = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/basement.png");
    SDL_Texture* black = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/black.png");

    /* �������� */
    SDL_Texture* bullet_texture = IMG_LoadTexture(renderer, "ISAAC/Characters/bullet.png");
    SDL_Texture* enemy_bullet_texture = IMG_LoadTexture(renderer, "ISAAC/Monsters/enemy_bullet.png");
    SDL_Texture* full_heart = IMG_LoadTexture(renderer, "ISAAC/Characters/full_heart.png");
    SDL_Texture* half_heart = IMG_LoadTexture(renderer, "ISAAC/Characters/half_heart.png");
    SDL_Texture* empty_heart = IMG_LoadTexture(renderer, "ISAAC/Characters/empty_heart.png");
    SDL_Texture* obstacleTexture = IMG_LoadTexture(renderer, "ISAAC/Obstacle/stone.png");
	SDL_Texture* end_menu = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/endmenu.png");
	SDL_Texture* pause_menu = IMG_LoadTexture(renderer, "ISAAC/Backgrounds/pausescreen.png");

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
	int isClear = 1;// �Ƿ����������

    while (!isquit) {

		if (isPause) {
			// ��ͣ����
			Mix_PauseMusic();
			// ��ʾ��ͣ����
			renderPauseScreen(renderer, ttfFont, window_width, window_height,pause_menu);
			while (isPause) {
				while (SDL_PollEvent(&event)) {
					if (event.type == SDL_QUIT) {
						isquit = true;
                        isPause = false;
					}
					if (event.type == SDL_KEYDOWN) {
						if (event.key.keysym.sym == SDLK_ESCAPE) {
							// ������Ϸ
							Mix_ResumeMusic();
                            isPause = false;
						}
					}
				}
			}
		}









        /*��������*/
        if (isaac.HP <= 0) {
			// ����������Ч
			playDeathGrunt();
            Mix_PlayChannel(-1, death_grunt, 0);
            // ����������������
            playEndingSound();
            Mix_PlayMusic(ending_sound, -1);
            // ��ʾ��������
            renderGameOverScreen(renderer, ttfFont, window_width, window_height,end_menu);
            // �ȴ��û����»س������¿�ʼ����ESC���˳���Ϸ
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
                            // ���¿�ʼ��Ϸ
                            isaac.HP = 12; // ���ý�ɫѪ��
                            isaac.x = window_width / 2; // ���ý�ɫλ��
                            isaac.y = window_height / 2;
                            headrect.x = window_width / 2 - headrect.w / 2;
                            headrect.y = window_height / 2 - headrect.h / 2 - 12;
                            bodyrect.x = window_width / 2 - bodyrect.w / 2;
                            bodyrect.y = window_height / 2 - bodyrect.h / 2 + 12;
                            Bullets.clear(); // ����ӵ�
                            FlyBullets.clear(); // ��չ����ӵ�
                            Flies.clear(); // ��ղ�Ӭ��
							BigFlies.clear(); // ��մ��Ӭ��
                            room_number = 0; // ���÷����
							isClear = 1; // �����Ƿ����������
                            generateMonsters(room_number, isaac); // �������ɹ���
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

        // �����¼�
        processInput(event, isquit, keyStates);

        // ÿ֡�������
        processShooting(keyStates, head_direction, headrect, isaac, last_shoot_time, is_attacking, attack_start_time);

        // ��������ӵ�
        updateBullets(Bullets, window_width, window_height, BurstMotions, false, isaac);
        // ���¹����ӵ�
        updateBullets(FlyBullets, window_width, window_height, EnemyBurstMotions, true, isaac);


        // ���¹���λ�ú�״̬
        updateMonsters(Flies, BigFlies, isaac, window_width, window_height);


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
        if (switching_room && SDL_GetTicks() - switch_start_time >= 1000) { // �ӳ��� 1 ��
            switchRoom(renderer, basement, headrect, bodyrect,isaac, fly_standard);
            switching_room = false;
            black_screen = false;
			int obstacle_number = randomNumberOneToSeven(); // �������1-7���ϰ���
            generateObstacles(obstacle_number, obstacleTexture); // ����1���ϰ���
            // ��������µĹ���
            generateMonsters(room_number, isaac);
			room_number++;
        }

        

        // ��Ⱦ����
        SDL_RenderClear(renderer);
        if (black_screen) {
            SDL_RenderCopy(renderer, black, NULL, NULL);
        }
        else {
            SDL_RenderCopy(renderer, basement, NULL, NULL);
        }

        // ��Ⱦ�ϰ���
        for (const auto& obstacle : Obstacles) {
            SDL_RenderCopy(renderer, obstacle.texture, NULL, &obstacle.rect);
        }





		// ����Ƿ��������������򲥷�ʤ������
		if (Flies.empty() && BigFlies.empty()) {
            if (isClear == room_number) {
                playClearMusic();
                Mix_PlayChannel(-1, clear_music, 0);
				isClear++;
            }
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
            SDL_RenderDrawRect(renderer, &isaac.bumpbox);
            // ���Ʋ�Ӭ�ֵ���ײ��
            for (const auto& fly : Flies) {
                SDL_RenderDrawRect(renderer, &fly.bumpbox);
            }
			// ���ƴ��Ӭ�ֵ���ײ��
			for (const auto& bigFly : BigFlies) {
				SDL_RenderDrawRect(renderer, &bigFly.bumpbox);
			}
            // �����ӵ�����ײ��
            for (const auto& bullet : Bullets) {
                SDL_RenderDrawRect(renderer, &bullet.bumpbox);
            }
            // �����ϰ������ײ��
            for (const auto& obstacle : Obstacles) {
                SDL_RenderDrawRect(renderer, &obstacle.collisionRect);
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
                if (fly.isDying) {
                    // ��Ⱦ��������
                    flyDeadMotion(renderer, FlyDeadMotions, fly.spriteRect, fly);
                }
                else if (fly.state == ATTACK) {
                    flyAttackMotion(renderer, FlyMotions, _FlyMotions, fly.spriteRect, fly, isaac);
                }
                else {
                    flyIdleMotion(renderer, FlyMotions, _FlyMotions, fly.spriteRect, fly, isaac);
                }
            }
            // ��Ⱦ���Ӭ��
            for (auto& bigFly : BigFlies) {
				// ��Ⱦ��������
                if (bigFly.isDying) {
                    bigFlyDeadMotion(renderer, FlyDeadMotions,bigFly.spriteRect, bigFly);
				}
                else {
                    bigFlyMotion(renderer, BigFlyMotions, bigFly.spriteRect, bigFly);
                }
            }

            // ��Ⱦ����ӵ�
            renderBullets(renderer, Bullets, bullet_texture, BurstMotions);
            // ��Ⱦ�з��ӵ�
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



