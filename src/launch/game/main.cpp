//
// Copyright (c) 2022 Julian Hinxlage. All rights reserved.
//

#include "core.h"
#include "window/Input.h"
#include "window/Window.h"
using namespace tri;

int main(int argc, char* argv[]) {
    MainLoop::init();

    std::vector<std::string> configFiles = { "./game.cfg", "../game.cfg", "../../game.cfg" };

    //read arguments
    bool configFlag = false;
    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-c") {
            configFlag = true;
        }
        else if (configFlag) {
            configFlag = false;
            configFiles = { arg };
        }
    }


    env->config->loadConfigFileFirstFound(configFiles);
    MainLoop::startup();

    //wait for all assets to be loaded before starting the scene
    env->console->executeCommand("waitForAllAssetsLoaded");

    env->eventManager->postTick.addListener([]() {
        if (env->input->down(Input::KEY_ESCAPE)) {
            env->window->close();
        }
    });

    MainLoop::run();
    MainLoop::shutdown();
    return 0;
}

#if WIN32
int __stdcall WinMain(void* hInstance, void* hPrevInstance, char* lpCmdLine, int nCmdShow) {
    auto args = StrUtil::split(lpCmdLine, " ", false);
    std::vector<const char*> argv;
    argv.reserve(args.size());
    for (auto& a : args) {
        argv.push_back(a.c_str());
    }
    int argc = argv.size();
    return main(argc, (char**)argv.data());
}
#endif
