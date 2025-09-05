#pragma once

// INCLUDES
#include "Renderer/Descriptor.hpp"
//--------------------------------
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
//----------------------------
#include <iostream>
#include <filesystem>
//-------------------

namespace Brisk 
{
	class MeshAsset {
	public:
        struct Vertex {
            glm::vec3 Position;
            glm::vec3 Normal;
            glm::vec2 UV0;
            glm::vec2 UV1;
            glm::vec3 Color;
            glm::vec4 Tangent;
            glm::uvec4 JointIndices;
            glm::vec4 JointWeights;
        };
        struct MeshletVertex {
            glm::vec3 Position;
            float _pad0;

            glm::vec3 Normal;
            float _pad1;     

            glm::vec2 UV0;           
            glm::vec2 UV1;           

            glm::vec3 Color;         
            float _pad2;             

            glm::vec4 Tangent;

            glm::uvec4 JointIndices;
            glm::vec4 JointWeights; 
        };


        struct Meshlet
        {
            uint32_t Vertices[64];
            uint8_t Indices[126];
            uint8_t IndexCount;
            uint8_t VertexCount;
            uint8_t MaterialIndex = 0;
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
            alignas(16) glm::vec4 baseColorFactor;
            alignas(16) glm::vec3 emissiveFactor;

            // --- Texture Indices ---
            alignas(4) int32_t baseColorTextureIndex = -1;
            alignas(4) int32_t baseColorTextureUV;        
            alignas(4) int32_t metallicRoughnessTextureIndex = -1;
            alignas(4) int32_t metallicRoughnessTextureUV;

            alignas(4) int32_t normalTextureIndex = -1;
            alignas(4) int32_t normalTextureUV;
            alignas(4) int32_t occlusionTextureIndex = -1;
            alignas(4) int32_t occlusionTextureUV;

            alignas(4) int32_t emissiveTextureIndex = -1;
            alignas(4) int32_t emissiveTextureUV;

            // --- Anisotropy ---
            alignas(4) float anisotropyStrength;           
            alignas(4) float anisotropyRotation;           
            alignas(4) int32_t anisotropyTextureIndex = -1;
            alignas(4) int32_t anisotropyTextureUV;       

            // --- Clearcoat ---
            alignas(4) float clearcoatFactor;              
            alignas(4) int32_t clearcoatTextureIndex = -1;
            alignas(4) int32_t clearcoatTextureUV;        
            alignas(4) float clearcoatRoughnessFactor;     

            alignas(4) int32_t clearcoatRoughnessTextureIndex = -1;
            alignas(4) int32_t clearcoatRoughnessTextureUV;
            alignas(4) int32_t clearcoatNormalTextureIndex = -1;
            alignas(4) int32_t clearcoatNormalTextureUV;

            // --- Iridescence ---
            alignas(4) float iridescenceFactor;            
            alignas(4) int32_t iridescenceTextureIndex = -1;
            alignas(4) int32_t iridescenceTextureUV;
            alignas(4) float iridescenceIor;               

            alignas(4) float iridescenceThicknessMinimum;  
            alignas(4) float iridescenceThicknessMaximum;  
            alignas(4) int32_t iridescenceThicknessTextureIndex = -1;
            alignas(4) int32_t iridescenceThicknessTextureUV;

            // --- Sheen ---
            alignas(16) glm::vec3 sheenColorFactor;

            alignas(4) int32_t sheenColorTextureIndex = -1;
            alignas(4) int32_t sheenColorTextureUV;
            alignas(4) float sheenRoughnessFactor;         
            alignas(4) int32_t sheenRoughnessTextureIndex = -1;
            alignas(4) int32_t sheenRoughnessTextureUV;

            // --- Specular ---
            alignas(4) float specularFactor;               
            alignas(4) int32_t specularTextureIndex = -1;
            alignas(4) int32_t specularTextureUV;

            alignas(16) glm::vec3 specularColorFactor;      
            alignas(4) int32_t specularColorTextureIndex = -1;
            alignas(4) int32_t specularColorTextureUV;

            // --- Transmission ---
            alignas(4) float transmissionFactor;           
            alignas(4) int32_t transmissionTextureIndex = -1;
            alignas(4) int32_t transmissionTextureUV;

            // --- Volume ---
            alignas(4) float thicknessFactor;              
            alignas(4) int32_t thicknessTextureIndex = -1;
            alignas(4) int32_t thicknessTextureUV;
            alignas(4) float attenuationDistance;          

            alignas(16) glm::vec3 attenuationColor;
        };


		struct Primitive {
			uint32_t firstIndex = 0;
			uint32_t indexCount = 0;
			uint32_t vertexCount = 0;
            int32_t materialIndex = -1;
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
        void Release();

        uint32_t GetMeshletCount() const { return m_MeshletCount; }

		std::shared_ptr<Buffer> GetIndexBuffer() const { return m_IndexBuffer; }
		std::shared_ptr<Buffer> GetVertexBuffer() const { return m_VertexBuffer; }
	private:
		void LoadNodes(Node* parent, uint32_t nodeIndex, const fastgltf::Asset& asset);
	/*private:*/
	public:
		std::vector<Node*> m_Nodes;
		std::vector<Mesh> m_Meshes;
        uint32_t m_MeshletCount;
		std::vector<std::shared_ptr<Texture>> m_Textures;
		std::vector<MaterialData> m_Materials;
		std::shared_ptr<Buffer> m_VertexBuffer;
		std::shared_ptr<Buffer> m_IndexBuffer;
		std::shared_ptr<Buffer> m_VertexStorageBuffer;
        std::shared_ptr<Buffer> m_MaterialStorageBuffer;
		std::shared_ptr<Buffer> m_MeshletsBuffer;
	};
}