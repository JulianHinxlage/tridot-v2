//
// Copyright (c) 2021 Julian Hinxlage. All rights reserved.
//

#include "tridot/core/Environment.h"
#include "tridot/render/Window.h"
#include "tridot/engine/engine.h"

using namespace tridot;

int main(int argc, char *argv[]) {
    env->console->options.level = DEBUG;
    env->console->addLogFile("log.txt", Console::Options(TRACE, true, true, false));
    env->console->addLogFile("error.txt", Console::Options(ERROR, true, true, false));

    env->events->init.invoke();

    bool running = true;
    env->events->exit.addCallback([&running](){
        running = false;
    });

    while(running){
        env->events->update.invoke();
        env->events->pollEvents();
    }

    env->events->shutdown.invoke();
}
