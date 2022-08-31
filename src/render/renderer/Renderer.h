//
// Copyright (c) 2022 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "pch.h"
#include "core/core.h"
#include "DrawList.h"
#include "RenderBatch.h"
#include "ShaderStructs.h"
#include "engine/Camera.h"
#include "engine/Light.h"
#include "engine/Transform.h"
#include "render/objects/Mesh.h"
#include "render/objects/Material.h"
#include "render/objects/FrameBuffer.h"
#include <glm/glm.hpp>

namespace tri {

	class Renderer : public System {
	public:
		void init() override;
		void startup() override;
		void tick() override;
		void shutdown() override;
		void submit(const glm::mat4& transform, Mesh* mesh, Material* material, Color color = color::white, EntityId id = -1);

	private:
		Ref<Mesh> quadMesh;
		Ref<Texture> defaultTexture;
		Ref<Material> defaultMaterial;

		DrawList drawList;
		RenderBatchList batches;
		EnvironmentData envData;
		Ref<Buffer> envBuffer;
		glm::vec3 eyePosition;

		Ref<Shader> geometryShader;
		Ref<Shader> ambientLightShader;
		Ref<Shader> directionalLightShader;
		Ref<Shader> pointLightShader;
		Ref<Shader> spotLightShader;
		Ref<Mesh> sphereMesh;
		Ref<Mesh> coneMesh;

		Ref<FrameBuffer> gBuffer;
		Ref<FrameBuffer> lightAccumulationBuffer;

		std::vector<FrameBufferAttachmentSpec> gBufferSpec;
		std::vector<FrameBufferAttachmentSpec> lightAccumulationSpec;

		class LightBatch {
		public:
			class Instance {
			public:
				glm::mat4 transform;
				glm::vec3 position;
				glm::vec3 direction;
				Color color;
				float intensity;
				float range;
				float falloff;
				float spotAngle;
			};

			Ref<BatchBuffer> instanceBuffer;
			Ref<VertexArray> vertexArray;
			bool hasPrepared = false;
		};
		LightBatch pointLightBatch;
		LightBatch spotLightBatch;


		void setupSpecs();
		bool updateFrameBuffer(Ref<FrameBuffer>& frameBuffer, const std::vector<FrameBufferAttachmentSpec> &spec);
		bool prepareLightBatches();
		void submitMeshes();
		void submitBatches(Camera& c, FrameBuffer* frameBuffer);
		void submitLights(const Camera &camera);
		bool submitLight(FrameBuffer* lightBuffer, FrameBuffer* gBuffer, const Light &light, const Transform& transform, const Camera& camera);
		void submitPointLightBatch();
		void submitSpotLightBatch();
	};

}
