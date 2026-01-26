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

			HandleTankCollision(p1, p2);
		}
	}
}

void World::HandleTankCollision(Tank& tank1, Tank& tank2)
{
	sf::Vector2f tank1Pos = tank1.GetWorldPosition();
	sf::Vector2f tank2Pos = tank2.GetWorldPosition();
	//this vector gives us arrow pointing from player2 to player1 
	sf::Vector2f diff = tank1Pos - tank2Pos;

	float distSq = diff.x * diff.x + diff.y * diff.y; //pythagoras distance squared 
	float dist = std::sqrt(distSq); //the acutal distance 

	if (dist > 0.1f) //preventing division by zero
	{
		sf::Vector2f normal = diff / dist; // normalize by making length 1 making it direciton

		//fixing bug with the collision into each other bugging with commands
		float pushForce = 2.0f;
		tank1.move(normal * pushForce); //move tank1 away along the normal so from tank2
		tank2.move(-normal * pushForce); //move tank2 oposite way 

		sf::Vector2f v1 = tank1.GetVelocity();
		sf::Vector2f v2 = tank2.GetVelocity();

		//-normal for tank1 because normal points away from tank2, we want speed towards tank2
		float tank1SpeedTowards = v1.x * -normal.x + v1.y * -normal.y;
		//normal because normal points towards tank 1 
		float tank2SpeedTowards = v2.x * normal.x + v2.y * normal.y;

		float damageThreshold = 10.0f;

		//logic for who is ramming who to be able to apply damage to the "rammed one"
		if (tank1SpeedTowards > tank2SpeedTowards + damageThreshold)
		{
			if (tank2.CanBeDamaged())
			{
				tank2.Damage(10);
				tank2.ResetCollisionCooldown();
				tank2.move(-normal *( pushForce + 50.0f));
			}
		}
		else if (tank2SpeedTowards > tank1SpeedTowards + damageThreshold)
		{
			if (tank1.CanBeDamaged())
			{
				tank1.Damage(10);
				tank1.ResetCollisionCooldown();
				tank1.move(normal * (pushForce + 50.0f));
			}
		}
		else if (tank1SpeedTowards > 0 && tank2SpeedTowards > 0)
		{
			// head-on collision
			if (tank1.CanBeDamaged()) { tank1.Damage(10); tank1.ResetCollisionCooldown(); tank1.move(normal * (pushForce + 50.0f));
			}
			if (tank2.CanBeDamaged()) { tank2.Damage(10); tank2.ResetCollisionCooldown(); tank2.move(-normal * (pushForce + 50.0f));
			}
		}

		//stopping from fighting the push
		float dot1 = v1.x * normal.x + v1.y * normal.y;
		float dot2 = v2.x * normal.x + v2.y * normal.y;

		if (dot1 < 0.f) tank1.SetVelocity(v1 - normal * dot1);
		if (dot2 > 0.f) tank2.SetVelocity(v2 - normal * dot2);
	}
}