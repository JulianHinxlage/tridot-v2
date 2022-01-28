//
// Copyright (c) 2021 Julian Hinxlage. All rights reserved.
//

#include "RuntimeMode.h"

namespace tri {

    TRI_REGISTER_SYSTEM_INSTANCE(RuntimeMode, env->runtime);

    void RuntimeMode::startup() {
        env->signals->postStartup.addCallback("RuntimeMode", [&]() {
            if (env->editor) {
                setMode(EDIT);
            }
            else {
                setMode(RUNTIME);
            }
        });
        env->signals->preShutdown.addCallback("RuntimeMode", [&]() {
            setMode(SHUTDOWN);
        });
    }

    RuntimeMode::Mode RuntimeMode::getMode() {
        return mode;
    }

    RuntimeMode::Mode RuntimeMode::getPreviousMode() {
        return previousMode;
    }
    
    void RuntimeMode::setMode(RuntimeMode::Mode mode) {
        if (mode != this->mode) {
            previousMode = this->mode;
            this->mode = mode;

            if (mode == EDIT || mode == PAUSE) {
                env->signals->update.setActiveAll(true);
            }
            else {
                env->signals->update.setActiveAll(true);
            }
            if (mode == PAUSE) {
                for (auto& c : callbacks[EDIT]) {
                    env->signals->update.setActiveCallback(c.name, c.active);
                }
            }
            for (auto& c : callbacks[mode]) {
                env->signals->update.setActiveCallback(c.name, c.active);
            }

            env->signals->runtimeModeChanged.invoke();
        }
    }

    void RuntimeMode::setActive(const std::string& callback, bool active, Mode mode) {
        for (auto& c : callbacks[mode]) {
            if (c.name == callback) {
                c.active = active;
                if (this->mode == mode) {
                    env->signals->update.setActiveCallback(callback, active);
                }
                return;
            }
        }
        callbacks[mode].push_back({ callback, active });
        if (this->mode == mode) {
            env->signals->update.setActiveCallback(callback, active);
        }
    }

}