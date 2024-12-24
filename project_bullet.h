#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <random>
#include "project_monster.h"
using namespace std;
#define PI 3.14159265358979323846




/* �������������������� �ӵ��ඨ�� �������������������� */
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

    //���������ӵ���Ϊ����

    //�ӵ�����λ��
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





/* �������������������� �ӵ��¼�����Ͷ�����Ⱦ �������������������� */

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

// �����ӵ�λ�ã����һ��������������������ҵ��ӵ����ǹ�����ӵ�
void updateBullets(vector<BULLET>& bullets, int window_width, int window_height, vector<SDL_Texture*>& BurstMotions, bool isEnemyBullet, PLAYER& player, vector<FLY>& Flies) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        it->update();

        // ����ӵ��Ƿ�����ǽ�ڣ����ڱ߽磩�������������
        if (!it->isBursting && (it->bumpbox.x < 0 || it->bumpbox.x > window_width - it->bumpbox.w ||
            it->bumpbox.y < 0 || it->bumpbox.y > window_height - it->bumpbox.h)) {
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
            SDL_RenderCopy(renderer, BurstMotions[frame], NULL, &bullet.bumpbox);
        }
        else {
            SDL_RenderCopy(renderer, bulletTexture, NULL, &bullet.bumpbox);
        }
    }
}


