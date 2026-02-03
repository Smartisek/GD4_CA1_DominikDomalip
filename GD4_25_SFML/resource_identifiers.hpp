#pragma once
#include "texture_id.hpp"
#include "resource_holder.hpp"
#include <SFML/Graphics/Font.hpp>
#include "fontID.hpp"
#include "sound_effect.hpp"

namespace sf
{
	class Texture;
	class Font;
	class SoundBuffer;
}

//template<typename Identifier, typename Resource>

typedef ResourceHolder<TextureID, sf::Texture> TextureHolder;
typedef ResourceHolder<FontID, sf::Font> FontHolder;
typedef ResourceHolder<SoundEffect, sf::SoundBuffer> SoundBufferHolder;