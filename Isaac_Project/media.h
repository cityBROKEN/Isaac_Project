#pragma once
#include "framwork.h"


/* —————————— 视频 —————————— */
void playVideo(const char* videoPath, SDL_Renderer* renderer);

/* —————————— 声音 —————————— */
// 声明音乐和音效全局变量
extern Mix_Music* main_music;
extern Mix_Music* opening_video_sound;
extern Mix_Music* title_screen_sound;
extern Mix_Music* ending_sound;

extern Mix_Chunk* hurt_grunt;
extern Mix_Chunk* death_grunt;
extern Mix_Chunk* shoot_sound1;
extern Mix_Chunk* shoot_sound2;
extern Mix_Chunk* clear_music;
extern Mix_Chunk* monster_death;
extern Mix_Chunk* monster_swarm;

// 声明音频播放函数
void playOpeningVideoSound();
void playMainMusic();
void playTitleScreenSound();
void playEndingSound();
void playHurtGrunt();
void playDeathGrunt();
void playShootSound1();
void playShootSound2();
void playClearMusic();
void playMonsterDeath();
void playMonsterSwarm();
void modifyVolume();

// 声明音频资源初始化和释放函数
bool initAudio();
void cleanupAudio();