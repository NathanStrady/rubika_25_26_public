#include "TextureMgr.h"

#include <Engine/Globals.h>
#include <rapidxml/rapidxml_utils.hpp>

#ifdef _USE_IMGUI
#include <Imgui/imgui.h>
#include <Imgui/imgui-SFML.h>
#endif

#include <assert.h>
#include <filesystem>
#include <iostream>
#include <SFML/Graphics/Image.hpp>
#include <utility>

#include "Engine/Profiler.h"
#include "Engine/Debug/DebugMgr.h"

PendingTexture::PendingTexture(const std::filesystem::path& path, const sf::Image& image, TextureData* data): Path(path), Image(image), Data(data)
{}

TextureMgr::TextureMgr()
{}

TextureMgr::~TextureMgr()
{
	for (const auto& p : Textures)
	{
		p.second.Release();
		assert(p.second.RefCount == 0);
	}

	Textures.clear();
}

void TextureMgr::Init()
{
	gData.DebugMgr->RegisterDebugableWindow("TextureMgr", this);
}

void TextureMgr::Shut()
{
	gData.DebugMgr->UnregisterDebugableWindow("TextureMgr");
}

bool TextureMgr::LoadTexture(const std::filesystem::path& path, sf::Image& outImage, TextureData*& outData)
{
	sf::Image image;
	if (!std::filesystem::exists(path.native()))
	{
		std::cerr << "Texture file doesn't exist " << path << std::endl;
		return false;
	}

	std::filesystem::path metadataPath = path;
	metadataPath = metadataPath.replace_extension(".xml");
	if (!std::filesystem::exists(metadataPath.native()))
	{
		std::cerr << "Texture metadata file doesn't exist " << metadataPath << std::endl;
		return false;
	}
	
	std::pair<std::unordered_map<const std::string, TextureData>::iterator, bool> p;
	{
		std::unique_lock<std::mutex> textureLock(TexturesMapMutex);
		p = Textures.emplace(std::piecewise_construct, 
					std::forward_as_tuple(path.string()),
					std::forward_as_tuple());
	}
	TextureData& textureData = p.first->second;
	textureData.AddRef();

	if (!image.loadFromFile(path.string()))
	{
		return false;
	}
	
	if (!LoadTextureMetadata(metadataPath, textureData))
	{
		return false;
	}

	outImage = image;
	outData = &textureData;
	return true;
}

bool TextureMgr::LoadTextureAsync(const std::filesystem::path& path, TextureLoadedCallback callback)
{
	gData.TaskMgr->RegisterTask([path, this, callback]()
	{

		PROFILER_EVENT_BEGIN(PROFILER_COLOR_DARK_BLUE, "Loading Texture : %s", path.string().c_str());
		Sleep(30000);
		sf::Image image;
		TextureData* data = nullptr;
		bool success = LoadTexture(path, image, data);
		
		PendingTexture p;
		
		if (success)
		{
			p.Path = path;
			p.Image = image;
			p.Data = data;
		}
		else
		{
			p.Path = "";
			p.Image = sf::Image();
			p.Data = nullptr;
		}
		
		p.Callback = callback;
		{
			std::unique_lock<std::mutex> queueLock(PendingTexturesMutex);
			PendingTextures.push(p);
		}
		PROFILER_EVENT_END();
	}, TaskMgr::ePhase::Worker);
	return true;
}

bool TextureMgr::LoadTexture(const std::filesystem::path& path, sf::Texture& texture)
{
	auto p = Textures.emplace(std::piecewise_construct,
				 std::forward_as_tuple(path.string()),  
				 std::forward_as_tuple());
	
	if (!p.second)
	{
		std::cerr << "LoadTexture: Internal error. Cannot emplace in map" << '\n';
		return false;
	}
	
	p.first->second.Texture = texture;
	p.first->second.AddRef();

	return true;
}

TextureData& TextureMgr::GetTextureData(const std::string& name)
{
	std::unique_lock<std::mutex> TexturesLock(TexturesMapMutex);
	assert(Textures.find(name) != Textures.end());
	return Textures.at(name);
}

sf::Texture emptyTexture;

const sf::Texture& TextureMgr::GetEmptyTexture()
{
	return emptyTexture;
}

const sf::Texture& TextureMgr::GetMissingTexture()
{
	return emptyTexture;
}

bool TextureMgr::LoadTextureMetadata(const std::filesystem::path& path, TextureData& textureData)
{
	rapidxml::file<> metadataFile(path.string().c_str());
	if (metadataFile.size() == 0)
	{
		std::cerr << "LoadTextureMetadata: Cannot open file " << path << std::endl;
		return false;
	}

	rapidxml::xml_document metadataXml;
	metadataXml.parse<0>(metadataFile.data());

	if (rapidxml::xml_node<>* node = metadataXml.first_node("Animations"))
	{
		if (!LoadAnimationMetadata(node, textureData))
		{
			return false;
		}
	}

	if (rapidxml::xml_node<>* node = metadataXml.first_node("Backgrounds"))
	{
		if (!LoadStaticTileMetadata(node, textureData))
		{
			return false;
		}
	}

	return true;
}

bool TextureMgr::LoadAnimationMetadata(rapidxml::xml_node<>* node, TextureData& textureData)
{
	if (!node)
	{
		return false;
	}

	rapidxml::xml_node<>* animationNode = node->first_node();
	while (animationNode)
	{
		rapidxml::xml_attribute<>* nameAttribute = animationNode->first_attribute("Name");
		if (nameAttribute)
		{
			auto p = textureData.AnimationsData.emplace(std::string(nameAttribute->value(), nameAttribute->value_size()), AnimationData());
			if (p.second)
			{
				AnimationData& data = p.first->second;
				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("X"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.StartX = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("Y"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.StartY = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("SizeX"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.SizeX = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("SizeY"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.SizeY = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("OffsetX"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.OffsetX = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("OffsetY"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.OffsetY = atoi(value.c_str());
				}
				
				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("SpriteNum"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.AnimationSpriteCount = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("SpritesOnLine"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.SpriteOnLine = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("Reverted"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.IsReverted = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("TimeBetweenAnimation"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.TimeBetweenAnimationInS = std::stof(value.c_str());
				}
			}
			else
			{
				std::cerr << "LoadAnimationMetadata: Cannot add animation " << nameAttribute->value() << ". Ignore it" << std::endl;
			}
		}
		else
		{
			std::cerr << "LoadAnimationMetadata: Find a animation node with no name. Ignore it" << std::endl;
		}

		animationNode = animationNode->next_sibling();
	}

	return true;
}

bool TextureMgr::LoadStaticTileMetadata(rapidxml::xml_node<>* node, TextureData& textureData)
{
	if (!node)
	{
		return false;
	}

	rapidxml::xml_node<>* tileNode = node->first_node();
	while (tileNode)
	{
		rapidxml::xml_attribute<>* nameAttribute = tileNode->first_attribute("Name");
		if (nameAttribute)
		{
			auto p = textureData.StaticTilesData.emplace(std::string(nameAttribute->value(), nameAttribute->value_size()), StaticTileData());
			if (p.second)
			{
				StaticTileData& data = p.first->second;
				if (rapidxml::xml_node<>* innerNode = tileNode->first_node("X"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.StartX = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = tileNode->first_node("Y"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.StartY = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = tileNode->first_node("SizeX"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.SizeX = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = tileNode->first_node("SizeY"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.SizeY = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = tileNode->first_node("RevertedX"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.IsRevertedX = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = tileNode->first_node("RevertedY"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.IsRevertedY = atoi(value.c_str());
				}
			}
			else
			{
				std::cerr << "LoadStaticTileMetadata: Cannot add static Data " << nameAttribute->value() << ". Ignore it" << std::endl;
			}
		}
		else
		{
			std::cerr << "LoadStaticTileMetadata: Find a tile node with no name. Ignore it" << std::endl;
		}

		tileNode = tileNode->next_sibling();
	}

	return true;
}

void TextureMgr::DrawDebug()
{
#ifdef _USE_IMGUI

	const auto flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders;

	if (ImGui::BeginTable("##Textures", 5, flags))
	{
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Width");
		ImGui::TableSetupColumn("Height");
		ImGui::TableSetupColumn("RefCount");
		ImGui::TableSetupColumn("Image");
		ImGui::TableHeadersRow();

		for (const auto& [name, data] : Textures)
		{
			int count = data.RefCount;

			ImVec4 color = count > 1 ? ImVec4(255, 255, 255, 255) : ImVec4(200, 0, 0, 255);
			ImGui::PushStyleColor(ImGuiCol_Text, color);

			ImGui::TableNextColumn();
			ImGui::TextWrapped(name.c_str());
			ImGui::TableNextColumn();
			ImGui::TextWrapped("%d", data.Texture.getSize().x);
			ImGui::TableNextColumn();
			ImGui::TextWrapped("%d", data.Texture.getSize().y);
			ImGui::TableNextColumn();
			ImGui::TextWrapped("%d", count);
			ImGui::TableNextColumn();

			const sf::Vector2f size((float)data.Texture.getSize().x, (float) data.Texture.getSize().y);
			const float displayMaxSizeX = 200;
			const float displayMaxSizeY = 200;

			sf::Vector2f displaySize;
			if (size.x < displayMaxSizeX && size.y < displayMaxSizeY)
			{
				displaySize = size;
			}
			else if (size.x > size.y)
			{
				displaySize.x = displayMaxSizeX;
				displaySize.y = size.y / size.x * displayMaxSizeY;
			}
			else
			{
				displaySize.x = size.x / size.y * displayMaxSizeX;
				displaySize.y = displayMaxSizeY;
			}

			ImGui::Image(data.Texture, displaySize);

			ImGui::PopStyleColor();

			ImGui::TableNextRow();
		}
		ImGui::EndTable();
	}

#endif
}

AnimationData::AnimationData(): StartX(0), StartY(0), SizeX(0), SizeY(0),
OffsetX(0), OffsetY(0), AnimationSpriteCount(0), SpriteOnLine(0), IsReverted(false)
{}

TextureData::TextureData(): Texture(), AnimationsData(), RefCount(0)
{}

TextureData::~TextureData()
{
	AnimationsData.clear();
}

void TextureData::AddRef() const
{
	RefCount++;
}

void TextureData::Release() const
{
	assert(RefCount > 0);
	RefCount--;
}
