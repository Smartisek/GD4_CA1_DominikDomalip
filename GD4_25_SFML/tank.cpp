#include "tank.hpp"
#include "texture_id.hpp"
#include "data_tables.hpp"
#include "projectile.hpp"
#include "utility.hpp"
#include "constants.hpp"
#include <iostream>
#include <cmath>
#include "sound_node.hpp"

namespace
{
	const std::vector<TankData> Table = InitializeTankData();
}

Tank::Tank(TankType type, const TextureHolder& textures, const FontHolder& fonts, ReceiverCategories category)
	:Entity(Table[static_cast<int>(type)].m_hitpoints,
		Table[static_cast<int>(type)].m_max_stamina,
		Table[static_cast<int>(type)].m_drain_rate,
		Table[static_cast<int>(type)].m_recharge_rate,
		Table[static_cast<int>(type)].m_sprint_multiplier
	)
	, m_type(type)
	, m_sprite(textures.Get(Table[static_cast<int>(type)].m_texture))
	, m_turret_sprite(nullptr)
	, m_category(category)
	, m_is_firing(false)
	, m_fire_countdown(sf::Time::Zero)
	, m_fire_rate(1)
	, m_current_ammo(Table[static_cast<int>(type)].m_ammo_amount)
	, m_collision_cooldown(sf::Time::Zero)
	, m_max_hitpoints(Table[static_cast<int>(type)].m_hitpoints)
	, m_show_explosion(true)
	, m_explosion(textures.Get(TextureID::kExplosion))
	, m_explosion_began(false)
	, m_show_fire_animation(false)
	, m_fire_animation(textures.Get(TextureID::kTankFireAnim))
	, m_ammo_icon(textures.Get(TextureID::kBulletUI))
	, m_ammo_text(fonts.Get(FontID::kMain))
{

	m_explosion.SetFrameSize(sf::Vector2i(256,256));
	m_explosion.SetNumFrames(16);
	m_explosion.SetDuration(sf::seconds(1));

	m_fire_animation.SetFrameSize(sf::Vector2i(256, 256));
	m_fire_animation.SetNumFrames(4);
	m_fire_animation.SetDuration(sf::seconds(0.2f));
	m_fire_animation.setPosition(sf::Vector2f(0.f, -120.f));
	

	Utility::CentreOrigin(m_sprite);
	Utility::CentreOrigin(m_explosion);
	Utility::CentreOrigin(m_fire_animation);

	m_fire_command.category = static_cast<int>(ReceiverCategories::kScene);
	m_fire_command.action = [this, &textures](SceneNode& node, sf::Time dt)
		{
			CreateBullet(node, textures);
		};

	//set up tanks health bar 
	m_health_bar_background.setSize(sf::Vector2f(150.f, 15.f));
	m_health_bar_background.setFillColor(sf::Color(50, 50, 50, 200));

	m_health_bar_foreground.setSize(sf::Vector2f(150.f, 15.f));
	m_health_bar_foreground.setFillColor(sf::Color(178, 34, 34));

	sf::Vector2f barPos = { 0.f, 150.f }; // Adjust -50.f based on tank size
	m_health_bar_background.setOrigin(m_health_bar_background.getSize() / 2.f);
	m_health_bar_foreground.setOrigin(m_health_bar_foreground.getSize() / 2.f);

	m_health_bar_background.setPosition(barPos);
	m_health_bar_foreground.setPosition(barPos);

	//set up stamina bar 
	m_stamina_bar_background.setSize(sf::Vector2f(150.f, 8.f));
	m_stamina_bar_background.setFillColor(sf::Color(50, 50, 50, 200));
	m_stamina_bar_background.setOrigin(m_stamina_bar_background.getSize() / 2.f);

	
	m_stamina_bar_foreground.setSize(sf::Vector2f(150.f, 8.f));
	m_stamina_bar_foreground.setFillColor(sf::Color::Cyan); 
	m_stamina_bar_foreground.setOrigin(m_stamina_bar_foreground.getSize() / 2.f);

	
	sf::Vector2f staminaPos = { 0.f, 170.f };
	m_stamina_bar_background.setPosition(staminaPos);
	m_stamina_bar_foreground.setPosition(staminaPos);

	//Ammo showup ui
	m_ammo_icon.setScale(sf::Vector2f(1.f, 1.f));
	m_ammo_icon.setPosition(sf::Vector2f(-60.f, 190.f));

	m_ammo_text.setCharacterSize(30);
	m_ammo_text.setFillColor(sf::Color::Black);
	m_ammo_text.setString("x " + std::to_string(m_current_ammo));
	m_ammo_text.setPosition({ -10.f, 185.f });

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

	if (IsDestroyed() && m_show_explosion)
	{
		target.draw(m_explosion, states);
	}


	if (!IsDestroyed())
	{
		target.draw(m_health_bar_background, states);
		target.draw(m_health_bar_foreground, states);

		target.draw(m_stamina_bar_background, states);
		target.draw(m_stamina_bar_foreground, states);

		target.draw(m_ammo_icon, states);
		target.draw(m_ammo_text, states);

		target.draw(m_sprite, states);

		if (m_show_fire_animation)
		{
			
			target.draw(m_fire_animation, states);
		}
	}
	

}

void Tank::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	//Logic for the tank rotation 
	sf::Vector2f velocity = GetVelocity();

	Entity::UpdateStamina(dt);
	UpdateHealthBar();
	UpdateStaminaBar();

	if (std::abs(velocity.x) > 1.0f || std::abs(velocity.y) > 1.0f) //check if we are moving 
	{
		//atan2 function takes in y and x coordinates and gives angle of connecting line (0,0) to that point 
		// https://www.w3schools.com/cpp/ref_math_atan2.asp
		float radians = std::atan2(velocity.y, velocity.x); 
		float degrees = Utility::ToDegrees(radians);
		//move to that angle
		setRotation(sf::degrees(degrees + 90.f));
	}

	if (IsDestroyed())
	{
		m_explosion.Update(dt);
		if (!m_explosion_began)
		{
			//I have to remove the turret sprite because otherwise body will disapear, animation play and turret stay, after finishing animation only then will turret disappear, this fixes that 
			DetachChild(*m_turret_sprite); 
			m_turret_sprite = nullptr;

			m_explosion_began = true;
		}
		return;
	}

	if (m_show_fire_animation)
	{
		m_fire_animation.Update(dt);
		if (m_fire_animation.IsFinished())
		{
			m_show_fire_animation = false;
		}
	}

	if (!IsDestroyed())
	{
		m_ammo_text.setString("x " + std::to_string(m_current_ammo));
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
	if (m_category == ReceiverCategories::kPlayer1Tank)
	{
		projType = ProjectileType::kPlayer1Bullet;
	}
	else
	{
		projType = ProjectileType::kPlayer2Bullet;
	}

	ReceiverCategories owner;
	if (m_category == ReceiverCategories::kPlayer1Tank)
	{
		owner = ReceiverCategories::kPlayer1Projectile;
	}
	else
	{
		owner = ReceiverCategories::kPlayer2Projectile;
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
	bullet->setPosition(GetWorldPosition() + direction * 100.f); 
	bullet->setRotation(rotation);                            
	bullet->SetVelocity(direction * bullet->GetMaxSpeed());

	node.AttachChild(std::move(bullet));
}

void Tank::CheckProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
	if (m_fire_countdown > sf::Time::Zero)
	{
		m_fire_countdown -= dt;
	}

	if (m_is_firing)
	{
		if (m_fire_countdown <= sf::Time::Zero && m_current_ammo > 0)
		{
			commands.Push(m_fire_command);
			SoundEffect soundEffect = SoundEffect::kTankBulletFire;
			PlayLocalSound(commands, soundEffect);

			m_show_fire_animation = true;
			m_fire_animation.Restart();

			m_current_ammo--;
			m_fire_countdown += Table[static_cast<int>(m_type)].m_fire_interval / (m_fire_rate + 1.f);
		} //possible sound in else
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

void Tank::UpdateHealthBar()
{
	//get the percebtage 
	float healthRatio = static_cast<float>(GetHitPoints()) / m_max_hitpoints;

	//apply the green health bar 
	m_health_bar_foreground.setSize(sf::Vector2f(150.f * healthRatio, 15.f));
}

float Tank::GetSpeed() const
{
	return Table[static_cast<int>(m_type)].m_speed;
}

void Tank::UpdateStaminaBar()
{
	float ratio = GetStaminaRatio();

	m_stamina_bar_foreground.setSize(sf::Vector2f(150.f * ratio, 8.f));
}

void Tank::PlayLocalSound(CommandQueue& commands, SoundEffect effect)
{
	sf::Vector2f world_position = GetWorldPosition();

	Command command;
	command.category = static_cast<int>(ReceiverCategories::kSoundEffect);
	command.action = DerivedAction<SoundNode>(
		[effect, world_position](SoundNode& node, sf::Time)
		{
			node.PlaySound(effect, world_position);
		});
	commands.Push(command);
}

bool Tank::IsMarkedForRemoval() const
{
	return IsDestroyed() && (m_explosion.IsFinished() || !m_show_explosion);
}

void Tank::Repair(int points)
{
	//how much to heal
	int currentHealth = GetHitPoints();
	int missingHealth = m_max_hitpoints - currentHealth;

	int amountToHeal = std::min(points, missingHealth);
	if (amountToHeal > 0)
	{
		Entity::Repair(amountToHeal);
	}
}

int Tank::GetAmmoCount() const
{
	return m_current_ammo;
}

void Tank::Reload(int amount)
{
	int maxAmmo = Table[static_cast<int>(m_type)].m_ammo_amount;
	m_current_ammo = std::min(m_current_ammo + amount, maxAmmo);
}