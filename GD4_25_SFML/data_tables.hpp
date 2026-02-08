#pragma once
#include "texture_id.hpp"
#include <SFML/System/Time.hpp>
#include <functional>
#include <SFML/Graphics/Color.hpp>
#include "animation.hpp"
#include "pickup_type.hpp"
#include "tank.hpp"

struct Direction
{
	Direction(float angle, float distance)
		: m_angle(angle), m_distance(distance) {
	}
	float m_angle;
	float m_distance;
};


struct TankData
{
	int m_hitpoints;
	float m_speed;
	TextureID m_texture;
	TextureID m_texture_turret;
	sf::Time m_fire_interval;
	std::vector<Direction> m_directions;
	float m_max_stamina;
	float m_sprint_multiplier;
	float m_drain_rate;
	float m_recharge_rate;
	int m_ammo_amount;
};

struct ProjectileData
{
	int m_damage;
	float m_speed;
	TextureID m_texture;

};

struct ParticleData
{
	sf::Color m_color;
	sf::Time m_lifetime;
};

struct PickupData
{
	TextureID m_texture;
	std::function<void(Tank&)> m_action;
};

struct MapData
{
	sf::IntRect m_texture_rect;
	sf::Color   m_ui_color;
};

std::vector<ProjectileData> InitializeProjectileData();
std::vector<TankData> InitializeTankData();
std::vector<ParticleData> InitializeParticleData();
std::vector<PickupData> InitializePickupData();
std::vector<MapData> InitializeMapData();




