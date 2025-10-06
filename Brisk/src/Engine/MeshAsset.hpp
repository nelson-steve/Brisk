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
            float vx, vy, vz; // 4 + 4 + 4 = 12
            float nx, ny, nz; // 4 + 4 + 4 = 12
            float tx, ty;     // 4 + 4 = 8
        };

        struct alignas(16) Mesh
        {
            glm::vec3 center;
            float radius;

            uint32_t vertexOffset;
            uint32_t vertexCount;
        };

        struct alignas(16) Meshlet
        {
            glm::vec3 center;
            float radius;
            int8_t cone_axis[3];
            int8_t cone_cutoff;

            uint32_t dataOffset;
            uint32_t baseVertex;
            uint8_t vertexCount;
            uint8_t triangleCount;
            uint8_t shortRefs;
            uint8_t padding;
        };

        struct Geometry
        {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            std::vector<Meshlet> meshlets;
            std::vector<uint32_t> meshletdata;
            std::vector<Mesh> meshes;
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

	public:
		MeshAsset() = default;
		~MeshAsset();
		void Load(const std::filesystem::path& path);
        void Release();

		std::shared_ptr<Buffer> GetIndexBuffer() const { return m_IndexBuffer; }
		std::shared_ptr<Buffer> GetVertexBuffer() const { return m_VertexBuffer; }
    /*private:*/
	public:
        Geometry m_Geometry;
        uint32_t m_MeshletCount;
		std::vector<std::shared_ptr<Texture>> m_Textures;
		std::vector<MaterialData> m_Materials;
		static std::shared_ptr<Buffer> m_VertexBuffer;
		static std::shared_ptr<Buffer> m_IndexBuffer;
		static std::shared_ptr<Buffer> m_MeshletsBuffer;
		static std::shared_ptr<Buffer> m_MeshletDataBuffer;
        static std::shared_ptr<Buffer> m_MaterialStorageBuffer;
		std::shared_ptr<Buffer> m_MeshBuffer;
	};
}