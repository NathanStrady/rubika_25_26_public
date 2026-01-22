#pragma once

#include <Engine/Debug/DebugMgr.h>

#include <SFML/Graphics/Texture.hpp>
#include <rapidxml/rapidxml.hpp>

#include <unordered_map>
#include <filesystem>
#include <string>
#include <atomic>

struct AnimationData
{
	AnimationData();

	int StartX;
	int StartY;
	int SizeX;
	int SizeY;
	int OffsetX;
	int OffsetY;
	int AnimationSpriteCount;
	int SpriteOnLine;
	bool IsReverted;
	float TimeBetweenAnimationInS;
};

struct StaticTileData
{
	int StartX;
	int StartY;
	int SizeX;
	int SizeY;
	bool IsRevertedX;
	bool IsRevertedY;
};

struct TextureData
{
	TextureData();
	~TextureData();

	sf::Texture Texture;
	std::unordered_map<std::string, AnimationData> AnimationsData;
	std::unordered_map<std::string, StaticTileData> StaticTilesData;

	void AddRef() const;
	void Release() const;

	friend class TextureMgr;
private:
	mutable std::atomic<int> RefCount;
};

class TextureMgr final : public IDebugable
{
public:

	TextureMgr();
	~TextureMgr();

	void Init();
	void Shut();

	bool LoadTexture(const std::filesystem::path& path);

	bool LoadTexture(const std::filesystem::path& path, sf::Texture& texture);

	const TextureData& GetTextureData(const std::string& name) const;

	static const sf::Texture& GetEmptyTexture();
	static const sf::Texture& GetMissingTexture();

	virtual void DrawDebug() override;

private:
	std::unordered_map<std::string, TextureData> Textures;

	bool LoadTextureMetadata(const std::filesystem::path& path, TextureData& textureData);
	bool LoadAnimationMetadata(rapidxml::xml_node<>* node, TextureData& textureData);
	bool LoadStaticTileMetadata(rapidxml::xml_node<>* node, TextureData& textureData);
};