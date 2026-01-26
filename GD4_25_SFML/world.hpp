#pragma once
#include <SFML/Graphics.hpp>
#include "resource_identifiers.hpp"
#include "scene_node.hpp"
#include "scene_layers.hpp"
#include "command_queue.hpp"
#include "tank.hpp"

class World
{
public:
	explicit World(sf::RenderWindow& window, FontHolder& font);
	void Update(sf::Time dt);
	void Draw();

	CommandQueue& GetCommandQueue();

private:
	void LoadTextures();
	void BuildScene();
	void DestroyEntitiesOutsideView();
	sf::FloatRect GetViewBounds() const;
	sf::FloatRect GetBattleFieldBounds() const;
	void HandleCollisions();

private:
	sf::RenderWindow& m_window;
	sf::View m_camera;
	TextureHolder m_textures;
	FontHolder& m_fonts;

	SceneNode m_scene_graph;
	std::array<SceneNode*, static_cast<int>(SceneLayers::kLayerCount)> m_scene_layers;
	CommandQueue m_command_queue;

	sf::FloatRect m_world_bounds;
	sf::Vector2f m_spawn_position;

	Tank* m_player_tank;
	Tank* m_player2_tank;
};

