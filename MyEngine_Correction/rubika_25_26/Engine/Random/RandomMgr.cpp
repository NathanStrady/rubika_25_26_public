#include "RandomMgr.h"

#include <iostream>
#include <ImGui/imgui.h>
#include <ImGui/imgui-SFML.h>
#include <SFML/Graphics/Sprite.hpp>

#include "Algorithm/CellularAutomata/CellularAutomata.h"
#include "Engine/Globals.h"
#include "Engine/Render/Ressource/TextureMgr.h"
#include "Instances/RandomInstance.h"

void RandomMgr::Init()
{
    gData.DebugMgr->RegisterDebugableWindow("RandomMgr", this);
    CreateInstance();
}

void RandomMgr::Shut()
{
    gData.DebugMgr->UnregisterDebugableWindow("RandomMgr");
    instances.clear();
}

RandomMgr::InstanceId RandomMgr::CreateInstance()
{
    if (instances.size() > MAX_RANDOM_INSTANCES)
    {
        return 0;
    }
    currentId++;
    RandomInstance* newInstance = new RandomInstance(GenerateRandomSeed());
    instances.insert(std::pair<InstanceId, RandomInstance*>(currentId, newInstance));
    return currentId;
}

RandomMgr::InstanceId RandomMgr::CreateInstance(InstanceId instanceId)
{
    if (instances.size() > MAX_RANDOM_INSTANCES)
    {
        return 0;
    }
    
    RandomInstance* newInstance = new RandomInstance(instanceId);
    instances.insert(std::pair<InstanceId, RandomInstance*>(instanceId, newInstance));
    return instanceId;
}

void RandomMgr::DestroyInstance(InstanceId instanceId)
{
    delete instances[instanceId];
    instances.erase(instanceId);
}

bool RandomMgr::GetInstance(InstanceId instanceId, RandomInstance*& randomInstance) const
{
    if (!instances.contains(instanceId))
    {
        randomInstance = nullptr;
        return false; 
    }

    randomInstance = instances.at(instanceId);
    return true; 
}



void RandomMgr::DrawDebug()
{
#ifdef _USE_IMGUI
    const auto flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders;
    const auto tabFlags = ImGuiTabBarFlags_None;
    const auto childFlags = ImGuiChildFlags_Border;


    if (ImGui::BeginTabBar("Random Managers", tabFlags))
    {
        if (ImGui::BeginTabItem("Random Instances"))
        {
            if (d_init)
            {
                for (auto& it : instances)
                {
                    debug.insert(std::pair<InstanceId, DebugValues>(it.first, DebugValues()));
                }
            }
        
            if (ImGui::SmallButton("Refresh"))
            {
                for (auto& it : instances)
                {
                    debug[it.first] = DebugValues{
                        it.second->RandUInt(1, 10),
                        it.second->RandInt32(1, 10),
                        it.second->RandDouble(1, 10),
                        it.second->RandNormalDouble(0, 100),
                    };
                }
            }

            if (ImGui::SmallButton("Create new Instance"))
            {
                int newId;
                newId = CreateInstance();
                if (newId != 0)
                {
                    debug.insert(std::pair<InstanceId, DebugValues>(newId, DebugValues()));
                } else
                {
                    std::cout << "Can't create new Instance" << std::endl;
                }
            }
        
            if (ImGui::BeginTable("##Random", 5, flags))
            {
                ImGui::TableSetupColumn("Instance Id");
                ImGui::TableSetupColumn("RandUInt");
                ImGui::TableSetupColumn("RandInt32");
                ImGui::TableSetupColumn("RandDouble");
                ImGui::TableSetupColumn("RandNormalDouble");
                ImGui::TableHeadersRow();
            }
        
            for (auto& [id, instance] : debug)
            {
                ImGui::TableNextColumn();
                ImGui::TextWrapped("%d", id);

                ImGui::TableNextColumn();
                ImGui::TextWrapped("%u", instance.u_linear);

                ImGui::TableNextColumn();
                ImGui::TextWrapped("%d", instance.i_linear);

                ImGui::TableNextColumn();
                ImGui::TextWrapped("%.2f", instance.d_linear);

                ImGui::TableNextColumn();
                ImGui::TextWrapped("%.2f", instance.d_normal);
            }
            ImGui::EndTable();
        
            d_init = false;
            
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Drunkard Walk Generation"))
        {
            static int randomId = 0;
            static int sizeX = 256; 
            static int sizeY = 256;
            static int iteration = 5;
            static int distance = 25;
            static int spawnNumber = 1;
            static sf::Texture tex = gData.TextureMgr->GetEmptyTexture();
        

            ImGui::PushItemWidth(150);
            ImGui::SliderInt("Instance", &randomId, 0, 10);
            ImGui::SliderInt("Size X", &sizeX, 1, 256);
            ImGui::SliderInt("Size Y", &sizeY, 1, 256);
            ImGui::SliderInt("Iteration", &iteration, 1, 1000); 
            ImGui::SliderInt("Distance", &distance, 1, 1000);
            ImGui::SliderInt("Spawn Number", &spawnNumber, 1, 100);
            ImGui::PopItemWidth();
    
            if (ImGui::Button("Generate", ImVec2(150, 20)))
            {
                drunkardWalk = std::make_unique<DrunkardWalk>(sizeX, sizeY, iteration, distance, spawnNumber);
                if (drunkardWalk->Generate(randomId))
                {
                    drunkardWalk->GenerateTexture(tex);
                }
            }

            ImGui::Image(tex, sf::Vector2f(static_cast<float>(256), static_cast<float>(256)));
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Cellular Automata Generation"))
        {
            static int randomId = 0;
            static int sizeX = 256; 
            static int sizeY = 256;
            static float SpawnPercent = 0.6f;
            static int NeightborThreshold = 5;
            static int Iteration = 5;
            static sf::Texture tex = gData.TextureMgr->GetEmptyTexture();
    

            ImGui::PushItemWidth(150);
            ImGui::SliderInt("Instance", &randomId, 0, 10);
            ImGui::SliderInt("Size X", &sizeX, 1, 256);
            ImGui::SliderInt("Size Y", &sizeY, 1, 256);
            ImGui::SliderFloat("SpawnPercent", &SpawnPercent, 0., 1.); 
            ImGui::SliderInt("NeightborThreshold", &NeightborThreshold, 1, 8);
            ImGui::SliderInt("Iteration", &Iteration, 0, 20);
            ImGui::PopItemWidth();
            
            if (ImGui::Button("Generate", ImVec2(150, 20)))
            {
                cellularAutomata = std::make_unique<CellularAutomata>(sizeX, sizeY, SpawnPercent, NeightborThreshold, Iteration);
                if (cellularAutomata->Generate(randomId))
                {
                    cellularAutomata->GenerateTexture(tex);
                }
            }
            
            ImGui::Image(tex, sf::Vector2f(static_cast<float>(512), static_cast<float>(512)));
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Perlin Noise"))
        {
            static int randomId = 0;
            static int sizeX = 256;
            static int sizeY = 256;
            static int CellSize  = 16;
            static int Octaves  = 5;
            static sf::Texture tex = gData.TextureMgr->GetEmptyTexture();

            ImGui::PushItemWidth(150);
            ImGui::SliderInt("Instance", &randomId, 0, 10);
            ImGui::SliderInt("Size X", &sizeX, 1, 256);
            ImGui::SliderInt("Size Y", &sizeY, 1, 256);
            ImGui::SliderInt("Cell Size", &CellSize, 0, 32); 
            ImGui::SliderInt("Octaves", &Octaves, 1, 8);
            ImGui::PopItemWidth();

            if (ImGui::Button("Generate", ImVec2(150, 20)))
            {
                perlinNoise = std::make_unique<Perlin>(sizeX, sizeY, CellSize, Octaves);
                if (perlinNoise->Generate(randomId))
                {
                    perlinNoise->GenerateTexture(tex);
                }
            }

            ImGui::Image(tex, sf::Vector2f(static_cast<float>(512), static_cast<float>(512)));
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
#endif
}


uint32_t RandomMgr::GenerateRandomSeed()
{
    std::random_device rd;
    return rd(); 
}
