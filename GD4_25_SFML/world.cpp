#include "world.hpp"
#include "sprite_node.hpp"
#include <iostream>
#include "state.hpp"
#include <SFML/System/Angle.hpp>
#include "tank.hpp"


World::World(sf::RenderWindow& window, FontHolder& font)
	: m_window(window)
	, m_camera(window.getDefaultView())
	, m_textures()
	, m_fonts(font)
	, m_scene_graph(ReceiverCategories::kNone)
	, m_scene_layers()
	, m_world_bounds(sf::Vector2f(0.f, 0.f), sf::Vector2f(m_camera.getSize().x, 3000.f))
	, m_spawn_position(m_camera.getSize().x / 2.f, m_world_bounds.size.y - m_camera.getSize().y/2.f)
{
	LoadTextures();
	BuildScene();
	m_camera.setCenter(m_spawn_position);
}

void World::Update(sf::Time dt)
{
	// 1. Process Input Commands
	while (!m_command_queue.IsEmpty())
	{
		m_scene_graph.OnCommand(m_command_queue.Pop(), dt);
	}

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
	m_textures.Load(TextureID::kLandscape, "Media/Textures/Desert.png");
	m_textures.Load(TextureID::kTankBody, "Media/Textures/Hull1.png");
	m_textures.Load(TextureID::kTankTurret, "Media/Textures/Gun1.png");
	

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
	texture.setRepeated(true);

	std::unique_ptr<SpriteNode> background_sprite(new SpriteNode(texture, textureRect));
	background_sprite->setPosition(sf::Vector2f(0, 0));
	m_scene_layers[static_cast<int>(SceneLayers::kBackground)]->AttachChild(std::move(background_sprite));

	//adding tank player 1 
	std::unique_ptr<Tank> playerTank(new Tank(Tank::kDefault, m_textures));
	playerTank->setPosition(m_spawn_position);

	m_scene_layers[static_cast<int>(SceneLayers::kAir)]->AttachChild(std::move(playerTank));

}

