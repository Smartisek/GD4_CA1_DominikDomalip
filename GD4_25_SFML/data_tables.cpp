#include "data_tables.hpp"
#include "projectile_type.hpp"
#include "tank_type.hpp"

#include "constants.hpp"

std::vector<TankData> InitializeTankData()
{
	std::vector<TankData> data(static_cast<int>(TankType::kTankCount));
	data[static_cast<int>(TankType::kTank1)].m_hitpoints = 150;
	data[static_cast<int>(TankType::kTank1)].m_speed = 60.f;
	data[static_cast<int>(TankType::kTank1)].m_fire_interval = sf::seconds(1);
	data[static_cast<int>(TankType::kTank1)].m_texture = TextureID::kTankBody;
	data[static_cast<int>(TankType::kTank1)].m_texture_turret = TextureID::kTankTurret;
	data[static_cast<int>(TankType::kTank1)].m_max_stamina = 100.f;
	data[static_cast<int>(TankType::kTank1)].m_sprint_multiplier = 1.05f;
	data[static_cast<int>(TankType::kTank1)].m_drain_rate = 40.f;
	data[static_cast<int>(TankType::kTank1)].m_recharge_rate = 20.f;


	data[static_cast<int>(TankType::kTank2)].m_hitpoints = 150;
	data[static_cast<int>(TankType::kTank2)].m_speed = 60.f;
	data[static_cast<int>(TankType::kTank2)].m_fire_interval = sf::seconds(1);
	data[static_cast<int>(TankType::kTank2)].m_texture = TextureID::kTankBody2;
	data[static_cast<int>(TankType::kTank2)].m_texture_turret = TextureID::kTankTurret2;
	data[static_cast<int>(TankType::kTank2)].m_max_stamina = 100.f;
	data[static_cast<int>(TankType::kTank2)].m_sprint_multiplier = 1.05f;
	data[static_cast<int>(TankType::kTank2)].m_drain_rate = 40.f;
	data[static_cast<int>(TankType::kTank2)].m_recharge_rate = 20.f;

	return data;
}

std::vector<ProjectileData> InitializeProjectileData()
{
	std::vector<ProjectileData> data(static_cast<int>(ProjectileType::kProjectileCount));
	data[static_cast<int>(ProjectileType::kPlayer1Bullet)].m_damage = 10;
	data[static_cast<int>(ProjectileType::kPlayer1Bullet)].m_speed = 800;
	data[static_cast<int>(ProjectileType::kPlayer1Bullet)].m_texture = TextureID::kBullet;

	data[static_cast<int>(ProjectileType::kPlayer2Bullet)].m_damage = 10;
	data[static_cast<int>(ProjectileType::kPlayer2Bullet)].m_speed = 800;
	data[static_cast<int>(ProjectileType::kPlayer2Bullet)].m_texture = TextureID::kBullet;

	data[static_cast<int>(ProjectileType::kMissile)].m_damage = 200;
	data[static_cast<int>(ProjectileType::kMissile)].m_speed = 150;
	/*data[static_cast<int>(ProjectileType::kMissile)].m_texture = TextureID::kMissile;*/


	return data;
}
