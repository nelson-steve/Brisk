// INCLUDES
#include "Scene.hpp"
#include "Entity.hpp"
#include "Component.hpp"
#include "Engine/Engine.hpp"
#include "Core/Log.hpp"
//--------------------------
#include "imgui.h"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <glm/glm.hpp>
#include <meshoptimizer.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
//-------------------------------------

namespace Brisk 
{
	static void decomposeTransform(float translation[3], float rotation[4], float scale[3], const float* transform)
	{
		float m[4][4] = {};
		memcpy(m, transform, 16 * sizeof(float));

		// extract translation from last row
		translation[0] = m[3][0];
		translation[1] = m[3][1];
		translation[2] = m[3][2];

		// compute determinant to determine handedness
		float det =
			m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
			m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
			m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

		float sign = (det < 0.f) ? -1.f : 1.f;

		// recover scale from axis lengths
		scale[0] = sqrtf(m[0][0] * m[0][0] + m[0][1] * m[0][1] + m[0][2] * m[0][2]) * sign;
		scale[1] = sqrtf(m[1][0] * m[1][0] + m[1][1] * m[1][1] + m[1][2] * m[1][2]) * sign;
		scale[2] = sqrtf(m[2][0] * m[2][0] + m[2][1] * m[2][1] + m[2][2] * m[2][2]) * sign;

		// normalize axes to get a pure rotation matrix
		float rsx = (scale[0] == 0.f) ? 0.f : 1.f / scale[0];
		float rsy = (scale[1] == 0.f) ? 0.f : 1.f / scale[1];
		float rsz = (scale[2] == 0.f) ? 0.f : 1.f / scale[2];

		float r00 = m[0][0] * rsx, r10 = m[1][0] * rsy, r20 = m[2][0] * rsz;
		float r01 = m[0][1] * rsx, r11 = m[1][1] * rsy, r21 = m[2][1] * rsz;
		float r02 = m[0][2] * rsx, r12 = m[1][2] * rsy, r22 = m[2][2] * rsz;

		// "branchless" version of Mike Day's matrix to quaternion conversion
		int qc = r22 < 0 ? (r00 > r11 ? 0 : 1) : (r00 < -r11 ? 2 : 3);
		float qs1 = qc & 2 ? -1.f : 1.f;
		float qs2 = qc & 1 ? -1.f : 1.f;
		float qs3 = (qc - 1) & 2 ? -1.f : 1.f;

		float qt = 1.f - qs3 * r00 - qs2 * r11 - qs1 * r22;
		float qs = 0.5f / sqrtf(qt);

		rotation[qc ^ 0] = qs * qt;
		rotation[qc ^ 1] = qs * (r01 + qs1 * r10);
		rotation[qc ^ 2] = qs * (r20 + qs2 * r02);
		rotation[qc ^ 3] = qs * (r12 + qs3 * r21);
	}

	glm::mat4 GetNodeLocalMatrix(const fastgltf::Node& node) {
		if (std::holds_alternative<fastgltf::TRS>(node.transform)) {
			const auto& trs = std::get<fastgltf::TRS>(node.transform);

			glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::make_vec3(trs.translation.data()));
			glm::mat4 rotation = glm::mat4_cast(glm::make_quat(trs.rotation.value_ptr()));
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::make_vec3(trs.scale.data()));

			return translation * rotation * scale;
		}
		else if (std::holds_alternative<fastgltf::math::fmat4x4>(node.transform)) {
			const auto& matrix = std::get<fastgltf::math::fmat4x4>(node.transform);
			return glm::make_mat4x4(matrix.data());
		}

		BRISK_CORE_ERROR("Node does not have any transform");
		return glm::mat4(1.0f);
	}

	std::optional<size_t> GetParentIndex(const fastgltf::Asset& asset, size_t child) {
		int parentIndex = 0;
		for (const fastgltf::Node& node : asset.nodes) {
			for (size_t index : node.children) {
				if (index == child)
					return std::make_optional<size_t>(parentIndex);
			}
			parentIndex++;
		}
		return std::nullopt;
	}

	glm::mat4 GetWorldTransform(const fastgltf::Asset& asset, size_t nodeIndex) {
		if (!GetParentIndex(asset, nodeIndex).has_value())
			return GetNodeLocalMatrix(asset.nodes[nodeIndex]);

		glm::mat4 child = GetNodeLocalMatrix(asset.nodes[nodeIndex]);
		while (GetParentIndex(asset, nodeIndex).has_value()) {
			glm::mat4 parent = GetNodeLocalMatrix(asset.nodes[GetParentIndex(asset, nodeIndex).value()]);
			child = parent * child;

			nodeIndex = GetParentIndex(asset, nodeIndex).value();
		}

		return child;
	}

	void Scene::LoadGltfScene(const std::filesystem::path& gltfPath) {
		// Validating path
		if (!std::filesystem::exists(gltfPath)) {
			std::cout << "Failed to find " << gltfPath << '\n';
		}

		if constexpr (std::is_same_v<std::filesystem::path::value_type, wchar_t>) {
			std::cout << "Loading " << gltfPath << '\n';
		}
		else {
			std::cout << "Loading " << gltfPath << '\n';
		}

		fastgltf::Asset asset;

		// Parse the glTF file and get the constructed asset
		{
			static constexpr auto supportedExtensions =
				fastgltf::Extensions::KHR_mesh_quantization |
				fastgltf::Extensions::KHR_texture_transform |
				fastgltf::Extensions::KHR_materials_variants;

			fastgltf::Parser parser(supportedExtensions);

			constexpr auto gltfOptions =
				fastgltf::Options::DontRequireValidAssetMember |
				//fastgltf::Options::AllowDouble |
				fastgltf::Options::LoadExternalBuffers |
				fastgltf::Options::LoadExternalImages |
				fastgltf::Options::DecomposeNodeMatrices |
				fastgltf::Options::GenerateMeshIndices;

			auto gltfFile = fastgltf::MappedGltfFile::FromPath(gltfPath);
			if (!bool(gltfFile)) {
				std::cerr << "Failed to open glTF file: " << fastgltf::getErrorMessage(gltfFile.error()) << '\n';
			}

			auto a = parser.loadGltf(gltfFile.get(), gltfPath.parent_path(), gltfOptions);
			if (a.error() != fastgltf::Error::None) {
				std::cerr << "Failed to load glTF: " << fastgltf::getErrorMessage(a.error()) << '\n';
			}

			asset = std::move(a.get());
		}

		if (asset.buffers.empty()) {
			throw std::runtime_error("GLTF file has no buffers");
		}

		if (asset.scenes.empty()) return;
		auto& scene = asset.scenes[asset.defaultScene.value_or(0)];

		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		for (const auto& gltfMesh : asset.meshes) {
			for (auto it = gltfMesh.primitives.begin(); it != gltfMesh.primitives.end(); ++it) {
				const fastgltf::Attribute* positionIt = it->findAttribute("POSITION");

				BRISK_CORE_ASSERT(positionIt != it->attributes.end());
				BRISK_CORE_ASSERT(it->indicesAccessor.has_value());

				auto& positionAccessor = asset.accessors[positionIt->accessorIndex];
				if (!positionAccessor.bufferViewIndex.has_value())
					continue;

				if (positionAccessor.componentType == fastgltf::ComponentType::Float &&
					positionAccessor.type == fastgltf::AccessorType::Vec3) {
					vertexCount += positionAccessor.count;
				}

				if (it->indicesAccessor.has_value()) {
					const auto& indexAccessor = asset.accessors[it->indicesAccessor.value()];
					indexCount += indexAccessor.count;
				}
			}
		}

		uint32_t maxVertexCount = 0;

		m_Geometry.vertices.reserve(m_Geometry.vertices.size() + vertexCount);
		m_Geometry.indices.reserve(m_Geometry.indices.size() + indexCount);

		std::vector<std::pair<uint32_t, uint32_t>> primitives;
		std::vector<uint32_t> primitiveMaterials;

		size_t firstMeshOffset = m_Geometry.meshes.size();

		for (const auto& gltfMesh : asset.meshes) {

			size_t meshOffset = m_Geometry.meshes.size();

			for (auto it = gltfMesh.primitives.begin(); it != gltfMesh.primitives.end(); ++it) {
				primitiveMaterials.push_back(it->materialIndex.value());

				const fastgltf::Attribute* positionIt = it->findAttribute("POSITION");
				const fastgltf::Attribute* normalIt = it->findAttribute("NORMAL");
				const fastgltf::Attribute* texCoord0It = it->findAttribute("TEXCOORD_0");
				const fastgltf::Attribute* texCoord1It = it->findAttribute("TEXCOORD_1");
				const fastgltf::Attribute* colorIt = it->findAttribute("COLOR_0");
				const fastgltf::Attribute* tangentIt = it->findAttribute("TANGENT"); // The W component of each TANGENT accessor element MUST be set to 1.0 or -1.0
				const fastgltf::Attribute* jointsIt = it->findAttribute("JOINTS_0");
				const fastgltf::Attribute* weightsIt = it->findAttribute("WEIGHTS_0");

				BRISK_CORE_ASSERT(positionIt != it->attributes.end());
				BRISK_CORE_ASSERT(it->indicesAccessor.has_value());

				auto& positionAccessor = asset.accessors[positionIt->accessorIndex];
				if (!positionAccessor.bufferViewIndex.has_value())
					continue;

				// Load positions
				std::vector<fastgltf::math::fvec3> positions;
				if (positionAccessor.componentType == fastgltf::ComponentType::Float &&
					positionAccessor.type == fastgltf::AccessorType::Vec3) {
					positions.resize(positionAccessor.count);
					fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, positionAccessor, positions.data());
				}

				// Load normals
				std::vector<fastgltf::math::fvec3> normals;
				bool hasNormals = (normalIt != it->attributes.end());
				if (hasNormals) {
					auto& normalAccessor = asset.accessors[normalIt->accessorIndex];
					if (normalAccessor.componentType == fastgltf::ComponentType::Float &&
						normalAccessor.type == fastgltf::AccessorType::Vec3 &&
						normalAccessor.bufferViewIndex.has_value()) {
						normals.resize(normalAccessor.count);
						fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, normalAccessor, normals.data());
					}
					else {
						hasNormals = false;
					}
				}

				// Load texcoords 0
				std::vector<fastgltf::math::fvec2> texcoords0;
				bool hasTexcoords0 = (texCoord0It != it->attributes.end());
				if (hasTexcoords0) {
					auto& texcoordAccessor = asset.accessors[texCoord0It->accessorIndex];
					if (texcoordAccessor.componentType == fastgltf::ComponentType::Float &&
						texcoordAccessor.type == fastgltf::AccessorType::Vec2 &&
						texcoordAccessor.bufferViewIndex.has_value()) {
						texcoords0.resize(texcoordAccessor.count);
						fastgltf::copyFromAccessor<fastgltf::math::fvec2>(asset, texcoordAccessor, texcoords0.data());
					}
					else {
						hasTexcoords0 = false;
					}
				}

				// Load texcoords 1
				std::vector<fastgltf::math::fvec2> texcoords1;
				bool hasTexcoords1 = (texCoord1It != it->attributes.end());
				if (hasTexcoords1) {
					auto& texcoordAccessor = asset.accessors[texCoord1It->accessorIndex];
					if (texcoordAccessor.componentType == fastgltf::ComponentType::Float &&
						texcoordAccessor.type == fastgltf::AccessorType::Vec2 &&
						texcoordAccessor.bufferViewIndex.has_value()) {
						texcoords1.resize(texcoordAccessor.count);
						fastgltf::copyFromAccessor<fastgltf::math::fvec2>(asset, texcoordAccessor, texcoords1.data());
					}
					else {
						hasTexcoords1 = false;
					}
				}

				// Load color
				//std::vector<fastgltf::math::fvec3> color;
				//bool hasColor = (colorIt != it->attributes.end());
				//if (hasColor) {
				//	auto& colorAccessor = asset.accessors[colorIt->accessorIndex];
				//	if (colorAccessor.componentType == fastgltf::ComponentType::Float &&
				//		colorAccessor.type == fastgltf::AccessorType::Vec3 &&
				//		colorAccessor.bufferViewIndex.has_value()) {
				//		color.resize(colorAccessor.count);
				//		fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, colorAccessor, color.data());
				//	}
				//	else {
				//		hasColor = false;
				//	}
				//}

				// Load tangent
				std::vector<fastgltf::math::fvec4> tangent;
				bool hasTangent = (tangentIt != it->attributes.end());
				if (hasTangent) {
					auto& tangentAccessor = asset.accessors[tangentIt->accessorIndex];
					if (tangentAccessor.componentType == fastgltf::ComponentType::Float &&
						tangentAccessor.type == fastgltf::AccessorType::Vec4 &&
						tangentAccessor.bufferViewIndex.has_value()) {
						tangent.resize(tangentAccessor.count);
						fastgltf::copyFromAccessor<fastgltf::math::fvec4>(asset, tangentAccessor, tangent.data());
					}
					else {
						hasTangent = false;
					}
				}

				// Load joint indices
				//std::vector<fastgltf::math::uvec4> jointIndices;
				//bool hasjointIndices = (jointsIt != it->attributes.end());
				//if (hasjointIndices) {
				//	auto& jointIndicesAccessor = asset.accessors[jointsIt->accessorIndex];
				//	if (jointIndicesAccessor.componentType == fastgltf::ComponentType::UnsignedInt &&
				//		jointIndicesAccessor.type == fastgltf::AccessorType::Vec4 &&
				//		jointIndicesAccessor.bufferViewIndex.has_value()) {
				//		jointIndices.resize(jointIndicesAccessor.count);
				//		fastgltf::copyFromAccessor<fastgltf::math::uvec4>(asset, jointIndicesAccessor, jointIndices.data());
				//	}
				//	else {
				//		hasjointIndices = false;
				//	}
				//}

				// Load joint weights
				//std::vector<fastgltf::math::uvec4> jointWeights;
				//bool hasjointWeights = (weightsIt != it->attributes.end());
				//if (hasjointWeights) {
				//	auto& jointWeightsAccessor = asset.accessors[weightsIt->accessorIndex];
				//	if (jointWeightsAccessor.componentType == fastgltf::ComponentType::UnsignedInt &&
				//		jointWeightsAccessor.type == fastgltf::AccessorType::Vec4 &&
				//		jointWeightsAccessor.bufferViewIndex.has_value()) {
				//		jointWeights.resize(jointWeightsAccessor.count);
				//		fastgltf::copyFromAccessor<fastgltf::math::uvec4>(asset, jointWeightsAccessor, jointWeights.data());
				//	}
				//	else {
				//		hasjointWeights = false;
				//	}
				//}

				std::vector<uint32_t> indices;
				std::vector<Vertex> vertices;
				std::vector<glm::vec3> localPositions;
				for (size_t i = 0; i < positions.size(); ++i) {
					Vertex vertex{};
					// Positions
					vertex.vx = positions[i].x();
					vertex.vy = positions[i].y();
					vertex.vz = positions[i].z();

					// Normals
					if (hasNormals && i < normals.size()) {
						vertex.nx = normals[i].x();
						vertex.ny = normals[i].y();
						vertex.nz = normals[i].z();
					}

					//  UV0
					if (hasTexcoords0 && i < texcoords0.size()) {
						vertex.tx = texcoords0[i].x();
						vertex.ty = texcoords0[i].y();
					}

					vertices.push_back(vertex);
				}

				if (it->indicesAccessor.has_value()) {
					const auto& indexAccessor = asset.accessors[it->indicesAccessor.value()];
					indexCount = indexAccessor.count;

					switch (indexAccessor.componentType) {
					case fastgltf::ComponentType::UnsignedByte: {
						std::vector<uint8_t> tempIndices(indexAccessor.count);
						fastgltf::copyFromAccessor<uint8_t>(asset, indexAccessor, tempIndices.data());
						for (uint8_t i : tempIndices) {
							indices.push_back(static_cast<uint32_t>(i));
						}
						break;
					}
					case fastgltf::ComponentType::UnsignedShort: {
						std::vector<uint16_t> tempIndices(indexAccessor.count);
						fastgltf::copyFromAccessor<uint16_t>(asset, indexAccessor, tempIndices.data());
						for (uint16_t i : tempIndices) {
							indices.push_back(static_cast<uint32_t>(i));
						}
						break;
					}
					case fastgltf::ComponentType::UnsignedInt: {
						std::vector<uint32_t> tempIndices(indexAccessor.count);
						fastgltf::copyFromAccessor<uint32_t>(asset, indexAccessor, tempIndices.data());
						for (uint32_t i : tempIndices) {
							indices.push_back(static_cast<uint32_t>(i));
						}
						break;
					}
					default:
						throw std::runtime_error("Unsupported index component type.");
					}
				}

				std::vector<uint32_t> remap(vertices.size());
				size_t uniqueVertices = meshopt_generateVertexRemap(remap.data(), indices.data(), indices.size(), vertices.data(), vertices.size(), sizeof(Vertex));

				meshopt_remapVertexBuffer(vertices.data(), vertices.data(), vertices.size(), sizeof(Vertex), remap.data());
				meshopt_remapIndexBuffer(indices.data(), indices.data(), indices.size(), remap.data());

				vertices.resize(uniqueVertices);

				//if (fast)
				//	meshopt_optimizeVertexCacheFifo(indices.data(), indices.data(), indices.size(), vertices.size(), 16);
				//else
				meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertices.size());

				meshopt_optimizeVertexFetch(vertices.data(), indices.data(), indices.size(), vertices.data(), vertices.size(), sizeof(Vertex));

				Mesh mesh{};
				mesh.vertexOffset = uint32_t(m_Geometry.vertices.size());
				mesh.vertexCount = uint32_t(vertices.size());

				m_Geometry.vertices.insert(m_Geometry.vertices.end(), vertices.begin(), vertices.end());

				glm::vec3 center{ 0.0f };
				for (auto& v : vertices) {
					center += glm::vec3(v.vx, v.vy, v.vz);
					localPositions.push_back(glm::vec3(v.vx, v.vy, v.vz));
				}
				center = center / float(vertices.size());

				float radius = 0;
				for (auto& v : vertices) {
					radius = std::max(radius, glm::distance(center, glm::vec3(v.vx, v.vy, v.vz)));
				}

				mesh.center = center;
				mesh.radius = radius;

				m_Geometry.indices.insert(m_Geometry.indices.end(), indices.begin(), indices.end());

				//if (fast)
				//	meshopt_optimizeVertexCacheFifo(lodIndices.data(), lodIndices.data(), lodIndices.size(), vertices.size(), 16);
				//else
				meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertices.size());

#define MESH_MAXVTX 64
#define MESH_MAXTRI 96

				const size_t max_vertices = MESH_MAXVTX;
				const size_t min_triangles = MESH_MAXTRI / 4;
				const size_t max_triangles = MESH_MAXTRI;
				const float cone_weight = 0.25f;
				const float fill_weight = 0.5f;

				std::vector<meshopt_Meshlet> meshlets(meshopt_buildMeshletsBound(indices.size(), max_vertices, min_triangles));
				std::vector<unsigned int> meshlet_vertices(meshlets.size() * max_vertices);
				std::vector<unsigned char> meshlet_triangles(meshlets.size() * max_triangles * 3);

				//if (fast)
				//	meshlets.resize(meshopt_buildMeshletsScan(meshlets.data(), meshlet_vertices.data(), meshlet_triangles.data(), indices.data(), indices.size(), vertices.size(), max_vertices, max_triangles));
				//else if (clrt && lod0) // only use spatial algo for lod0 as this is the only lod that is used for raytracing
				//	meshlets.resize(meshopt_buildMeshletsSpatial(meshlets.data(), meshlet_vertices.data(), meshlet_triangles.data(), indices.data(), indices.size(), &vertices[0].x, vertices.size(), sizeof(vec3), max_vertices, min_triangles, max_triangles, fill_weight));
				//else
				meshlets.resize(meshopt_buildMeshlets(meshlets.data(), meshlet_vertices.data(), meshlet_triangles.data(), indices.data(), indices.size(), &localPositions[0].x, localPositions.size(), sizeof(glm::vec3), max_vertices, max_triangles, cone_weight));

				for (auto& meshlet : meshlets)
				{
					meshopt_optimizeMeshlet(&meshlet_vertices[meshlet.vertex_offset], &meshlet_triangles[meshlet.triangle_offset], meshlet.triangle_count, meshlet.vertex_count);

					size_t dataOffset = m_Geometry.meshletdata.size();

					unsigned int minVertex = ~0u, maxVertex = 0;
					for (unsigned int i = 0; i < meshlet.vertex_count; ++i)
					{
						minVertex = std::min(meshlet_vertices[meshlet.vertex_offset + i], minVertex);
						maxVertex = std::max(meshlet_vertices[meshlet.vertex_offset + i], maxVertex);
					}

					bool shortRefs = maxVertex - minVertex < (1 << 16);

					for (unsigned int i = 0; i < meshlet.vertex_count; ++i)
					{
						unsigned int ref = meshlet_vertices[meshlet.vertex_offset + i] - minVertex;
						if (shortRefs && i % 2)
							m_Geometry.meshletdata.back() |= ref << 16;
						else
							m_Geometry.meshletdata.push_back(ref);
					}

					const unsigned int* indexGroups = reinterpret_cast<const unsigned int*>(&meshlet_triangles[0] + meshlet.triangle_offset);
					unsigned int indexGroupCount = (meshlet.triangle_count * 3 + 3) / 4;

					for (unsigned int i = 0; i < indexGroupCount; ++i)
						m_Geometry.meshletdata.push_back(indexGroups[i]);

					Meshlet m = {};
					m.dataOffset = uint32_t(dataOffset);
					m.baseVertex = mesh.vertexOffset + minVertex;
					m.triangleCount = meshlet.triangle_count;
					m.vertexCount = meshlet.vertex_count;
					m.shortRefs = shortRefs;

					m_Geometry.meshlets.push_back(m);
				}
				mesh.materialIndex = it->materialIndex.value() + m_Materials.size();
				mesh.meshletCount = uint32_t(meshlets.size());
				mesh.meshletOffset = uint32_t(m_Geometry.meshlets.size() - meshlets.size());

				m_Geometry.meshes.push_back(mesh);
			}

			primitives.push_back(std::make_pair(meshOffset, m_Geometry.meshes.size() - meshOffset));
		}

		for (int nodeIndex = 0; nodeIndex < asset.nodes.size(); nodeIndex++) {
			if (!asset.nodes[nodeIndex].meshIndex.has_value()) continue;
			std::pair<uint32_t, uint32_t> range = primitives[asset.nodes[nodeIndex].meshIndex.value()];

			std::cout << "Node " << asset.nodes[nodeIndex].name << ":\n";
			for (int i = 0; i < range.second; i++) {
				const auto& trs = std::get<fastgltf::TRS>(asset.nodes[nodeIndex].transform);

				glm::mat4 mat = GetWorldTransform(asset, nodeIndex);

				float scale[3];
				float rotation[4];
				float translation[3];

				//glm::decompose(mat, scale, rotation, translation, skew, perspective);

				decomposeTransform(translation, rotation, scale, glm::value_ptr(mat));

				MeshDraw draw = {};
				draw.position = glm::vec3(translation[0], translation[1], translation[2]);
				draw.scale = std::max(scale[0], std::max(scale[1], scale[2]));
				draw.orientation = glm::vec4(rotation[0], rotation[1], rotation[2], rotation[3]);

				draw.meshIndex = range.first + i;
				draw.materialIndex = m_Geometry.meshes[draw.meshIndex].materialIndex;
				draw.meshletCount = m_Geometry.meshes[draw.meshIndex].meshletCount;
				draw.meshletOffset = m_Geometry.meshes[draw.meshIndex].meshletOffset;

				draw.groupCountX = m_Geometry.meshes[draw.meshIndex].meshletCount;
				draw.groupCountY = 1;
				draw.groupCountZ = 1;

				m_Geometry.draws.push_back(draw);

				std::cout << "MeshDraw " << (m_Geometry.draws.size() - 1) << ":\n"
					<< "  Mesh Index: " << draw.meshIndex << "\n"
					<< "  Translation: (" << draw.position.x << ", " << draw.position.y << ", " << draw.position.z << ")\n"
					<< "  Scale: (" << draw.scale << ")\n"
					<< "  Rotation (quat): (" << draw.orientation.x << ", " << draw.orientation.y << ", " << draw.orientation.z << ", " << draw.orientation.w << ")\n"
					<< "----------------------------------------\n";
			}
		}

		for (const auto& node : asset.nodes) {
			if (!node.meshIndex.has_value()) continue;

			Entity e = CreateEntity(node.name.c_str());
			MeshComponent& mc = e.AddComponent<MeshComponent>();
			 std::pair<uint32_t, uint32_t> range = primitives[node.meshIndex.value()];
			for (int i = 0; i < range.second; i++) {
				mc.p_SubMeshIndices.push_back(range.first + i);
			}
		}

		
		Application::GetRenderer()->m_TransferCmdBuffer->Bind();
		Application::GetRenderer()->m_DrawsBuffer->RecordUpload(Application::GetRenderer()->m_TransferCmdBuffer, sizeof(m_Geometry.draws[0]) * m_Geometry.draws.size(), m_Geometry.draws.data());

		Application::GetRenderer()->m_IndexBuffer->RecordUpload(Application::GetRenderer()->m_TransferCmdBuffer, sizeof(m_Geometry.indices[0]) * m_Geometry.indices.size(), m_Geometry.indices.data());

		Application::GetRenderer()->m_VertexBuffer->RecordUpload(Application::GetRenderer()->m_TransferCmdBuffer, sizeof(m_Geometry.vertices[0]) * m_Geometry.vertices.size(), m_Geometry.vertices.data());

		Application::GetRenderer()->m_MeshletDataBuffer->RecordUpload(Application::GetRenderer()->m_TransferCmdBuffer, sizeof(m_Geometry.meshletdata[0]) * m_Geometry.meshletdata.size(), m_Geometry.meshletdata.data());

		Application::GetRenderer()->m_MeshletsBuffer->RecordUpload(Application::GetRenderer()->m_TransferCmdBuffer, sizeof(m_Geometry.meshlets[0]) * m_Geometry.meshlets.size(), m_Geometry.meshlets.data());

		uint32_t texturesOffset = Engine::s_TexturesOffset;

		m_Materials.reserve(asset.materials.size());
		for (const auto& material : asset.materials) {
			MaterialData outMaterial{};
			BRISK_CORE_INFO("Apha mode: {}", (int)material.alphaMode);
			outMaterial.alphaMode = (int)material.alphaMode;
			outMaterial.alphaCutoff = material.alphaCutoff;
			outMaterial.metallicFactor = material.pbrData.metallicFactor;
			outMaterial.roughnessFactor = material.pbrData.roughnessFactor;
			//outMaterial.ior = material.ior;
			//outMaterial.dispersion = material.dispersion;
			//outMaterial.doubleSided = material.doubleSided;
			//outMaterial.unlit = material.unlit;
			outMaterial.emissiveStrength = material.emissiveStrength;

			outMaterial.baseColorFactor = glm::make_vec4(material.pbrData.baseColorFactor.data());
			outMaterial.emissiveFactor = glm::make_vec4(material.emissiveFactor.data());

			if (material.pbrData.baseColorTexture.has_value()) {
				outMaterial.baseColorTextureIndex = material.pbrData.baseColorTexture.value().textureIndex + texturesOffset;
				//outMaterial.baseColorTextureUV = material.pbrData.baseColorTexture.value().texCoordIndex;
			}

			BRISK_CORE_INFO("Base texture index: {}", outMaterial.baseColorTextureIndex);

			if (material.pbrData.metallicRoughnessTexture.has_value()) {
				outMaterial.metallicRoughnessTextureIndex = material.pbrData.metallicRoughnessTexture.value().textureIndex + texturesOffset;
				//outMaterial.metallicRoughnessTextureUV = material.pbrData.metallicRoughnessTexture.value().texCoordIndex;
			}

			if (material.normalTexture.has_value()) {
				outMaterial.normalTextureIndex = material.normalTexture.value().textureIndex + texturesOffset;
				//outMaterial.normalTextureUV = material.normalTexture.value().texCoordIndex;
			}

			if (material.occlusionTexture.has_value()) {
				outMaterial.occlusionTextureIndex = material.occlusionTexture.value().textureIndex + texturesOffset;
				//outMaterial.occlusionTextureUV = material.occlusionTexture.value().texCoordIndex;
			}

			if (material.emissiveTexture.has_value()) {
				outMaterial.emissiveTextureIndex = material.emissiveTexture.value().textureIndex + texturesOffset;
				//outMaterial.emissiveTextureUV = material.emissiveTexture.value().texCoordIndex;
			}

			//if (material.anisotropy) {
			//	outMaterial.anisotropyStrength = material.anisotropy->anisotropyStrength;
			//	outMaterial.anisotropyRotation = material.anisotropy->anisotropyRotation;
			//	if (material.anisotropy->anisotropyTexture.has_value()) {
			//		outMaterial.anisotropyTextureIndex = material.anisotropy->anisotropyTexture.value().textureIndex + texturesOffset;
			//		outMaterial.anisotropyTextureUV = material.anisotropy->anisotropyTexture.value().texCoordIndex;
			//	}
			//}

			//if (material.clearcoat) {
			//	outMaterial.clearcoatFactor = material.clearcoat->clearcoatFactor;
			//	if (material.clearcoat->clearcoatTexture.has_value()) {
			//		outMaterial.clearcoatTextureIndex = material.clearcoat->clearcoatTexture.value().textureIndex + texturesOffset;
			//		outMaterial.clearcoatTextureUV = material.clearcoat->clearcoatTexture.value().texCoordIndex;
			//	}
			//	outMaterial.clearcoatRoughnessFactor = material.clearcoat->clearcoatRoughnessFactor;
			//	if (material.clearcoat->clearcoatRoughnessTexture.has_value()) {
			//		outMaterial.clearcoatRoughnessTextureIndex = material.clearcoat->clearcoatRoughnessTexture.value().textureIndex + texturesOffset;
			//		outMaterial.clearcoatRoughnessTextureUV = material.clearcoat->clearcoatRoughnessTexture.value().texCoordIndex;
			//	}
			//	if (material.clearcoat->clearcoatNormalTexture.has_value()) {
			//		outMaterial.clearcoatNormalTextureIndex = material.clearcoat->clearcoatNormalTexture.value().textureIndex + texturesOffset;
			//		outMaterial.clearcoatNormalTextureUV = material.clearcoat->clearcoatNormalTexture.value().texCoordIndex;
			//	}
			//}

			//if (material.iridescence) {
			//	outMaterial.iridescenceFactor = material.iridescence->iridescenceFactor;
			//	if (material.iridescence->iridescenceTexture.has_value()) {
			//		outMaterial.iridescenceTextureIndex = material.iridescence->iridescenceTexture.value().textureIndex + texturesOffset;
			//		outMaterial.iridescenceTextureUV = material.iridescence->iridescenceTexture.value().texCoordIndex;
			//	}
			//	outMaterial.iridescenceIor = material.iridescence->iridescenceIor;
			//	outMaterial.iridescenceThicknessMinimum = material.iridescence->iridescenceThicknessMinimum;
			//	outMaterial.iridescenceThicknessMaximum = material.iridescence->iridescenceThicknessMaximum;
			//	if (material.iridescence->iridescenceThicknessTexture.has_value()) {
			//		outMaterial.iridescenceThicknessTextureIndex = material.iridescence->iridescenceThicknessTexture.value().textureIndex + texturesOffset;
			//		outMaterial.iridescenceThicknessTextureUV = material.iridescence->iridescenceThicknessTexture.value().texCoordIndex;
			//	}
			//}

			//if (material.sheen) {
			//	outMaterial.sheenColorFactor = glm::make_vec3(material.sheen->sheenColorFactor.data());
			//	if (material.sheen->sheenColorTexture.has_value()) {
			//		outMaterial.sheenColorTextureIndex = material.sheen->sheenColorTexture.value().textureIndex + texturesOffset;
			//		outMaterial.sheenColorTextureUV = material.sheen->sheenColorTexture.value().texCoordIndex;
			//	}
			//	outMaterial.sheenRoughnessFactor = material.sheen->sheenRoughnessFactor;
			//	if (material.sheen->sheenRoughnessTexture.has_value()) {
			//		outMaterial.sheenRoughnessTextureIndex = material.sheen->sheenRoughnessTexture.value().textureIndex + texturesOffset;
			//		outMaterial.sheenRoughnessTextureUV = material.sheen->sheenRoughnessTexture.value().texCoordIndex;
			//	}
			//}

			//if (material.specular) {
			//	outMaterial.specularFactor = material.specular->specularFactor;
			//	if (material.specular->specularTexture.has_value()) {
			//		outMaterial.specularTextureIndex = material.specular->specularTexture.value().textureIndex + texturesOffset;
			//		outMaterial.specularTextureUV = material.specular->specularTexture.value().texCoordIndex;
			//	}
			//	outMaterial.specularColorFactor = glm::make_vec3(material.specular->specularColorFactor.data());
			//	if (material.specular->specularColorTexture.has_value()) {
			//		outMaterial.specularColorTextureIndex = material.specular->specularColorTexture.value().textureIndex + texturesOffset;
			//		outMaterial.specularColorTextureUV = material.specular->specularColorTexture.value().texCoordIndex;
			//	}
			//}

			//if (material.transmission) {
			//	outMaterial.transmissionFactor = material.transmission->transmissionFactor;
			//	if (material.transmission->transmissionTexture.has_value()) {
			//		outMaterial.transmissionTextureIndex = material.transmission->transmissionTexture.value().textureIndex + texturesOffset;
			//		outMaterial.transmissionTextureUV = material.transmission->transmissionTexture.value().texCoordIndex;
			//	}
			//}

			//if (material.volume) {
			//	outMaterial.thicknessFactor = material.volume->thicknessFactor;
			//	outMaterial.thicknessTextureIndex = material.volume->thicknessTexture.value().textureIndex + texturesOffset;
			//	outMaterial.thicknessTextureUV = material.volume->thicknessTexture.value().texCoordIndex;
			//	outMaterial.attenuationDistance = material.volume->attenuationDistance;
			//	outMaterial.attenuationColor = glm::make_vec3(material.volume->attenuationColor.data());
			//}

			m_Materials.push_back(outMaterial);

		}

		Application::GetRenderer()->m_MaterialStorageBuffer->RecordUpload(Application::GetRenderer()->m_TransferCmdBuffer, sizeof(m_Materials[0]) * m_Materials.size(), m_Materials.data());
		Application::GetRenderer()->m_TransferCmdBuffer->UnBind();

		for (const auto& tex : asset.textures) {
			const fastgltf::Image& image = asset.images[tex.imageIndex.value()];

			std::shared_ptr<Texture> texture = Texture::Create();
			texture->Init(image, asset);
			m_Textures.push_back(texture);

		}

		Engine::s_TexturesOffset += m_Textures.size();
		Application::GetRenderer()->AddGlobalTexture(m_Textures);
	}

	// Copy Component functions
	//-----------------------------------------------------------------------------------------------------------------------------------
	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}
	//-----------------------------------------------------------------------------------------------------------------------------------

	// Create Entity functions
	//-----------------------------------------------------------------------------------------------------------------------------------

	Entity Scene::CreateCubeEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreatePlaneEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateLightEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<PointLightComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateCameraEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<CameraComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}
	//-----------------------------------------------------------------------------------------------------------------------------------

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize our non-FixedAspectRatio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			//if (!cameraComponent.FixedAspectRatio)
				//cameraComponent.Camera->SetViewportSize(width, height);
		}
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const TagComponent& tc = view.get<TagComponent>(entity);
			if (tc.Tag == name)
				return Entity{ entity, this };
		}
		return {};
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		// Copy name because we're going to modify component data structure
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);
		CopyComponentIfExists(AllComponents{}, newEntity, entity);
		return newEntity;
	}

	// OnComponentAdded functions
	//-----------------------------------------------------------------------------------------------------------------------------------

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SkyboxComponent>(Entity entity, SkyboxComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SphereColliderComponent>(Entity entity, SphereColliderComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CapsuleColliderComponent>(Entity entity, CapsuleColliderComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MaterialComponent>(Entity entity, MaterialComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<PhysicsComponent>(Entity entity, PhysicsComponent& component)
	{
	}
	//-----------------------------------------------------------------------------------------------------------------------------------
}
