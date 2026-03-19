#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <Engine/Render/Ressource/TextureMgr.h>
#include <Engine/Console/LogConsole.h>
#include <Engine/Globals.h>
#include <Engine/Gameplay/GameMgr.h>

#include <Engine/Gameplay/Entity/Entity.h>
#include <Engine/Gameplay/Component/Transform/Transform.h>
#include <Engine/Gameplay/Component/Renderer/Renderer.h>
#include <Engine/Render/Drawable/Sprite/Sprite.h>

#ifdef _USE_IMGUI
#include "Imgui/imgui.h"
#include "Imgui/imgui-SFML.h"
#endif

#include <iostream>
#include <Engine/Profiler.h>

#include "Engine/Random/RandomMgr.h"

Entity* CreateEntity()
{
    Entity* e = new Entity();

    Transform* TransformComp = e->AddComponent<Transform>();
    Renderer* RendererComp = e->AddComponent<Renderer>();

    Sprite* Body = RendererComp->AddNewDrawable<Sprite>("Body", sf::Vector2f(2, 0), 0, sf::Vector2f(1, 1));
    Body->SetVisibility(true);
    Body->SetTexture("../Ressources/IsaacSprite.png");
    Body->SetAnimation("Body_Vertical");

    Sprite* Head = RendererComp->AddNewDrawable<Sprite>("Head", sf::Vector2f(-2.5f, -20), 0, sf::Vector2f(1, 1));
    Head->SetVisibility(true);
    Head->SetTexture("../Ressources/IsaacSprite.png");
    Head->SetAnimation("Head_Down");

    TransformComp->SetWorldPosition(sf::Vector2f(150.f, 150.f));

    return e;
}

void PopulateUpdate()
{
    for (int i = 0; i < 10; ++i)
    {
        gData.TaskMgr->RegisterTask([i]()
            {
                PROFILER_EVENT_BEGIN(PROFILER_COLOR_DARK_BLUE, "Update %d", i);

                Sleep(5);

                PROFILER_EVENT_END();
            },
            TaskMgr::ePhase::Update);
    }
}

void PopulateDraw()
{
    for (int i = 0; i < 20; ++i)
    {
        gData.TaskMgr->RegisterTask([i]()
            {
                PROFILER_EVENT_BEGIN(PROFILER_COLOR_DARK_BLUE, "Draw %d", i);

                Sleep(5);

                PROFILER_EVENT_END();
            },
            TaskMgr::ePhase::Draw);
    }
}

int main()
{
    gData.Init();
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "MyEngine - Rubika 2025 2026");
    
#ifdef _USE_IMGUI
    if (!ImGui::SFML::Init(window))
    {
        return -1;
    }
#endif

 
    gData.TextureMgr->LoadTextureAsync("../Ressources/IsaacSprite.png", [](const TextureData* t)
    {
        if (t)
        {
            Entity* entity = CreateEntity();
            gData.GameMgr->AddEntity(entity);
        }
        else
            std::cout << "Failure" << std::endl;
    });
    gData.TextureMgr->LoadTextureAsync("../Ressources/Basement.png",  [](const TextureData* t)
    {
        if (t)
            std::cout << "Success" << std::endl;
        else
            std::cout << "Failure" << std::endl;
    });
    gData.TextureMgr->LoadTextureAsync("../Ressources/Tear.png",  [](const TextureData* t)
    {
        if (t)
            std::cout << "Success" << std::endl;
        else
            std::cout << "Failure" << std::endl;
    });
    gData.TextureMgr->LoadTextureAsync("../Ressources/Rocks.png",  [](const TextureData* t)
    {
        if (t)
            std::cout << "Success" << std::endl;
        else
            std::cout << "Failure" << std::endl;
    });
    gData.TextureMgr->LoadTextureAsync("../Ressources/Doors.png",  [](const TextureData* t)
    {
        if (t)
            std::cout << "Success" << std::endl;
        else
            std::cout << "Failure" << std::endl;
    });
    
    
    sf::Clock clock;
    clock.restart();
    
    while (window.isOpen() && !gData.ExipApp)
    {
        if (!gData.TextureMgr->PendingTextures.empty())
        {
            PendingTexture texture;
            {
                std::unique_lock<std::mutex> pendingLock(gData.TextureMgr->PendingTexturesMutex);
                texture = gData.TextureMgr->PendingTextures.front();
                gData.TextureMgr->PendingTextures.pop();
            }
            
            TextureData* t = texture.Data;
            bool loaded = t && t->Texture.loadFromImage(texture.Image);
            gData.TaskMgr->RegisterTask(
                [callback = texture.Callback, t, loaded]() {
                    if (callback)
                        callback(loaded ? t : nullptr);
                },
                TaskMgr::ePhase::Worker
            );
        }

        PROFILER_EVENT_BEGIN(PROFILER_COLOR_BLACK, "Frame %llu", gData.FrameCount);
        {
            int deltaTimeMS = clock.getElapsedTime().asMilliseconds();
            float fDeltaTimeS = (float)deltaTimeMS / 1000.f;
            sf::Time imGuiTime = clock.restart();
            
            PROFILER_EVENT_BEGIN(PROFILER_COLOR_BLUE, "Event & Input");
            {
                while (const std::optional event = window.pollEvent())
                {

                    if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
                    {
                        window.close();
                        gData.ExipApp = true;
                    }

#ifdef _USE_IMGUI
                    ImGui::SFML::ProcessEvent(window, *event);
#endif
                }
            }
            PROFILER_EVENT_END();

            PopulateDraw();
            PopulateUpdate();
            PROFILER_EVENT_BEGIN(PROFILER_COLOR_RED, "Update");
            {
     
                gData.TaskMgr->StartPhase(TaskMgr::ePhase::Update);
#ifdef _USE_IMGUI
                ImGui::SFML::Update(window, imGuiTime);
#endif
                gData.GameMgr->Update(fDeltaTimeS);
                
                
                gData.TaskMgr->WaitPhase();
            }
            PROFILER_EVENT_END();
            
            PROFILER_EVENT_BEGIN(PROFILER_COLOR_GREEN, "Draw");
            {
     
                gData.TaskMgr->StartPhase(TaskMgr::ePhase::Draw);
                PROFILER_EVENT_BEGIN(PROFILER_COLOR_BROWN, "Debug Draw");
                gData.DebugMgr->Draw();
                PROFILER_EVENT_END();

                window.clear();

                PROFILER_EVENT_BEGIN(PROFILER_COLOR_PURPLE, "Game Draw");
                gData.GameMgr->Draw(window);
                PROFILER_EVENT_END();

#ifdef _USE_IMGUI
                PROFILER_EVENT_BEGIN(PROFILER_COLOR_ORANGE, "ImGui Draw");
                ImGui::SFML::Render(window);
                PROFILER_EVENT_END();
#endif
                PROFILER_EVENT_BEGIN(PROFILER_COLOR_CYAN, "Window Display");
                window.display();
                PROFILER_EVENT_END();
                
                gData.TaskMgr->WaitPhase();
                
            }

            
            PROFILER_EVENT_END();
        }
        PROFILER_EVENT_END();
        ++gData.FrameCount;
        gData.TaskMgr->StartPhase(TaskMgr::ePhase::None);
    }

    gData.ExipApp = true;

    gData.Shut();
    gData.Destroy();
#ifdef _USE_IMGUI
    ImGui::SFML::Shutdown();
#endif

    return 0;
}