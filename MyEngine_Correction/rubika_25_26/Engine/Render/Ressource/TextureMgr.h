#pragma once

#include <Engine/Debug/DebugMgr.h>

#include <SFML/Graphics/Texture.hpp>
#include <rapidxml/rapidxml.hpp>

#include <unordered_map>
#include <filesystem>
#include <string>
#include <atomic>
#include <future>
#include <queue>
#include <SFML/Graphics/Image.hpp>

struct TextureData;
using TextureLoadedCallback = std::function<void(const TextureData*)>;

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

struct PendingTexture
{
	std::filesystem::path Path;
	sf::Image Image;
	TextureData* Data;
	TextureLoadedCallback Callback;

	PendingTexture(): Image(), Data(nullptr) {}
	PendingTexture(const std::filesystem::path& path, const sf::Image& image, TextureData* data);
};

class TextureMgr final : public IDebugable
{
public:
	TextureMgr();
	~TextureMgr();

	void Init();
	void Shut();
	
	bool LoadTexture(const std::filesystem::path& path, sf::Image& outImage, TextureData*& outData);
	bool LoadTextureAsync(const std::filesystem::path& path, TextureLoadedCallback callback);


	bool LoadTexture(const std::filesystem::path& path, sf::Texture& texture);
	TextureData& GetTextureData(const std::string& name);

	static const sf::Texture& GetEmptyTexture();
	static const sf::Texture& GetMissingTexture();

	virtual void DrawDebug() override;

	std::queue<PendingTexture> PendingTextures;
	std::mutex PendingTexturesMutex;
	std::mutex TexturesMapMutex;
	
private:
	std::unordered_map<std::string, TextureData> Textures;

	bool LoadTextureMetadata(const std::filesystem::path& path, TextureData& textureData);
	bool LoadAnimationMetadata(rapidxml::xml_node<>* node, TextureData& textureData);
	bool LoadStaticTileMetadata(rapidxml::xml_node<>* node, TextureData& textureData);
};