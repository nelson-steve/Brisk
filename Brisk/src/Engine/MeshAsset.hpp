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

#pragma once
#include <glm/glm.hpp>
#include <limits>

        struct MaterialData {
            // --- General ---
            int alphaMode;                        
            float alphaCutoff;                   
            float metallicFactor;                
            float roughnessFactor;              

            float ior;                          
            float dispersion;                   
            int doubleSided;                    
            int unlit;                          

            float emissiveStrength;             
            float _pad0[3];                     // Padding to align next vec4

            glm::vec4 baseColorFactor;          // 16
            glm::vec3 emissiveFactor;           // 12
            float _pad1;                        // Padding to align next uvec2

            // --- Texture Indices ---
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

            // --- Anisotropy ---
            float anisotropyStrength;           
            float anisotropyRotation;           
            uint32_t anisotropyTextureIndex;    
            uint32_t anisotropyTextureUV;       

            // --- Clearcoat ---
            float clearcoatFactor;              
            uint32_t clearcoatTextureIndex;     
            uint32_t clearcoatTextureUV;        
            float clearcoatRoughnessFactor;     

            uint32_t clearcoatRoughnessTextureIndex;
            uint32_t clearcoatRoughnessTextureUV;
            uint32_t clearcoatNormalTextureIndex;
            uint32_t clearcoatNormalTextureUV;

            // --- Iridescence ---
            float iridescenceFactor;            
            uint32_t iridescenceTextureIndex;
            uint32_t iridescenceTextureUV;
            float iridescenceIor;               

            float iridescenceThicknessMinimum;  
            float iridescenceThicknessMaximum;  
            uint32_t iridescenceThicknessTextureIndex;
            uint32_t iridescenceThicknessTextureUV;

            // --- Sheen ---
            glm::vec3 sheenColorFactor;         // 12
            float _pad2;                        // 4

            uint32_t sheenColorTextureIndex;
            uint32_t sheenColorTextureUV;
            float sheenRoughnessFactor;         
            uint32_t sheenRoughnessTextureIndex;
            uint32_t sheenRoughnessTextureUV;
            float _pad3;                        // to align next float to 16

            // --- Specular ---
            float specularFactor;               
            uint32_t specularTextureIndex;
            uint32_t specularTextureUV;
            float _pad4;                        

            glm::vec3 specularColorFactor;      
            float _pad5;                        
            uint32_t specularColorTextureIndex;
            uint32_t specularColorTextureUV;

            // --- Transmission ---
            float transmissionFactor;           
            uint32_t transmissionTextureIndex;
            uint32_t transmissionTextureUV;
            float _pad6;                        

            // --- Volume ---
            float thicknessFactor;              
            uint32_t thicknessTextureIndex;
            uint32_t thicknessTextureUV;
            float attenuationDistance;          

            glm::vec3 attenuationColor;         // 12
            float _pad7;                        // pad to align struct to 16 bytes
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