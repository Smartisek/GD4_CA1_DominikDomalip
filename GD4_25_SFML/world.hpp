#pragma once
#include <SFML/Graphics.hpp>
#include "resource_identifiers.hpp"
#include "scene_node.hpp"
#include "scene_layers.hpp"
#include "command_queue.hpp"
#include "tank.hpp"
#include "sound_player.hpp"
#include "bloom_effect.hpp"

class World
{
public:
	explicit World(sf::RenderTarget& output_target, FontHolder& font, SoundPlayer& sounds);
	void Update(sf::Time dt);
	void Draw();

	CommandQueue& GetCommandQueue();

private:
	void LoadTextures();
	void BuildScene();
	void DestroyEntitiesOutsideView();
	sf::FloatRect GetViewBounds() const;
	sf::FloatRect GetBattleFieldBounds() const;
	void UpdateSounds();
	void HandleCollisions();
	void HandleTankCollision(Tank& tank1, Tank& tank2);
	void ApplyFriction(sf::Time dt);

private:
	sf::RenderTarget& m_target;
	sf::RenderTexture m_scene_texture;
	//sf::RenderWindow& m_window;
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

	SoundPlayer& m_sounds;

	BloomEffect m_bloom_effect;
};

