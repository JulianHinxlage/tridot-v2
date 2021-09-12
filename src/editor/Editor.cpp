//
// Copyright (c) 2021 Julian Hinxlage. All rights reserved.
//

#include "pch.h"
#include "Editor.h"
#include "render/Window.h"
#include "core/util/StrUtil.h"
#include "engine/Serializer.h"
#include "engine/Transform.h"
#include "engine/MeshComponent.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace tri {

	Editor* editor = nullptr;

	TRI_REGISTER_SYSTEM(Editor);

    void createTestScene();

    void Editor::startup(){
		editor = this;
		updated = false;
		runtimeMode = false;
		env->signals->update.callbackOrder({ "Imgui.begin", "Editor", "MeshComponent"});
		env->signals->postStartup.addCallback([](){
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0.3));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.4));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.5));
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1, 1, 1, 0.2));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1, 1, 1, 0.3));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1, 1, 1, 0.4));
			ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(1, 1, 1, 0.1));
			ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(1, 1, 1, 0.25));
			ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(1, 1, 1, 0.40));
			ImGui::PushStyleColor(ImGuiCol_TabUnfocused, ImVec4(1, 1, 1, 0.1));
			ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, ImVec4(1, 1, 1, 0.25));
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1, 1, 1, 0.35));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(1, 1, 1, 0.45));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(1, 1, 1, 0.55));
			ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1, 1, 1, 0.35));
			ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1, 1, 1, 0.35));
			ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1, 1, 1, 0.45));
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.116, 0.125, 0.133, 1));
			ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.177, 0.177, 0.177, 1));
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.238, 0.238, 0.238, 1));
			ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(0.0, 0.32, 1.0, 1));

			ImGuiSettingsHandler handler;
			handler.TypeName = "UserData";
			handler.TypeHash = ImHashStr("UserData");
			handler.ReadOpenFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name) -> void* {
				if (std::string(name) == "OpenFlags") {
					return (void*)1;
				}
				else {
					return nullptr;
				}
			};
			handler.ReadLineFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line) {
				auto parts = StrUtil::split(line, "=");
				if (parts.size() >= 2) {
					for (auto* window : editor->windows) {
						if (window) {
							if (window->name == parts[0]) {
								try {
									window->isOpen = std::stoi(parts[1]);
								}
								catch (...) {}
							}
						}
					}
				}
			};
			handler.WriteAllFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf) {
				buf->append("[UserData][OpenFlags]\n");
				for (auto* window : editor->windows) {
					buf->appendf("%s=%i\n", window->name.c_str(), (int)window->isOpen);
				}
			};
			ImGui::GetCurrentContext()->SettingsHandlers.push_back(handler);
		});
		env->signals->postUpdate.addCallback([this](){
			updated = false;
		});
	}

	void Editor::update() {
		if (!updated && ImGui::GetCurrentContext() && ImGui::GetCurrentContext()->WithinFrameScope) {
			updated = true;
			ImGui::DockSpaceOverViewport();
			updateMenuBar();
			for (auto* window : windows) {
				if (window) {
					if (window->isOpen) {
					    TRI_PROFILE(window->profileName.c_str());
						if (window->isWindow) {
							if (ImGui::Begin(window->name.c_str(), &window->isOpen)) {
								window->update();
							}
							ImGui::End();
						}
						else {
							window->update();
						}
					}
				}
			}
		}
	}

	void Editor::shutdown(){
	    std::filesystem::copy("autosave.scene", "autosave2.scene", std::filesystem::copy_options::overwrite_existing);
        env->serializer->serializeScene("autosave.scene", *env->scene);
		for (auto* window : windows) {
			window->shutdown();
		}
	}

	void Editor::addWindow(EditorWindow* window){
		if (window) {
			windows.push_back(window);
			window->startup();
            window->profileName = "editor/" + window->name;
        }
	}

	void Editor::updateMenuBar() {
		if (ImGui::BeginMainMenuBar()) {

			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open")) {
				    env->scene->clear();
                    env->serializer->deserializeScene("autosave.scene", *env->scene);
                    env->scene->update();
                    env->signals->sceneLoad.invoke(env->scene);
				}
				if (ImGui::MenuItem("Save")) {
                    env->serializer->serializeScene("autosave.scene", *env->scene);
                }
				if (ImGui::MenuItem("Close")) {
				    env->scene->clear();
				}
				if (ImGui::MenuItem("Create Test Scene")) {
                    env->scene->clear();
                    createTestScene();
                    env->scene->update();
                    env->signals->sceneLoad.invoke(env->scene);
                }
                if (ImGui::MenuItem("Exit")) {
                    env->window->close();
                }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View")) {
				for (auto* window : windows) {
					if (window && !window->isDebugWindow) {
						ImGui::MenuItem(window->name.c_str(), nullptr, &window->isOpen);
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Debug")) {
				for (auto* window : windows) {
					if (window && window->isDebugWindow) {
						ImGui::MenuItem(window->name.c_str(), nullptr, &window->isOpen);
					}
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

    float randf() {
        return (float)std::rand() / RAND_MAX;
    }
    glm::vec3 randf3() {
        return { randf(), randf(), randf(), };
    }
	void createTestScene(){
        for (int i = 0; i < 100; i++) {
            env->scene->addEntity(Transform(randf3(), (randf3() * 0.1f) + glm::vec3(0.01, 0.01, 0.01), glm::vec3(0, 0, randf() * 6)), MeshComponent(nullptr, nullptr, Color(glm::vec4(randf3(), 1))));
        }
    }

	class ImguiDemo : public EditorWindow {
	public:
		void startup() {
			name = "ImGui Demo";
			isDebugWindow = true;
			isWindow = false;
		}
		void update() override{
			ImGui::ShowDemoWindow(&isOpen);
		}
	};
	TRI_STARTUP_CALLBACK("") {
		editor->addWindow(new ImguiDemo);
	}

}