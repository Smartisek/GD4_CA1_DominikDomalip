#pragma once
#include "entity.hpp"
#include "text_node.hpp"
#include "sprite_node.hpp"
#include "resource_identifiers.hpp"


class Tank : public Entity
{
public:
	enum Type { kDefault }; //might not need types, we will se later on 

	Tank(Type type, const TextureHolder& textures);
	
	virtual unsigned int GetCategory() const override;

	//more functions will be added later

private:
	virtual void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;

private:
	Type m_type;
	sf::Sprite m_sprite; //body for tank
	SpriteNode* m_turret_sprite; //child node for the tank body 

};

