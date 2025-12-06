#include "pch.hpp"
//// INCLUDES
//
//#include "MeshAsset.hpp"
//#include "Core/Log.hpp"
//#include "Engine.hpp"
//// --------------------
//#include <fastgltf/core.hpp>
//#include <fastgltf/tools.hpp>
////-------------------
//
//#include <meshoptimizer.h>
//
//namespace Brisk {
//	std::shared_ptr<Buffer> MeshAsset::m_VertexBuffer;
//	std::shared_ptr<Buffer> MeshAsset::m_IndexBuffer;
//	std::shared_ptr<Buffer> MeshAsset::m_DrawsBuffer;
//	std::shared_ptr<Buffer> MeshAsset::m_MeshletsBuffer;
//	std::shared_ptr<Buffer> MeshAsset::m_MeshletDataBuffer;
//	std::shared_ptr<Buffer> MeshAsset::m_MaterialStorageBuffer;
//
//	MeshAsset::~MeshAsset() {
//	}
//
//	void MeshAsset::Load(const std::filesystem::path& path) {
//	}
//
//	void MeshAsset::Release() {
//		for (auto texture : m_Textures) {
//			texture->Release();
//		}
//
//		m_MaterialStorageBuffer->Release();
//		m_VertexBuffer->Release();
//		m_IndexBuffer->Release();
//	}
//}