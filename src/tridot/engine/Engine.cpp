//
// Copyright (c) 2021 Julian Hinxlage. All rights reserved.
//

#include "Engine.h"
#include "tridot/components/Tag.h"
#include "tridot/components/RenderComponent.h"
#include "tridot/components/ComponentCache.h"
#include "tridot/render/Camera.h"
#include "tridot/engine/Serializer.h"
#include "tridot/render/MeshFactory.h"
#include "tridot/render/RenderContext.h"
#include <GL/glew.h>

tridot::Engine engine;

void imguiInit();

bool &isEngineConstructed(){
    static bool value = false;
    return value;
}

bool &isEngineInitialized(){
    static bool value = false;
    return value;
}

bool &isEngineDestructed(){
    static bool value = false;
    return value;
}

tridot::Signal<> &tmpUpdateSignal(){
    static tridot::Signal<> signal;
    return signal;
}

tridot::Signal<> &tmpInitSignal(){
    static tridot::Signal<> signal;
    return signal;
}

namespace tridot {

    Engine::Engine() {
        isEngineConstructed() = true;
        onInitSignal = tmpInitSignal();
        onUpdateSignal = tmpUpdateSignal();
    }

    Engine::~Engine() {
        isEngineDestructed() = true;
    }

    void Engine::init(uint32_t width, uint32_t height, const std::string &title, const std::string &resourceDirectory, bool autoReload) {
        Log::info("Tridot version " TRI_VERSION);

        registerComponent<Tag, uuid, Transform, RenderComponent, PerspectiveCamera, OrthographicCamera, Light, RigidBody, Collider, ComponentCache>();

        window.init(width, height, title);
        input.init();
        time.init();
        physics.init();
        renderer.init(resources.get<Shader>("shaders/mesh.glsl"));
        pbRenderer.init(resources.get<Shader>("shaders/PBR.glsl"));
        RenderContext::setDepth(true);

        resources.addSearchDirectory(resourceDirectory);
        resources.autoReload = autoReload;
        resources.threadCount = 4;
        engine.resources.update();
        engine.resources.setup<Mesh>("cube")
                .setPostLoad([](Ref<Mesh> &mesh){MeshFactory::createCube(mesh); return true;})
                .setPreLoad(nullptr);
        engine.resources.setup<Mesh>("sphere")
                .setPostLoad([](Ref<Mesh> &mesh){MeshFactory::createSphere(32, 32, mesh); return true;})
                .setPreLoad(nullptr);
        engine.resources.setup<Mesh>("quad")
                .setPostLoad([](Ref<Mesh> &mesh){MeshFactory::createQuad(mesh); return true;})
                .setPreLoad(nullptr);
        engine.resources.setup<Mesh>("circle")
                .setPostLoad([](Ref<Mesh> &mesh){MeshFactory::createRegularPolygon(64, mesh); return true;})
                .setPreLoad(nullptr);
        engine.resources.defaultOptions<Scene>().setPostLoad([](Ref<Scene> &scene){
            bool valid = scene->postLoad();
            engine.swap(*scene);
            engine.resources.remove(engine.resources.getName(scene));
            return valid;
        });

        onUpdate().add("time", [this](){
            time.update();
        });
        onUpdate().add("input", [this](){
            input.update();
        });
        onUpdate().add("resources", [this](){
            resources.update();
        });
        onUpdate().add("window", [this](){
            window.update();
        });

        imguiInit();
        onInitSignal.invoke();
        onInitSignal.setActiveAll(false);
        isEngineInitialized() = true;
    }

    void Engine::beginScene() {
        onBeginSceneSignal.invoke();
    }

    void Engine::update() {
        onInitSignal.invoke();
        onInitSignal.setActiveAll(false);
        onUpdateSignal.invoke();
    }

    void Engine::run() {
        while(window.isOpen()){
            update();
        }
    }

    void Engine::endScene() {
        onEndSceneSignal.invoke();
    }

    void Engine::shutdown() {
        onShutdownSignal.invoke();
    }

    SignalRef<> Engine::onInit(){
        if(isEngineConstructed()){
            return onInitSignal.ref();
        }else{
            return tmpInitSignal().ref();
        }
    }

    SignalRef<> Engine::onBeginScene(){
        return onBeginSceneSignal.ref();
    }

    SignalRef<> Engine::onUpdate(){
        if(isEngineConstructed()){
            return onUpdateSignal.ref();
        }else{
            return tmpUpdateSignal().ref();
        }
    }

    SignalRef<> Engine::onEndScene(){
        return onEndSceneSignal.ref();
    }

    SignalRef<> Engine::onShutdown(){
        return onShutdownSignal.ref();
    }

}

tridot::impl::UpdateSignalRegisterer::UpdateSignalRegisterer(const std::string &name, const std::function<void()> &callback){
    this->id = engine.onUpdate().add(name, callback);
}
tridot::impl::UpdateSignalRegisterer::~UpdateSignalRegisterer(){
    if(!isEngineDestructed()){
        engine.onUpdate().remove(id);
    }
}

tridot::impl::InitSignalRegisterer::InitSignalRegisterer(const std::string &name, const std::function<void()> &callback){
    this->id = engine.onInit().add(name, callback);
}
tridot::impl::InitSignalRegisterer::~InitSignalRegisterer(){
    if(!isEngineDestructed()){
        engine.onInit().remove(id);
    }
}
