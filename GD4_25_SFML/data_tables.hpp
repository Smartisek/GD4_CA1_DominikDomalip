#pragma once
#include "texture_id.hpp"
#include <SFML/System/Time.hpp>
#include <functional>
#include <SFML/Graphics/Color.hpp>
#include "animation.hpp"

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

std::vector<ProjectileData> InitializeProjectileData();
std::vector<TankData> InitializeTankData();
std::vector<ParticleData> InitializeParticleData();




