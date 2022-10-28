//
// Copyright (c) 2022 Julian Hinxlage. All rights reserved.
//

#include "AnimationComponent.h"
#include "core/core.h"
#include "engine/Time.h"

namespace tri {

	TRI_COMPONENT(AnimationComponent);
	TRI_PROPERTIES6(AnimationComponent, animation, speed, playing, reversed, looping, pingPong);
	TRI_FUNCTION(AnimationComponent, play);
	TRI_FUNCTION(AnimationComponent, stop);
	TRI_FUNCTION(AnimationComponent, unpause);
	TRI_FUNCTION(AnimationComponent, playReverse);

	void AnimationComponent::play() {
		playing = true;

		if (animation) {
			maxTime = animation->calculateMaxTime();
		}

		reversed = false;
		if (reversed) {
			time = maxTime;
		}
		else {
			time = 0;
		}
	}

	void AnimationComponent::stop() {
		playing = false;
	}

	void AnimationComponent::unpause() {
		playing = true;
	}

	void AnimationComponent::playReverse() {
		playing = true;

		if (animation) {
			maxTime = animation->calculateMaxTime();
		}

		reversed = true;
		if (reversed) {
			time = maxTime;
		}
		else {
			time = 0;
		}
	}

	class AnimationSystem : public System {
	public:
		float deltaTime = 0;

		void init() override {
			env->jobManager->addJob("Animation")->addSystem<AnimationSystem>();
		}

		void tick() override {
			deltaTime = env->time->deltaTime;

			env->world->each<AnimationComponent>([&](EntityId id, AnimationComponent& anim) {

				if (anim.playing && anim.animation) {
					float speed = anim.speed * (1 - (float)anim.reversed * 2);
					anim.time += speed * deltaTime;

					if (anim.maxTime == 0) {
						anim.maxTime = anim.animation->calculateMaxTime();
					}

					float overlap = 0;
					if (anim.reversed) {
						overlap = -anim.time;
					}
					else {
						overlap = anim.time - anim.maxTime;
					}
					if (overlap >= 0) {
						if (anim.pingPong) {
							if (anim.reversed && !anim.looping) {
								anim.playing = false;
							}
							anim.reversed = !anim.reversed;

							if (anim.reversed) {
								anim.time -= overlap * 2;
							}
							else {
								anim.time += overlap * 2;
							}
						}
						else {

							if (anim.reversed) {
								anim.time = anim.maxTime - overlap;
							}
							else {
								anim.time = overlap;
							}

							if (!anim.looping) {
								anim.playing = false;
							}
						}
					}

					if (anim.playing) {
						anim.animation->apply(anim.time, id);
					}
					else {
						if (anim.reversed != anim.pingPong) {
							anim.animation->apply(0, id);
						}
						else {
							anim.animation->apply(anim.maxTime, id);
						}
					}

				}

			});

		}
	};
	TRI_SYSTEM(AnimationSystem);

}