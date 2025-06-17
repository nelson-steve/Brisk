#pragma once

// INCLUDES
#include "Renderer/Texture.hpp"
#include "Renderer/Descriptor.hpp"
//-------------------------------
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
//----------------------------
#include <iostream>
#include <filesystem>
//----------------

namespace Brisk 
{
	class MeshAsset {
		struct MeshData {
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec2 UV0;
			glm::vec2 UV1;
			//glm::vec3 Color;
		};

		struct MaterialData {
			enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
			AlphaMode alphaMode = ALPHAMODE_OPAQUE;
			float alphaCutoff = 1.0f;
			float metallicFactor = 1.0f;
			float roughnessFactor = 1.0f;
			glm::vec4 baseColorFactor = glm::vec4(1.0f);
			glm::vec4 emissiveFactor = glm::vec4(0.0f);

			uint32_t baseColorTextureIndex;
			uint32_t metallicRoughnessTextureIndex;
			uint32_t normalTextureIndex;
			uint32_t occlusionTextureIndex;
			uint32_t emissiveTextureIndex;

			std::shared_ptr<Texture> baseColorTexture;
			std::shared_ptr<Texture> metallicRoughnessTexture;
			std::shared_ptr<Texture> normalTexture;
			std::shared_ptr<Texture> occlusionTexture;
			std::shared_ptr<Texture> emissiveTexture;
			bool doubleSided = false;
			struct TexCoordSets {
				uint8_t baseColor = 0;
				uint8_t metallicRoughness = 0;
				uint8_t specularGlossiness = 0;
				uint8_t normal = 0;
				uint8_t occlusion = 0;
				uint8_t emissive = 0;
			} texCoordSets;
			struct Extension {
				std::shared_ptr<Texture> specularGlossinessTexture;
				std::shared_ptr<Texture> diffuseTexture;
				glm::vec4 diffuseFactor = glm::vec4(1.0f);
				glm::vec3 specularFactor = glm::vec3(0.0f);
			} extension;
			struct PbrWorkflows {
				bool metallicRoughness = true;
				bool specularGlossiness = false;
			} pbrWorkflows;
			int index = 0;
			bool unlit = false;
			float emissiveStrength = 1.0f;
		};

		struct Primitive {
			uint32_t first_index = 0;
			uint32_t index_count = 0;
			uint32_t vertex_count = 0;
			int material_index;
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
			//glm::vec3 translation;
			//glm::vec3 scale = glm::vec3(1.0f);
			//glm::quat rotation;
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
		void LoadNodes(Node* parent, uint32_t nodeIndex, const fastgltf::Asset& asset);
		//void GetNodeProps(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertex_count, size_t& index_count);
		//void LoadNode(GLTF_Node* parent, const tinygltf::Node& node, uint32_t node_index, const tinygltf::Model& model);
		//void LoadMaterials(tinygltf::Model model);

		//const std::vector<Node*> GetNodes() const { return m_nodes; }
		//const std::vector<Node*> GetLinearNodes() const { return m_linear_nodes; }
		//const std::vector<MaterialData>& GetMaterials() const { return m_materials; }
		//const MaterialData& GetMaterial(int i) const { return m_materials[i]; }
		//MaterialData& GetMaterial(int i) { return m_materials[i]; }
		std::shared_ptr<Buffer> GetIndexBuffer() const { return m_IndexBuffer; }
		std::shared_ptr<Buffer> GetVertexBuffer() const { return m_VertexBuffer; }
	private:
		std::vector<Node*> m_Nodes;
		std::vector<Mesh> m_Meshes;
		std::vector<std::shared_ptr<Texture>> m_Textures;
		std::vector<MaterialData> m_Materials;
		std::shared_ptr<Buffer> m_VertexBuffer;
		std::shared_ptr<Buffer> m_IndexBuffer;
	};
}