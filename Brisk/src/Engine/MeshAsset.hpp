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
            // --- General ---
            alignas(4) int alphaMode;
            alignas(4) float alphaCutoff;                   
            alignas(4) float metallicFactor;                
            alignas(4) float roughnessFactor;              

            alignas(4) float ior;                          
            alignas(4) float dispersion;                   
            alignas(4) int doubleSided;                    
            alignas(4) int unlit;                          

            alignas(4) float emissiveStrength;             
            alignas(16) glm::vec4 baseColorFactor;          // 16
            alignas(16) glm::vec3 emissiveFactor;           // 12

            // --- Texture Indices ---
            alignas(4) int32_t baseColorTextureIndex;     
            alignas(4) int32_t baseColorTextureUV;        
            alignas(4) int32_t metallicRoughnessTextureIndex;
            alignas(4) int32_t metallicRoughnessTextureUV;

            alignas(4) int32_t normalTextureIndex;
            alignas(4) int32_t normalTextureUV;
            alignas(4) int32_t occlusionTextureIndex;
            alignas(4) int32_t occlusionTextureUV;

            alignas(4) int32_t emissiveTextureIndex;
            alignas(4) int32_t emissiveTextureUV;

            // --- Anisotropy ---
            alignas(4) float anisotropyStrength;           
            alignas(4) float anisotropyRotation;           
            alignas(4) int32_t anisotropyTextureIndex;    
            alignas(4) int32_t anisotropyTextureUV;       

            // --- Clearcoat ---
            alignas(4) float clearcoatFactor;              
            alignas(4) int32_t clearcoatTextureIndex;     
            alignas(4) int32_t clearcoatTextureUV;        
            alignas(4) float clearcoatRoughnessFactor;     

            alignas(4) int32_t clearcoatRoughnessTextureIndex;
            alignas(4) int32_t clearcoatRoughnessTextureUV;
            alignas(4) int32_t clearcoatNormalTextureIndex;
            alignas(4) int32_t clearcoatNormalTextureUV;

            // --- Iridescence ---
            alignas(4) float iridescenceFactor;            
            alignas(4) int32_t iridescenceTextureIndex;
            alignas(4) int32_t iridescenceTextureUV;
            alignas(4) float iridescenceIor;               

            alignas(4) float iridescenceThicknessMinimum;  
            alignas(4) float iridescenceThicknessMaximum;  
            alignas(4) int32_t iridescenceThicknessTextureIndex;
            alignas(4) int32_t iridescenceThicknessTextureUV;

            // --- Sheen ---
            alignas(16) glm::vec3 sheenColorFactor;         // 12

            alignas(4) int32_t sheenColorTextureIndex;
            alignas(4) int32_t sheenColorTextureUV;
            alignas(4) float sheenRoughnessFactor;         
            alignas(4) int32_t sheenRoughnessTextureIndex;
            alignas(4) int32_t sheenRoughnessTextureUV;

            // --- Specular ---
            alignas(4) float specularFactor;               
            alignas(4) int32_t specularTextureIndex;
            alignas(4) int32_t specularTextureUV;

            alignas(16) glm::vec3 specularColorFactor;      
            alignas(4) int32_t specularColorTextureIndex;
            alignas(4) int32_t specularColorTextureUV;

            // --- Transmission ---
            alignas(4) float transmissionFactor;           
            alignas(4) int32_t transmissionTextureIndex;
            alignas(4) int32_t transmissionTextureUV;

            // --- Volume ---
            alignas(4) float thicknessFactor;              
            alignas(4) int32_t thicknessTextureIndex;
            alignas(4) int32_t thicknessTextureUV;
            alignas(4) float attenuationDistance;          

            alignas(16) glm::vec3 attenuationColor;         // 12
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