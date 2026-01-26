#include "world.hpp"
#include "sprite_node.hpp"
#include <iostream>
#include "state.hpp"
#include <SFML/System/Angle.hpp>
#include "tank.hpp"
#include "projectile.hpp"


World::World(sf::RenderWindow& window, FontHolder& font)
	: m_window(window)
	, m_camera(window.getDefaultView())
	, m_textures()
	, m_fonts(font)
	, m_scene_graph(ReceiverCategories::kNone)
	, m_scene_layers()
	, m_world_bounds(sf::Vector2f(0.f, 0.f), sf::Vector2f(1920, 1080))
	, m_spawn_position(m_camera.getSize().x / 2.f, m_world_bounds.size.y - m_camera.getSize().y/2.f)
	, m_player_tank(nullptr)
	, m_player2_tank(nullptr)
{
	LoadTextures();
	BuildScene();
	m_camera.setCenter(m_spawn_position);
}

void World::Update(sf::Time dt)
{

	if (m_player_tank) {
		m_player_tank->SetVelocity(0.f, 0.f);
	}

	if (m_player2_tank) m_player2_tank->SetVelocity(0.f, 0.f);

	DestroyEntitiesOutsideView();

	// 1. Process Input Commands
	while (!m_command_queue.IsEmpty())
	{
		m_scene_graph.OnCommand(m_command_queue.Pop(), dt);
	}
	
	HandleCollisions();

	m_scene_graph.RemoveWrecks();
	// 2. Update Scene Graph (Animations, Movement)
	m_scene_graph.Update(dt, m_command_queue);
}



void World::Draw()
{
	m_window.setView(m_camera);
	m_window.draw(m_scene_graph);
}


CommandQueue& World::GetCommandQueue()
{
	return m_command_queue;
}


void World::LoadTextures()
{
	//m_textures.Load(TextureID::kLandscape, "Media/Textures/Desert.png");
	m_textures.Load(TextureID::kLandscape, "Media/Textures/Background.png");
	m_textures.Load(TextureID::kTankBody, "Media/Textures/Hull1.png");
	m_textures.Load(TextureID::kTankTurret, "Media/Textures/Gun1.png");
	m_textures.Load(TextureID::kTankBody2, "Media/Textures/Hull2.png");
	m_textures.Load(TextureID::kTankTurret2, "Media/Textures/Gun2.png");
	m_textures.Load(TextureID::kBullet, "Media/Textures/Bullet.png");

	

}

void World::BuildScene()
{
	//Initialise the different layers
	for (int i = 0; i < static_cast<int>(SceneLayers::kLayerCount); i++)
	{
		ReceiverCategories category = (i == static_cast<int>(SceneLayers::kAir)) ? ReceiverCategories::kScene : ReceiverCategories::kNone;
		SceneNode::Ptr layer(new SceneNode(category));
		m_scene_layers[i] = layer.get();
		m_scene_graph.AttachChild(std::move(layer));
	}

	sf::Texture& texture = m_textures.Get(TextureID::kLandscape);
	sf::IntRect textureRect(m_world_bounds);
	//texture.setRepeated(true);

	std::unique_ptr<SpriteNode> background_sprite(new SpriteNode(texture, textureRect));
	background_sprite->setPosition(sf::Vector2f(0, 0));
	m_scene_layers[static_cast<int>(SceneLayers::kBackground)]->AttachChild(std::move(background_sprite));

	//adding tank player 1 
	std::unique_ptr<Tank> playerTank(new Tank(TankType::kTank1, m_textures, ReceiverCategories::kPlayer1Tank));
	m_player_tank = playerTank.get();
	m_player_tank->setScale(sf::Vector2f(0.5f, 0.5f));
	m_player_tank->setPosition(m_spawn_position);
	m_scene_layers[static_cast<int>(SceneLayers::kAir)]->AttachChild(std::move(playerTank));

	//addding player tank 2 
	std::unique_ptr<Tank> player2Tank(new Tank(TankType::kTank2, m_textures, ReceiverCategories::kPlayer2Tank));
	m_player2_tank = player2Tank.get();
	m_player2_tank->setScale(sf::Vector2f(0.5f, 0.5f));
	m_player2_tank->setPosition(m_spawn_position + sf::Vector2f(150.f, 0.f));
	m_scene_layers[static_cast<int>(SceneLayers::kAir)]->AttachChild(std::move(player2Tank));

}

void World::DestroyEntitiesOutsideView()
{
	Command command;
	command.category = static_cast<int>(ReceiverCategories::kPlayer1Projectile) | static_cast<int>(ReceiverCategories::kPlayer2Projectile);
	command.action = DerivedAction<Entity>([this](Entity& e, sf::Time dt)
		{
			//Does the object intersect with the battlefield
			if (GetBattleFieldBounds().findIntersection(e.GetBoundingRect()) == std::nullopt)
			{
				e.Destroy();
			}
		});
	m_command_queue.Push(command);

}

sf::FloatRect World::GetBattleFieldBounds() const
{
	//Return camera bounds + a small area off screen where the enemies spawn
	sf::FloatRect bounds = GetViewBounds();
	bounds.position.y -= 100.f;
	bounds.size.y += 100.f;
	return bounds;
}

sf::FloatRect World::GetViewBounds() const
{
	return sf::FloatRect(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());;
}

//Helper function to check if the scene nodes mathch the given categories
bool MatchesCategories(SceneNode::Pair& colliders, ReceiverCategories type1, ReceiverCategories type2)
{
	unsigned int category1 = colliders.first->GetCategory();
	unsigned int category2 = colliders.second->GetCategory();

	if ((static_cast<int>(type1) & category1) && (static_cast<int>(type2) & category2))
	{
		return true;
	}
	else if ((static_cast<int>(type1) & category2) && (static_cast<int>(type2) & category1))
	{
		std::swap(colliders.first, colliders.second);
		return true;
	}
	else
	{
		return false;
	}

}

void World::HandleCollisions() {
	std::set<SceneNode::Pair> collisionPairs;
	m_scene_graph.CheckSceneCollision(m_scene_graph, collisionPairs);

	for (SceneNode::Pair pair : collisionPairs)
	{
		// 1. player 1 hit by player 2's projectile
		if (MatchesCategories(pair, ReceiverCategories::kPlayer1Tank, ReceiverCategories::kPlayer2Projectile))
		{
			auto& tank = static_cast<Tank&>(*pair.first);
			auto& bullet = static_cast<Projectile&>(*pair.second);

			tank.Damage(bullet.GetDamage());
			bullet.Destroy();
			std::cout << "Player 1 hit by Player 2's projectile! Tank HP: " << tank.GetHitPoints() << "\n";
		}

		// 2. player 2 hit by player 1's projectile
		else if (MatchesCategories(pair, ReceiverCategories::kPlayer2Tank, ReceiverCategories::kPlayer1Projectile))
		{
			auto& tank = static_cast<Tank&>(*pair.first);
			auto& bullet = static_cast<Projectile&>(*pair.second);

			tank.Damage(bullet.GetDamage());
			bullet.Destroy();
			std::cout << "Player 2 hit by Player 1's projectile! Tank HP: " << tank.GetHitPoints() << "\n";
		}

		// 3. tank vs tank (body collision)
		else if (MatchesCategories(pair, ReceiverCategories::kPlayer1Tank, ReceiverCategories::kPlayer2Tank))
		{
			auto& p1 = static_cast<Tank&>(*pair.first);
			auto& p2 = static_cast<Tank&>(*pair.second);

			if (p1.CanBeDamaged())
			{
				p1.Damage(10);
				p1.ResetCollisionCooldown();
				std::cout << "Tank 1 damaged by Tank2" << "\n";
			}
			if (p2.CanBeDamaged())
			{
				p2.Damage(10);
				p2.ResetCollisionCooldown();
				std::cout << "Tank 2 damaged by Tank1" << "\n";
			}
		}
	}
}