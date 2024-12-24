#pragma once
#include <SDL_mixer.h>

/* ！！！！！！！！！！ 蕗咄 ！！！！！！！！！！ */
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