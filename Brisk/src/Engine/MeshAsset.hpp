#pragma once

// INCLUDES
#include "Renderer/Texture.hpp"
#include "Renderer/Descriptor.hpp"
//--------------------------------
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <fastgltf/core.hpp>
//----------------------------
#include <iostream>
#include <filesystem>
//-------------------

namespace Brisk 
{
	class MeshAsset {
	public:
		struct MeshData {
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec2 UV0;
			//glm::vec2 UV1;
			//glm::vec3 Color;
		};

		struct MaterialData {
			enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
			AlphaMode alphaMode = ALPHAMODE_OPAQUE;
			float alphaCutoff = 1.0f;
			float metallicFactor = 1.0f;
			float roughnessFactor = 1.0f;
			float ior = 1.0f;
			float dispersion = 1.0f;
			bool doubleSided = false;
			bool unlit = false;
			float emissiveStrength = 1.0f;

			glm::vec4 baseColorFactor = glm::vec4(1.0f);
			glm::vec3 emissiveFactor = glm::vec3(0.0f);
	
			uint32_t baseColorTextureIndex;
			uint32_t baseColorTextureUV;

			uint32_t metallicRoughnessTextureIndex;
			uint32_t metallicRoughnessTextureUV;

			uint32_t normalTextureIndex;
			uint32_t normalTextureUV;

			uint32_t occlusionTextureIndex;
			uint32_t occlusionTextureUV;

			uint32_t emissiveTextureIndex;
			uint32_t emissiveTextureUV;

			//struct TexCoordSets {
			//	uint8_t baseColor = 0;
			//	uint8_t metallicRoughness = 0;
			//	uint8_t specularGlossiness = 0;
			//	uint8_t normal = 0;
			//	uint8_t occlusion = 0;
			//	uint8_t emissive = 0;
			//} texCoordSets;
			//struct Extension {
			//	std::shared_ptr<Texture> specularGlossinessTexture;
			//	std::shared_ptr<Texture> diffuseTexture;
			//	glm::vec4 diffuseFactor = glm::vec4(1.0f);
			//	glm::vec3 specularFactor = glm::vec3(0.0f);
			//} extension;
			//struct PbrWorkflows {
			//	bool metallicRoughness = true;
			//	bool specularGlossiness = false;
			//} pbrWorkflows;
			//int index = 0;

			// anisotropy
			float anisotropyStrength = 0.0f;
			float anisotropyRotation = 0.0f;
			uint32_t anisotropyTextureIndex;
			uint32_t anisotropyTextureUV;

			// clear coat
			float clearcoatFactor = 0.0f;
			uint32_t clearcoatTextureIndex;
			uint32_t clearcoatTextureUV;
			float clearcoatRoughnessFactor = 0.0f;
			uint32_t clearcoatRoughnessTextureIndex;
			uint32_t clearcoatRoughnessTextureUV;
			uint32_t clearcoatNormalTextureIndex;
			uint32_t clearcoatNormalTextureUV;

			// iridiscence
			float iridescenceFactor = 0.0f;
			uint32_t iridescenceTextureIndex;
			uint32_t iridescenceTextureUV;
			float iridescenceIor = 1.3f;
			float iridescenceThicknessMinimum = 100.0f;
			float iridescenceThicknessMaximum = 400.0f;
			uint32_t iridescenceThicknessTextureIndex;
			uint32_t iridescenceThicknessTextureUV;

			// sheen
			glm::vec3 sheenColorFactor = glm::vec3(1.0f);
			uint32_t sheenColorTextureIndex;
			uint32_t sheenColorTextureUV;
			float sheenRoughnessFactor = 0.0f;
			uint32_t sheenRoughnessTextureIndex;
			uint32_t sheenRoughnessTextureUV;

			// specular
			float specularFactor = 1.0f;
			uint32_t specularTextureIndex;
			uint32_t specularTextureUV;
			glm::vec3 specularColorFactor = glm::vec3(1.0f);
			uint32_t specularColorTextureIndex;
			uint32_t specularColorTextureUV;

			// transmission
			float transmissionFactor = 0.0f;
			uint32_t transmissionTextureIndex;
			uint32_t transmissionTextureUV;

			// materialvolume
			float thicknessFactor = 0.0f;
			uint32_t thicknessTextureIndex;
			uint32_t thicknessTextureUV;
			float attenuationDistance = std::numeric_limits<float>::infinity();
			glm::vec3 attenuationColor = glm::vec3(1.0);
		};

		struct Primitive {
			uint32_t firstIndex = 0;
			uint32_t indexCount = 0;
			uint32_t vertexCount = 0;
			int materialIndex;
			bool has_indices = false;
		};

		struct Mesh {
			std::vector<Primitive> primitives;
		};

		struct Node {
			Node* parent;
			std::vector<Node*> children;
			uint32_t meshIndex;
			glm::mat4 matrix;
			std::string name;
			~Node() {
				for (auto& child : children) {
					delete child;
				}
			}
		};
	public:
		MeshAsset() = default;
		~MeshAsset();
		void Load(const std::filesystem::path& path);

		std::shared_ptr<Buffer> GetIndexBuffer() const { return m_IndexBuffer; }
		std::shared_ptr<Buffer> GetVertexBuffer() const { return m_VertexBuffer; }
	private:
		void LoadNodes(Node* parent, uint32_t nodeIndex, const fastgltf::Asset& asset);
	/*private:*/
	public:
		std::vector<Node*> m_Nodes;
		std::vector<Mesh> m_Meshes;
		std::vector<std::shared_ptr<Texture>> m_Textures;
		std::vector<MaterialData> m_Materials;
		std::shared_ptr<Buffer> m_VertexBuffer;
		std::shared_ptr<Buffer> m_IndexBuffer;
	};
}