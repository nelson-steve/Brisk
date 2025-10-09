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
            uint32_t meshletCount;
            uint32_t meshletOffset;
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

        struct alignas(16) MeshDraw
        {
            glm::vec3 position;
            float scale;
            glm::quat orientation;

            uint32_t meshIndex;
            uint32_t meshletVisibilityOffset;
            uint32_t postPass;
            uint32_t materialIndex;
            uint32_t meshletCount;
            uint32_t meshletOffset;
        };

        struct Geometry
        {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            std::vector<Meshlet> meshlets;
            std::vector<uint32_t> meshletdata;
            std::vector<Mesh> meshes;
            std::vector<MeshDraw> draws;
        };

        struct alignas(16) MaterialData {
            uint32_t alphaMode; // use 4 bytes to avoid packing issues
            float alphaCutoff;
            float metallicFactor;
            float roughnessFactor;

            alignas(16) glm::vec4 baseColorFactor;
            alignas(16) glm::vec3 emissiveFactor;

            float emissiveStrength;

            uint32_t baseColorTextureIndex;
            uint32_t metallicRoughnessTextureIndex;
            uint32_t normalTextureIndex;
            uint32_t occlusionTextureIndex;
            uint32_t emissiveTextureIndex;
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