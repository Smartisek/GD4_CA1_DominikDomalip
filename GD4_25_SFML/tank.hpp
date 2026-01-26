#pragma once
#include "entity.hpp"
#include "text_node.hpp"
#include "sprite_node.hpp"
#include "resource_identifiers.hpp"
#include "tank_type.hpp"


class Tank : public Entity
{
public:

	Tank(TankType type, const TextureHolder& textures, ReceiverCategories category = ReceiverCategories::kPlayer1Tank);
	
	virtual unsigned int GetCategory() const override;

	//more functions will be added later
	void CreateBullet(SceneNode& node, const TextureHolder& textures) const;
	void Fire();

	// cooldown for bumping into other tanks
	bool CanBeDamaged() const;
	void ResetCollisionCooldown();
	void ReduceCollisionCooldown(sf::Time dt);
	sf::FloatRect GetBoundingRect() const;

private:
	virtual void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;
	void CheckProjectileLaunch(sf::Time dt, CommandQueue& commands);
	

private:
	TankType m_type;
	sf::Sprite m_sprite; //body for tank
	SpriteNode* m_turret_sprite; //child node for the tank body 
	ReceiverCategories m_category; // what ccategory this tank belongs to

	//fire control
	bool m_is_firing;
	sf::Time m_fire_countdown;
	unsigned int m_fire_rate;
	Command m_fire_command;

	sf::Time m_collision_cooldown;
};

