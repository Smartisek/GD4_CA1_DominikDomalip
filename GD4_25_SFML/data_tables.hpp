#pragma once
#include "texture_id.hpp"
#include <SFML/System/Time.hpp>
#include <functional>

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
};

struct ProjectileData
{
	int m_damage;
	float m_speed;
	TextureID m_texture;
};

std::vector<ProjectileData> InitializeProjectileData();
std::vector<TankData> InitializeTankData();





