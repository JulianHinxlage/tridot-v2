//
// Copyright (c) 2021 Julian Hinxlage. All rights reserved.
//

#include "Camera.h"
#include "core/core.h"
#include "entity/Scene.h"
#include "Transform.h"
#include "render/Window.h"
#include "render/RenderPipeline.h"
#include "render/RenderThread.h"
#include <glm/gtc/matrix_transform.hpp>

namespace tri {

    TRI_REGISTER_COMPONENT(Camera);
    TRI_REGISTER_MEMBER(Camera, forward);
    TRI_REGISTER_MEMBER(Camera, up);
    TRI_REGISTER_MEMBER(Camera, right);
    TRI_REGISTER_MEMBER(Camera, projection);
    TRI_REGISTER_MEMBER(Camera, output);
    TRI_REGISTER_MEMBER(Camera, isPrimary);
    TRI_REGISTER_MEMBER(Camera, active);
    TRI_REGISTER_MEMBER(Camera, type);
    TRI_REGISTER_MEMBER(Camera, near);
    TRI_REGISTER_MEMBER(Camera, far);
    TRI_REGISTER_MEMBER_RANGE(Camera, fieldOfView, 30, 90);
    TRI_REGISTER_MEMBER(Camera, aspectRatio);

    TRI_REGISTER_TYPE(Camera::Type);
    TRI_REGISTER_CONSTANT(Camera::Type, PERSPECTIVE);
    TRI_REGISTER_CONSTANT(Camera::Type, ORTHOGRAPHIC);

    Camera::Camera(Type type, bool isPrimary) {
        forward = { 0, 0, -1 };
        up = { 0, 1, 0 };
        right = { 1, 0, 0 };
        projection = glm::mat4(1);
        output = nullptr;
        this->isPrimary = isPrimary;
        this->type = type;
        active = true;
        near = 0.01;
        far = 1000;
        fieldOfView = 60;
        aspectRatio = 1.0f;
    }

    TRI_STARTUP_CALLBACK("Camera") {
        env->signals->update.callbackOrder({"Window", "Camera"});
    }

    TRI_UPDATE_CALLBACK("Camera") {

        if (!env->editor) {
            env->renderPipeline->setSize(env->window->getSize().x, env->window->getSize().y);
        }

        env->scene->view<Camera, Transform>().each([](Camera& camera, Transform &transform) {
            glm::mat4 t = transform.getMatrix();
            camera.forward = t * glm::vec4(0, 0, -1, 0);
            camera.right = t * glm::vec4(1, 0, 0, 0);
            camera.up = t * glm::vec4(0, 1, 0, 0);
            camera.viewMatrix = glm::inverse(t);
            if (camera.type == Camera::PERSPECTIVE) {
                camera.projection = glm::perspective(glm::radians(camera.fieldOfView), camera.aspectRatio, camera.near, camera.far) * camera.viewMatrix;
            }else if (camera.type == Camera::ORTHOGRAPHIC) {
                camera.projection = glm::ortho(-transform.scale.x * camera.aspectRatio, transform.scale.x * camera.aspectRatio, -transform.scale.y, transform.scale.y, camera.near, camera.far) * camera.viewMatrix;
            }
            camera.transform = t;

            //frame buffer
            if (camera.active) {
                if (!env->editor) {
                    if (camera.isPrimary) {
                        camera.aspectRatio = env->window->getAspectRatio();
                    }
                }
                if (!camera.output) {
                    if (env->renderPipeline->defaultFrameBufferSpecs.size() > 0) {
                        camera.output = Ref<FrameBuffer>::make();
                        env->renderThread->addTask([output = camera.output]() {
                            output->init(0, 0, env->renderPipeline->defaultFrameBufferSpecs);
                        });
                    }
                }
                else {
                    env->renderPipeline->getPass("clear")->addCommand("clear", CLEAR)->frameBuffer = camera.output;
                    env->renderPipeline->getPass("clear")->addCommand("resize", RESIZE)->frameBuffer = camera.output;
                }
            }
        });
    }

}
