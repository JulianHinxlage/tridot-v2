//
// Copyright (c) 2021 Julian Hinxlage. All rights reserved.
//

#ifndef TRIDOT_ENGINE_H
#define TRIDOT_ENGINE_H

#include "tridot/ecs/Registry.h"
#include "Time.h"
#include "Input.h"
#include "Physics.h"
#include "ResourceLoader.h"
#include "tridot/render/MeshRenderer.h"
#include "tridot/render/PBRenderer.h"
#include "tridot/render/Window.h"
#include "tridot/core/Log.h"
#include "tridot/core/config.h"

namespace tridot {

    class Engine : public ecs::Registry {
    public:
        Time time;
        Input input;
        Physics physics;
        ResourceLoader resources;
        MeshRenderer renderer;
        PBRenderer pbRenderer;
        Window window;

        Engine();
        void init(uint32_t width, uint32_t height, const std::string &title, const std::string &resourceDirectory, bool autoReload = false);
        void update();
        void run();

        auto onUpdate(){
            return onUpdateSignal.ref();
        }

        auto onInit(){
            return onInitSignal.ref();
        }

    private:
        ecs::Signal<> onUpdateSignal;
        ecs::Signal<> onInitSignal;
    };

}

extern tridot::Engine engine;

#define TRI_UNIQUE_NAME_3(name, line) name##line
#define TRI_UNIQUE_NAME_2(name, line) TRI_UNIQUE_NAME_3(name, line)
#define TRI_UNIQUE_NAME(name) TRI_UNIQUE_NAME_2(name, __COUNTER__)
bool addUpdateSignalCallback(const std::string &name, const std::function<void()> &callback);
bool addInitSignalCallback(const std::string &name, const std::function<void()> &callback);
#define TRI_UPDATE_2(name, func) static void func();\
namespace{ bool TRI_UNIQUE_NAME(___tri_global___) = addUpdateSignalCallback(name, &func);}\
static void func()
#define TRI_UPDATE(name) TRI_UPDATE_2(name, TRI_UNIQUE_NAME(___tri_update_func___))
#define TRI_INIT_2(name, func) static void func();\
namespace{ bool TRI_UNIQUE_NAME(___tri_global___) = addInitSignalCallback(name, &func);}\
static void func()
#define TRI_INIT(name) TRI_INIT_2(name, TRI_UNIQUE_NAME(___tri_init_func___))

#endif //TRIDOT_ENGINE_H
