#include "tank.hpp"
#include "texture_id.hpp"
#include "data_tables.hpp"
#include "utility.hpp"
#include "constants.hpp"
#include <iostream>


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

Tank::Tank(Type type, const TextureHolder& textures)
	:Entity(Table[static_cast<int>(type)].m_hitpoints)
	, m_type(type)
	, m_sprite(textures.Get(Table[static_cast<int>(type)].m_texture))
	, m_turret_sprite(nullptr)

{

	Utility::CentreOrigin(m_sprite);

	// 2. Setup Turret
	// We create a SpriteNode and attach it to the tank
	const sf::Texture& turretTexture = textures.Get(TextureID::kTankTurret);
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
	Entity::UpdateCurrent(dt, commands);
}

unsigned int Tank::GetCategory() const
{
	return static_cast<unsigned int>(ReceiverCategories::kPlayerTank);
}