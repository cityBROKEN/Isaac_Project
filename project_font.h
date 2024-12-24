#pragma once
#include <SDL.h>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <SDL_ttf.h>
#include <SDL_image.h>


/* �������������������� ���� �������������������� */
// �ַ��ṹ��
struct Character {
    SDL_Texture* texture;  // �ַ�����
    SDL_Rect srcRect;      // �ַ��������е�λ�úʹ�С
    int xoffset, yoffset;  // ��Ⱦʱ��ƫ��
    int xadvance;          // ��һ���ַ���ˮƽƫ��
};

// �ַ�ӳ��ȫ���������
TTF_Font* ttfFont = nullptr;
SDL_Texture* fontTexture = nullptr;
std::map<int, Character> Characters;


// �������庯��
bool loadFont(const std::string& fontPath, SDL_Renderer* renderer, int fontSize = 24) {
    // �ж��ļ���׺
    std::string extension = fontPath.substr(fontPath.find_last_of('.') + 1);
    if (extension == "ttf") {
        // ʹ�� TTF ����
        ttfFont = TTF_OpenFont(fontPath.c_str(), fontSize);
        if (!ttfFont) {
            SDL_Log("Failed to load TTF font: %s\n", TTF_GetError());
            return false;
        }
        return true;
    }
    else if (extension == "fnt") {
        // ʹ�� BMFont ���壨.fnt + .png��
        std::string texturePath = fontPath.substr(0, fontPath.find_last_of('.')) + "_0.png";
        // ������������
        SDL_Surface* surface = IMG_Load(texturePath.c_str());
        if (!surface) {
            SDL_Log("Failed to load font texture: %s", IMG_GetError());
            return false;
        }
        fontTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!fontTexture) {
            SDL_Log("Failed to create font texture: %s", SDL_GetError());
            return false;
        }

        // �� `.fnt` �ļ�
        std::ifstream fontFile(fontPath);
        if (!fontFile.is_open()) {
            SDL_Log("Failed to open font file: %s", fontPath.c_str());
            return false;
        }

        std::string line;
        while (std::getline(fontFile, line)) {
            // ���Ұ����ַ���Ϣ����
            if (line.substr(0, 4) == "char") {
                Character ch;
                int id;
                sscanf_s(line.c_str(), "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d",
                    &id, &ch.srcRect.x, &ch.srcRect.y, &ch.srcRect.w, &ch.srcRect.h,
                    &ch.xoffset, &ch.yoffset, &ch.xadvance);
                ch.texture = fontTexture;
                Characters[id] = ch;
            }
        }
        fontFile.close();
        return true;
    }
    else {
        SDL_Log("Unsupported font format: %s", extension.c_str());
        return false;
    }
}


// ��Ⱦ���ֺ���
void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
    if (ttfFont) {
        // ʹ�� TTF ������Ⱦ
        SDL_Surface* textSurface = TTF_RenderText_Blended(ttfFont, text.c_str(), color);
        if (!textSurface) {
            SDL_Log("Unable to render text surface: %s\n", TTF_GetError());
            return;
        }

        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);
        if (!textTexture) {
            SDL_Log("Unable to create texture from rendered text: %s\n", SDL_GetError());
            return;
        }

        SDL_Rect dstRect;
        dstRect.x = x;
        dstRect.y = y;
        SDL_QueryTexture(textTexture, nullptr, nullptr, &dstRect.w, &dstRect.h);
        SDL_RenderCopy(renderer, textTexture, nullptr, &dstRect);

        SDL_DestroyTexture(textTexture);
    }
    else if (fontTexture) {
        // ʹ�� BMFont ������Ⱦ
        int cursorX = x;
        int cursorY = y;
        for (const char& c : text) {
            int ascii = static_cast<int>(c);
            if (Characters.find(ascii) != Characters.end()) {
                Character ch = Characters[ascii];
                SDL_Rect destRect = { cursorX + ch.xoffset, cursorY + ch.yoffset, ch.srcRect.w, ch.srcRect.h };
                SDL_RenderCopy(renderer, ch.texture, &ch.srcRect, &destRect);
                cursorX += ch.xadvance;
            }
        }
    }
    else {
        SDL_Log("No font loaded.");
    }
}