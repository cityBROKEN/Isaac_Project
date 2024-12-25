#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#ifdef __cplusplus
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#endif
#ifdef __cplusplus
}
#endif




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
Mix_Music* main_music = NULL;//主音乐
Mix_Music* opening_video_sound = NULL;//开场音乐
Mix_Music* title_screen_sound = NULL;//标题音乐
Mix_Music* ending_sound = NULL;//结束音乐

Mix_Chunk* hurt_grunt = NULL;//受伤音效
Mix_Chunk* death_grunt = NULL;//死亡音效
Mix_Chunk* shoot_sound1 = NULL;//射击音效
Mix_Chunk* shoot_sound2 = NULL;//射击音效
Mix_Chunk* clear_music = NULL;//清除房间音效
Mix_Chunk* monster_death = NULL;//怪物死亡音效
Mix_Chunk* monster_swarm = NULL;//怪物音效

void playOpeningVideoSound(){ opening_video_sound = Mix_LoadMUS("ISAAC/Sounds/openingvideo.wav");}
void playMainMusic(){main_music = Mix_LoadMUS("ISAAC/Sounds/burning basement.mp3");}
void playTitleScreenSound() { title_screen_sound = Mix_LoadMUS("ISAAC/Sounds/title screen.mp3"); }
void playEndingSound() { ending_sound = Mix_LoadMUS("ISAAC/Sounds/you died.ogg"); }
void playHurtGrunt() { hurt_grunt = Mix_LoadWAV("ISAAC/Sounds/hurt grunt.wav"); }
void playDeathGrunt() { death_grunt = Mix_LoadWAV("ISAAC/Sounds/isaac dies new.wav"); }
void playShootSound1() { shoot_sound1 = Mix_LoadWAV("ISAAC/Sounds/tear fire 4.wav"); }
void playShootSound2() { shoot_sound2 = Mix_LoadWAV("ISAAC/Sounds/tear fire 5.wav"); }
void playClearMusic() { clear_music = Mix_LoadWAV("ISAAC/Sounds/danglewhistle.wav"); }
void playMonsterDeath() { monster_death = Mix_LoadWAV("ISAAC/Sounds/death burst small 3.wav"); }
void playMonsterSwarm() { monster_swarm = Mix_LoadWAV("ISAAC/Sounds/insect swarm.wav"); }

void modifyVolume() {
    Mix_VolumeMusic(10);
    Mix_VolumeChunk(shoot_sound1,128);
	Mix_VolumeChunk(shoot_sound2, 128);
	Mix_VolumeChunk(hurt_grunt, 128);
	Mix_VolumeChunk(death_grunt, 128);
	Mix_VolumeChunk(clear_music, 128);
	Mix_VolumeChunk(monster_death, 128);
	Mix_VolumeChunk(monster_swarm, 0);
}