#pragma once

// C 头文件


// C++ 头文件
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

// 第三方库


//SDL头文件
#include <../include/SDL2/SDL.h>
#include <../include/SDL2/SDL_image.h>
#include <../include/SDL2/SDL_mixer.h>
#include <../include/SDL2/SDL_ttf.h>

//ffmpeg头文件
#ifdef __cplusplus
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#endif
#ifdef __cplusplus
}
#endif