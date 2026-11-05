#pragma once
#include "framework.h"


/* —————————— 视频 —————————— */
void playVideo(const char* videoPath, SDL_Renderer* renderer);

/* —————————— 声音 —————————— */
// 声明音乐和音效全局变量
extern MIX_Mixer* audioMixer;
extern MIX_Audio* main_music;
extern MIX_Audio* opening_video_sound;
extern MIX_Audio* title_screen_sound;
extern MIX_Audio* ending_sound;

extern MIX_Audio* hurt_grunt;
extern MIX_Audio* death_grunt;
extern MIX_Audio* shoot_sound1;
extern MIX_Audio* shoot_sound2;
extern MIX_Audio* clear_music;
extern MIX_Audio* monster_death;
extern MIX_Audio* monster_swarm;

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

// M3: SDL3_mixer track wrappers (keep original call-site semantics)
void playMusic(MIX_Audio* audio);
void playSound(MIX_Audio* audio);

// 声明音频资源初始化和释放函数
bool initAudio();
void cleanupAudio();