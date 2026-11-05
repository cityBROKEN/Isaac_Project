#pragma once
#include <SDL3/SDL.h>

// M2-2: SDL3 render APIs require SDL_FRect; game logic keeps integer SDL_Rect.
// Conversion happens only at the render boundary via SDL_RectToFRect.
inline bool renderTexture(SDL_Renderer* renderer, SDL_Texture* texture,
	const SDL_Rect* srcrect, const SDL_Rect* dstrect)
{
	SDL_FRect fsrc = {}, fdst = {};
	if (srcrect) SDL_RectToFRect(srcrect, &fsrc);
	if (dstrect) SDL_RectToFRect(dstrect, &fdst);
	return SDL_RenderTexture(renderer, texture,
		srcrect ? &fsrc : nullptr, dstrect ? &fdst : nullptr);
}

inline bool renderRect(SDL_Renderer* renderer, const SDL_Rect* rect)
{
	SDL_FRect frect = {};
	if (rect) SDL_RectToFRect(rect, &frect);
	return SDL_RenderRect(renderer, rect ? &frect : nullptr);
}

// M2-2: the SDL2 texture-size query was removed; SDL_GetTextureSize reports float.
inline bool queryTextureSize(SDL_Texture* texture, int* w, int* h)
{
	float fw = 0.0f, fh = 0.0f;
	if (!SDL_GetTextureSize(texture, &fw, &fh)) return false;
	if (w) *w = static_cast<int>(fw);
	if (h) *h = static_cast<int>(fh);
	return true;
}
