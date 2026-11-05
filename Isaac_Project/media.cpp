#include "media.h"

// M3: SDL3_mixer global objects and persistent tracks
MIX_Mixer* audioMixer = nullptr;
MIX_Track* bgmTrack = nullptr;
MIX_Track* hurtGruntTrack = nullptr;
MIX_Track* deathGruntTrack = nullptr;
MIX_Track* shootSound1Track = nullptr;
MIX_Track* shootSound2Track = nullptr;
MIX_Track* clearMusicTrack = nullptr;
MIX_Track* monsterDeathTrack = nullptr;
MIX_Track* monsterSwarmTrack = nullptr;


/* —————————— 声音 —————————— */
// 定义音乐和音效全局变量
MIX_Audio* main_music = nullptr; //主音乐
MIX_Audio* opening_video_sound = nullptr; //开场音乐
MIX_Audio* title_screen_sound = nullptr; //标题音乐
MIX_Audio* ending_sound = nullptr; //结束音乐

MIX_Audio* hurt_grunt = nullptr; //受伤音效
MIX_Audio* death_grunt = nullptr; //死亡音效
MIX_Audio* shoot_sound1 = nullptr; //射击音效
MIX_Audio* shoot_sound2 = nullptr; //射击音效
MIX_Audio* clear_music = nullptr; //清除房间音效
MIX_Audio* monster_death = nullptr; //怪物死亡音效
MIX_Audio* monster_swarm = nullptr; //怪物音效

// 实现音频播放函数
void playOpeningVideoSound() { opening_video_sound = MIX_LoadAudio(audioMixer, "ISAAC/Sounds/openingvideo.wav", false); }
void playMainMusic() { main_music = MIX_LoadAudio(audioMixer, "ISAAC/Sounds/burning basement.mp3", false); }
void playTitleScreenSound() { title_screen_sound = MIX_LoadAudio(audioMixer, "ISAAC/Sounds/title screen.mp3", false); }
void playEndingSound() { ending_sound = MIX_LoadAudio(audioMixer, "ISAAC/Sounds/you died.ogg", false); }
void playHurtGrunt() { hurt_grunt = MIX_LoadAudio(audioMixer, "ISAAC/Sounds/hurt grunt.wav", true); }
void playDeathGrunt() { death_grunt = MIX_LoadAudio(audioMixer, "ISAAC/Sounds/isaac dies new.wav", true); }
void playShootSound1() { shoot_sound1 = MIX_LoadAudio(audioMixer, "ISAAC/Sounds/tear fire 4.wav", true); }
void playShootSound2() { shoot_sound2 = MIX_LoadAudio(audioMixer, "ISAAC/Sounds/tear fire 5.wav", true); }
void playClearMusic() { clear_music = MIX_LoadAudio(audioMixer, "ISAAC/Sounds/danglewhistle.wav", true); }
void playMonsterDeath() { monster_death = MIX_LoadAudio(audioMixer, "ISAAC/Sounds/death burst small 3.wav", true); }
void playMonsterSwarm() { monster_swarm = MIX_LoadAudio(audioMixer, "ISAAC/Sounds/insect swarm.wav", true); }

void modifyVolume()
{
    // SDL2 music volume only affected music; a mixer-wide gain would also mute SFX.
    MIX_SetTrackGain(bgmTrack, 30.0f / 128.0f);
    MIX_SetTrackGain(shootSound1Track, 100.0f / 128.0f);
    MIX_SetTrackGain(shootSound2Track, 100.0f / 128.0f);
    MIX_SetTrackGain(hurtGruntTrack, 128.0f / 128.0f);
    MIX_SetTrackGain(deathGruntTrack, 80.0f / 128.0f);
    MIX_SetTrackGain(clearMusicTrack, 128.0f / 128.0f);
    MIX_SetTrackGain(monsterDeathTrack, 100.0f / 128.0f);
    MIX_SetTrackGain(monsterSwarmTrack, 0.0f / 128.0f);
}

bool initAudio()
{
    if (!MIX_Init())
    {
        SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s", SDL_GetError());
        return false;
    }

    SDL_AudioSpec spec = {};
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;
    spec.freq = 44100;
    audioMixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    if (!audioMixer)
    {
        SDL_Log("SDL_mixer could not create mixer device! SDL_mixer Error: %s", SDL_GetError());
        return false;
    }

    bgmTrack = MIX_CreateTrack(audioMixer);
    hurtGruntTrack = MIX_CreateTrack(audioMixer);
    deathGruntTrack = MIX_CreateTrack(audioMixer);
    shootSound1Track = MIX_CreateTrack(audioMixer);
    shootSound2Track = MIX_CreateTrack(audioMixer);
    clearMusicTrack = MIX_CreateTrack(audioMixer);
    monsterDeathTrack = MIX_CreateTrack(audioMixer);
    monsterSwarmTrack = MIX_CreateTrack(audioMixer);
    return true;
}

void cleanupAudio()
{
    MIX_DestroyAudio(main_music);
    MIX_DestroyAudio(opening_video_sound);
    MIX_DestroyAudio(title_screen_sound);
    MIX_DestroyAudio(ending_sound);
    MIX_DestroyAudio(death_grunt);
    MIX_DestroyAudio(hurt_grunt);
    MIX_DestroyAudio(shoot_sound1);
    MIX_DestroyAudio(shoot_sound2);
    MIX_DestroyAudio(clear_music);
    MIX_DestroyAudio(monster_death);
    MIX_DestroyAudio(monster_swarm);
    MIX_DestroyTrack(bgmTrack);
    MIX_DestroyTrack(hurtGruntTrack);
    MIX_DestroyTrack(deathGruntTrack);
    MIX_DestroyTrack(shootSound1Track);
    MIX_DestroyTrack(shootSound2Track);
    MIX_DestroyTrack(clearMusicTrack);
    MIX_DestroyTrack(monsterDeathTrack);
    MIX_DestroyTrack(monsterSwarmTrack);
    MIX_DestroyMixer(audioMixer);
    MIX_Quit();
}


// M3: BGM uses a persistent looping track, matching the SDL2 looped music call
void playMusic(MIX_Audio* audio)
{
    if (!bgmTrack) return;
    MIX_SetTrackAudio(bgmTrack, audio);
    MIX_SetTrackLoops(bgmTrack, -1);
    MIX_PlayTrack(bgmTrack, 0);
}

// M3: SFX use per-effect tracks so SDL2 per-chunk volumes stay per-effect
void playSound(MIX_Audio* audio)
{
    MIX_Track* track = nullptr;
    if (audio == hurt_grunt) track = hurtGruntTrack;
    else if (audio == death_grunt) track = deathGruntTrack;
    else if (audio == shoot_sound1) track = shootSound1Track;
    else if (audio == shoot_sound2) track = shootSound2Track;
    else if (audio == clear_music) track = clearMusicTrack;
    else if (audio == monster_death) track = monsterDeathTrack;
    else if (audio == monster_swarm) track = monsterSwarmTrack;
    if (!track)
    {
        MIX_PlayAudio(audioMixer, audio);
        return;
    }
    MIX_SetTrackAudio(track, audio);
    MIX_PlayTrack(track, 0);
}

/* —————————— 视频 —————————— */
void playVideo(const char* videoPath, SDL_Renderer* renderer)
{
    avformat_network_init();

    AVFormatContext* formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, videoPath, nullptr, nullptr) != 0)
    {
        SDL_Log("Failed to open video file: %s", videoPath);
        return;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0)
    {
        SDL_Log("Failed to retrieve stream info from video file: %s", videoPath);
        return;
    }

    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++)
    {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1)
    {
        SDL_Log("Failed to find video stream in file: %s", videoPath);
        return;
    }

    AVCodecParameters* videoCodecParameters = formatContext->streams[videoStreamIndex]->codecpar;
    const AVCodec* videoCodec = avcodec_find_decoder(videoCodecParameters->codec_id);
    if (!videoCodec)
    {
        SDL_Log("Failed to find codec for video file: %s", videoPath);
        return;
    }

    AVCodecContext* videoCodecContext = avcodec_alloc_context3(videoCodec);
    if (avcodec_parameters_to_context(videoCodecContext, videoCodecParameters) < 0)
    {
        SDL_Log("Failed to copy codec parameters to codec context for video file: %s", videoPath);
        return;
    }

    if (avcodec_open2(videoCodecContext, videoCodec, nullptr) < 0)
    {
        SDL_Log("Failed to open codec for video file: %s", videoPath);
        return;
    }

    AVFrame* frame = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();
    struct SwsContext* swsContext = sws_getContext(
        videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,
        videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

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
    while (isPlaying && av_read_frame(formatContext, packet) >= 0)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                isPlaying = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE)
            {
                isPlaying = false;
            }
        }

        if (packet->stream_index == videoStreamIndex)
        {
            if (avcodec_send_packet(videoCodecContext, packet) >= 0)
            {
                while (avcodec_receive_frame(videoCodecContext, frame) >= 0)
                {
                    uint8_t* data[3] = { frame->data[0], frame->data[1], frame->data[2] };
                    int linesize[3] = { frame->linesize[0], frame->linesize[1], frame->linesize[2] };
                    SDL_UpdateYUVTexture(texture, &videoRect, data[0], linesize[0], data[1], linesize[1], data[2],
                        linesize[2]);

                    SDL_RenderClear(renderer);
                    renderTexture(renderer, texture, nullptr, &videoRect);
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


