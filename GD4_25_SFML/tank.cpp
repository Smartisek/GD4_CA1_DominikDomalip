#include "tank.hpp"
#include "texture_id.hpp"
#include "data_tables.hpp"
#include "projectile.hpp"
#include "utility.hpp"
#include "constants.hpp"
#include <iostream>
#include <cmath>


// will change this to decide on the texture for player1 and player2 tanks later on
//TextureID ToTextureID(AircraftType type)
//{
//	switch (type)
//	{
//	case AircraftType::kEagle:
//		return TextureID::kEagle;
//		break;
//	case AircraftType::kRaptor:
//		return TextureID::kRaptor;
//		break;
//	}
//	return TextureID::kEagle;
//}

namespace
{
	const std::vector<TankData> Table = InitializeTankData();
}

Tank::Tank(TankType type, const TextureHolder& textures, ReceiverCategories category)
	:Entity(Table[static_cast<int>(type)].m_hitpoints)
	, m_type(type)
	, m_sprite(textures.Get(Table[static_cast<int>(type)].m_texture))
	, m_turret_sprite(nullptr)
	, m_category(category)
	, m_is_firing(false)
	, m_fire_countdown(sf::Time::Zero)
	, m_fire_rate(1)
	, m_collision_cooldown(sf::Time::Zero)
{

	Utility::CentreOrigin(m_sprite);

	m_fire_command.category = static_cast<int>(ReceiverCategories::kScene);
	m_fire_command.action = [this, &textures](SceneNode& node, sf::Time dt)
		{
			CreateBullet(node, textures);
		};

	// 2. Setup Turret
	// We create a SpriteNode and attach it to the tank
	const sf::Texture& turretTexture = textures.Get(Table[static_cast<int>(m_type)].m_texture_turret);
	std::unique_ptr<SpriteNode> turret(new SpriteNode(turretTexture));
	m_turret_sprite = turret.get();

	// Center Turret Origin
	sf::Vector2u texSize = turretTexture.getSize();
	m_turret_sprite->setOrigin(sf::Vector2f(texSize.x / 2.f, texSize.y / 2.f));

	// Attach turret to Body
	AttachChild(std::move(turret));
}

void Tank::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	//turret should get drawn by the scenegraph
	target.draw(m_sprite, states);
}

void Tank::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	//Logic for the tank rotation 
	sf::Vector2f velocity = GetVelocity();

	if (std::abs(velocity.x) > 0.1f || std::abs(velocity.y) > 0.1f) //check if we are moving 
	{
		//atan2 function takes in y and x coordinates and gives angle of connecting line (0,0) to that point 
		// https://www.w3schools.com/cpp/ref_math_atan2.asp
		float radians = std::atan2(velocity.y, velocity.x); 
		float degrees = Utility::ToDegrees(radians);
		//move to that angle
		setRotation(sf::degrees(degrees + 90.f));
	}

	Entity::UpdateCurrent(dt, commands);

	//Check if bullets or missiles were fired
	ReduceCollisionCooldown(dt);
	CheckProjectileLaunch(dt, commands);
}

unsigned int Tank::GetCategory() const
{
	return static_cast<unsigned int>(m_category);
}

void Tank::Fire()
{
	if (m_fire_countdown <= sf::Time::Zero)
	{
		m_is_firing = true;
	}
}

void Tank::CreateBullet(SceneNode& node, const TextureHolder& textures) const
{
	//checks for the projectile type and owner 
	ProjectileType projType;
	if (m_category == ReceiverCategories::kPlayer1Projectile)
	{
		projType = ProjectileType::kPlayer1Bullet;
	}
	else
	{
		projType = ProjectileType::kPlayer2Bullet;
	}

	ReceiverCategories owner;
	if (m_category == ReceiverCategories::kPlayer2Tank)
	{
		owner = ReceiverCategories::kPlayer2Projectile;
	}
	else
	{
		owner = ReceiverCategories::kPlayer1Projectile;
	}

	std::unique_ptr<Projectile> bullet(new Projectile(projType, textures, owner));
	bullet->setScale(sf::Vector2f(0.7f, 0.7f));
	// decide the rotation for bullet
	sf::Angle rotation = getRotation();
	if (m_turret_sprite)
	{
		rotation += m_turret_sprite->getRotation(); //add them together 
	}
	//conversions 
	float radiansRotation = rotation.asRadians();
	sf::Vector2f direction(std::sin(radiansRotation), -std::cos(radiansRotation));
	// positions rotation and speed 
	bullet->setPosition(GetWorldPosition() + direction * 50.f); 
	bullet->setRotation(rotation);                            
	bullet->SetVelocity(direction * bullet->GetMaxSpeed());

	node.AttachChild(std::move(bullet));
}

void Tank::CheckProjectileLaunch(sf::Time dt, CommandQueue& commands)
{

	if (m_is_firing && m_fire_countdown <= sf::Time::Zero)
	{
		commands.Push(m_fire_command);
		m_fire_countdown += Table[static_cast<int>(m_type)].m_fire_interval / (m_fire_rate + 1.f);
		m_is_firing = false;
	}
	else if (m_fire_countdown > sf::Time::Zero)
	{
		m_fire_countdown -= dt;
		m_is_firing = false;
	}
}

sf::FloatRect Tank::GetBoundingRect() const
{
	 //transform local sprite bounds into world coordinates
	return GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
}

// **FUNCTIONS FOR COLLISION COOLDOWN**
bool Tank::CanBeDamaged() const
{
	return m_collision_cooldown <= sf::Time::Zero;
}

void Tank::ResetCollisionCooldown()
{
	m_collision_cooldown = sf::seconds(1.0f);
}

void Tank::ReduceCollisionCooldown(sf::Time dt)
{
	// check if the cooldown is more than zero so active, we can reduce it being called in update
	if (m_collision_cooldown > sf::Time::Zero)
	{
		m_collision_cooldown -= dt;
		if (m_collision_cooldown < sf::Time::Zero)
			m_collision_cooldown = sf::Time::Zero;
	}
}
