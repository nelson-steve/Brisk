// INCLUDES
#include "MeshAsset.hpp"
#include <stb_image.h>
#include <fastgltf/util.hpp>
#include <fastgltf/math.hpp>
#include <fastgltf/tools.hpp>
#include <Graphics/Vulkan/TextureVulkan.hpp>
//-------------------

namespace Brisk {
	MeshAsset::~MeshAsset() {
	}

	void MeshAsset::LoadNodes(Node* parent, uint32_t nodeIndex, const fastgltf::Asset& asset) {
		const fastgltf::Node& gltfNode = asset.nodes[nodeIndex];

		Node* newNode = new Node();
		newNode->parent = parent;
		newNode->name = gltfNode.name;

		// Load transform
		newNode->matrix = glm::make_mat4(fastgltf::getTransformMatrix(gltfNode).data());

		if (gltfNode.meshIndex.has_value()) {
			newNode->meshIndex = static_cast<uint32_t>(*gltfNode.meshIndex);
		}
		else {
			newNode->meshIndex = UINT32_MAX;
		}

		if (gltfNode.children.size() > 0) {
			for (const auto& childIndex : gltfNode.children) {
				LoadNodes(newNode, static_cast<uint32_t>(childIndex), asset);
			}
		}

		if (parent) {
			parent->children.push_back(newNode);
		}

		m_Nodes.push_back(newNode);
	}

	void MeshAsset::Load(const std::filesystem::path& path) {
		if (!std::filesystem::exists(path)) {
			std::cout << "Failed to find " << path << '\n';
		}

		if constexpr (std::is_same_v<std::filesystem::path::value_type, wchar_t>) {
			std::cout << "Loading " << path << '\n';
		}
		else {
			std::cout << "Loading " << path << '\n';
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
				fastgltf::Options::GenerateMeshIndices;

			auto gltfFile = fastgltf::MappedGltfFile::FromPath(path);
			if (!bool(gltfFile)) {
				std::cerr << "Failed to open glTF file: " << fastgltf::getErrorMessage(gltfFile.error()) << '\n';
			}

			auto a = parser.loadGltf(gltfFile.get(), path.parent_path(), gltfOptions);
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
		for (const auto& node : scene.nodeIndices) {
			LoadNodes(nullptr, node, asset);
		}

		uint32_t totalVertexCount = 0;
		uint32_t totalIndexCount = 0;
		for (const auto& gltfMesh : asset.meshes) {
			for (auto it = gltfMesh.primitives.begin(); it != gltfMesh.primitives.end(); ++it) {
				Primitive outPrimitive{};
				const fastgltf::Attribute* positionIt = it->findAttribute("POSITION");

				assert(positionIt != it->attributes.end());
				assert(it->indicesAccessor.has_value());

				auto& positionAccessor = asset.accessors[positionIt->accessorIndex];
				if (!positionAccessor.bufferViewIndex.has_value())
					continue;

				// Load positions
				if (positionAccessor.componentType == fastgltf::ComponentType::Float &&
					positionAccessor.type == fastgltf::AccessorType::Vec3) {
					totalVertexCount += positionAccessor.count;
				}

				if (it->indicesAccessor.has_value()) {
					const auto& indexAccessor = asset.accessors[it->indicesAccessor.value()];
					totalIndexCount += indexAccessor.count;
				}
			}
		}

		uint32_t indexPos = 0;
		uint32_t vertexPos = 0;
		std::vector<MeshData> verticesData;
		std::vector<uint32_t> indicesData;
		verticesData.reserve(totalVertexCount);
		indicesData.reserve(totalIndexCount);
		for (const auto& gltfMesh : asset.meshes) {
			Mesh outMesh{};
			for (auto it = gltfMesh.primitives.begin(); it != gltfMesh.primitives.end(); ++it) {
				Primitive outPrimitive{};
				uint32_t indexStart = indexPos;
				uint32_t vertexStart = vertexPos;
				uint32_t indexCount = 0;
				uint32_t vertexCount = 0;

				const fastgltf::Attribute* positionIt = it->findAttribute("POSITION");
				const fastgltf::Attribute* normalIt = it->findAttribute("NORMAL");
				const fastgltf::Attribute* tangentIt = it->findAttribute("TANGENT"); // The W component of each TANGENT accessor element MUST be set to 1.0 or -1.0
				const fastgltf::Attribute* texCoord0It = it->findAttribute("TEXCOORD_0");
				const fastgltf::Attribute* texCoord1It = it->findAttribute("TEXCOORD_1");
				const fastgltf::Attribute* colorIt = it->findAttribute("COLOR_0");
				const fastgltf::Attribute* jointsIt = it->findAttribute("JOINTS_0");
				const fastgltf::Attribute* weightsIt = it->findAttribute("WEIGHTS_0");

				assert(positionIt != it->attributes.end());
				assert(it->indicesAccessor.has_value());

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

				// Combine into vertex struct
				for (size_t i = 0; i < positions.size(); ++i) {
					MeshData data{};
					data.Position = glm::vec3(positions[i].x(), positions[i].y(), positions[i].z());

					if (hasNormals && i < normals.size())
						data.Normal = glm::vec3(normals[i].x(), normals[i].y(), normals[i].z());
					else
						data.Normal = glm::vec3(0.0f);

					if (hasTexcoords0 && i < texcoords0.size())
						data.UV0 = glm::vec2(texcoords0[i].x(), texcoords0[i].y());
					else
						data.UV0 = glm::vec2(0.0f);

					//if (hasTexcoords1 && i < texcoords1.size())
					//	data.UV1 = glm::vec2(texcoords1[i].x(), texcoords1[i].y());
					//else
					//	data.UV1 = glm::vec2(0.0f);

					vertexCount++;
					vertexPos++;
					verticesData.push_back(data);
				}

				size_t primitiveIndex = std::distance(gltfMesh.primitives.begin(), it);
				const auto& primitive = *it;

				//// Load indices
				if (it->indicesAccessor.has_value()) {
					const auto& indexAccessor = asset.accessors[it->indicesAccessor.value()];
					indexCount = indexAccessor.count;

					switch (indexAccessor.componentType) {
					case fastgltf::ComponentType::UnsignedByte: {
						std::vector<uint8_t> indices(indexAccessor.count);
						fastgltf::copyFromAccessor<uint8_t>(asset, indexAccessor, indices.data());
						for (uint8_t i : indices) {
							indicesData.push_back(static_cast<uint32_t>(i) + vertexStart);
							indexPos++;
						}
						break;
					}
					case fastgltf::ComponentType::UnsignedShort: {
						std::vector<uint16_t> indices(indexAccessor.count);
						fastgltf::copyFromAccessor<uint16_t>(asset, indexAccessor, indices.data());
						for (uint16_t i : indices) {
							indicesData.push_back(static_cast<uint32_t>(i) + vertexStart);
							indexPos++;
						}
						break;
					}
					case fastgltf::ComponentType::UnsignedInt: {
						std::vector<uint32_t> indices(indexAccessor.count);
						fastgltf::copyFromAccessor<uint32_t>(asset, indexAccessor, indices.data());
						for (uint32_t i : indices) {
							indicesData.push_back(static_cast<uint32_t>(i) + vertexStart);
							indexPos++;
						}
						break;
					}
					default:
						throw std::runtime_error("Unsupported index component type.");
					}
					outPrimitive.has_indices = true;
				}

				outPrimitive.materialIndex = it->materialIndex.has_value() ? it->materialIndex.value() : -1;
				outPrimitive.firstIndex = indexStart;
				outPrimitive.indexCount = indexCount;
				outPrimitive.vertexCount = vertexCount;

				outMesh.primitives.push_back(outPrimitive);
			}
			m_Meshes.push_back(outMesh);
		}


		m_VertexBuffer = Buffer::Create();
		m_VertexBuffer->Init(sizeof(verticesData[0]) * verticesData.size(),
			verticesData.data(),
			Core::BufferUsage::VertexBuffer | Core::BufferUsage::TransferDst,
			Core::MemoryProperty::DeviceLocal,
			true);

		if (indicesData.size() > 0) {
			m_IndexBuffer = Buffer::Create();
			m_IndexBuffer->Init(sizeof(indicesData[0]) * indicesData.size(),
				indicesData.data(),
				Core::BufferUsage::IndexBuffer | Core::BufferUsage::TransferDst,
				Core::MemoryProperty::DeviceLocal,
				true);
		}

		m_Materials.reserve(asset.materials.size());
		for (const auto& material : asset.materials) {
			MaterialData outMaterial{};
			outMaterial.alphaCutoff = material.alphaCutoff;
			outMaterial.metallicFactor = material.pbrData.metallicFactor;
			outMaterial.roughnessFactor = material.pbrData.roughnessFactor;
			outMaterial.ior = material.ior;
			outMaterial.dispersion = material.dispersion;
			outMaterial.doubleSided = material.doubleSided;
			outMaterial.unlit = material.unlit;
			outMaterial.emissiveStrength = material.emissiveStrength;

			outMaterial.baseColorFactor = glm::make_vec4(material.pbrData.baseColorFactor.data());
			outMaterial.emissiveFactor = glm::make_vec4(material.emissiveFactor.data());

			if (material.pbrData.baseColorTexture.has_value()) {
				outMaterial.baseColorTextureIndex = material.pbrData.baseColorTexture.value().texCoordIndex;
				outMaterial.baseColorTextureUV = material.pbrData.baseColorTexture.value().texCoordIndex;

				if (material.pbrData.metallicRoughnessTexture.has_value()) {
					outMaterial.metallicRoughnessTextureIndex = material.pbrData.metallicRoughnessTexture.value().texCoordIndex;
					outMaterial.metallicRoughnessTextureUV = material.pbrData.metallicRoughnessTexture.value().texCoordIndex;
				}

				if (material.normalTexture.has_value()) {
					outMaterial.normalTextureIndex = material.normalTexture.value().texCoordIndex;
					outMaterial.normalTextureUV = material.normalTexture.value().texCoordIndex;
				}

				if (material.occlusionTexture.has_value()) {
					outMaterial.occlusionTextureIndex = material.occlusionTexture.value().texCoordIndex;
					outMaterial.occlusionTextureUV = material.occlusionTexture.value().texCoordIndex;
				}

				if (material.emissiveTexture.has_value()) {
					outMaterial.emissiveTextureIndex = material.emissiveTexture.value().texCoordIndex;
					outMaterial.emissiveTextureUV = material.emissiveTexture.value().texCoordIndex;
				}

				if (material.anisotropy) {
					outMaterial.anisotropyStrength = material.anisotropy->anisotropyStrength;
					outMaterial.anisotropyRotation = material.anisotropy->anisotropyRotation;
					if (material.anisotropy->anisotropyTexture.has_value()) {
						outMaterial.anisotropyTextureIndex = material.anisotropy->anisotropyTexture.value().texCoordIndex;
						outMaterial.anisotropyTextureUV = material.anisotropy->anisotropyTexture.value().texCoordIndex;
					}
				}

				if (material.clearcoat) {
					outMaterial.clearcoatFactor = material.clearcoat->clearcoatFactor;
					if (material.clearcoat->clearcoatTexture.has_value()) {
						outMaterial.clearcoatTextureIndex = material.clearcoat->clearcoatTexture.value().textureIndex;
						outMaterial.clearcoatTextureUV = material.clearcoat->clearcoatTexture.value().texCoordIndex;
					}
					outMaterial.clearcoatRoughnessFactor = material.clearcoat->clearcoatRoughnessFactor;
					if (material.clearcoat->clearcoatRoughnessTexture.has_value()) {
						outMaterial.clearcoatRoughnessTextureIndex = material.clearcoat->clearcoatRoughnessTexture.value().texCoordIndex;
						outMaterial.clearcoatRoughnessTextureUV = material.clearcoat->clearcoatRoughnessTexture.value().texCoordIndex;
					}
					if (material.clearcoat->clearcoatNormalTexture.has_value()) {
						outMaterial.clearcoatNormalTextureIndex = material.clearcoat->clearcoatNormalTexture.value().texCoordIndex;
						outMaterial.clearcoatNormalTextureUV = material.clearcoat->clearcoatNormalTexture.value().texCoordIndex;
					}
				}

				if (material.iridescence) {
					outMaterial.iridescenceFactor = material.iridescence->iridescenceFactor;
					if (material.iridescence->iridescenceTexture.has_value()) {
						outMaterial.iridescenceTextureIndex = material.iridescence->iridescenceTexture.value().textureIndex;
						outMaterial.iridescenceTextureUV = material.iridescence->iridescenceTexture.value().texCoordIndex;
					}
					outMaterial.iridescenceIor = material.iridescence->iridescenceIor;
					outMaterial.iridescenceThicknessMinimum = material.iridescence->iridescenceThicknessMinimum;
					outMaterial.iridescenceThicknessMaximum = material.iridescence->iridescenceThicknessMaximum;
					if (material.iridescence->iridescenceThicknessTexture.has_value()) {
						outMaterial.iridescenceThicknessTextureIndex = material.iridescence->iridescenceThicknessTexture.value().textureIndex;
						outMaterial.iridescenceThicknessTextureUV = material.iridescence->iridescenceThicknessTexture.value().texCoordIndex;
					}
				}

				if (material.sheen) {
					outMaterial.sheenColorFactor = glm::make_vec3(material.sheen->sheenColorFactor.data());
					if (material.sheen->sheenColorTexture.has_value()) {
						outMaterial.sheenColorTextureIndex = material.sheen->sheenColorTexture.value().textureIndex;
						outMaterial.sheenColorTextureUV = material.sheen->sheenColorTexture.value().texCoordIndex;
					}
					outMaterial.sheenRoughnessFactor = material.sheen->sheenRoughnessFactor;
					if (material.sheen->sheenRoughnessTexture.has_value()) {
						outMaterial.sheenRoughnessTextureIndex = material.sheen->sheenRoughnessTexture.value().textureIndex;
						outMaterial.sheenRoughnessTextureUV = material.sheen->sheenRoughnessTexture.value().texCoordIndex;
					}
				}

				if (material.specular) {
					outMaterial.specularFactor = material.specular->specularFactor;
					if (material.specular->specularTexture.has_value()) {
						outMaterial.specularTextureIndex = material.specular->specularTexture.value().textureIndex;
						outMaterial.specularTextureUV = material.specular->specularTexture.value().texCoordIndex;
					}
					outMaterial.specularColorFactor = glm::make_vec3(material.specular->specularColorFactor.data());
					if (material.specular->specularColorTexture.has_value()) {
						outMaterial.specularColorTextureIndex = material.specular->specularColorTexture.value().textureIndex;
						outMaterial.specularColorTextureUV = material.specular->specularColorTexture.value().texCoordIndex;
					}
				}

				if (material.transmission) {
					outMaterial.transmissionFactor = material.transmission->transmissionFactor;
					if (material.transmission->transmissionTexture.has_value()) {
						outMaterial.transmissionTextureIndex = material.transmission->transmissionTexture.value().textureIndex;
						outMaterial.transmissionTextureUV = material.transmission->transmissionTexture.value().texCoordIndex;
					}
				}

				if (material.volume) {
					outMaterial.thicknessFactor = material.volume->thicknessFactor;
					outMaterial.thicknessTextureIndex = material.volume->thicknessTexture.value().textureIndex;
					outMaterial.thicknessTextureUV = material.volume->thicknessTexture.value().texCoordIndex;
					outMaterial.attenuationDistance = material.volume->attenuationDistance;
					outMaterial.attenuationColor = glm::make_vec3(material.volume->attenuationColor.data());
				}

				m_Materials.push_back(outMaterial);
			}

		}

		std::shared_ptr<Buffer> materialStorageBuffer = Buffer::Create();
		materialStorageBuffer->Init(sizeof(MaterialData)* m_Materials.size(), m_Materials.data(), Core::BufferUsage::StorageBuffer, Core::MemoryProperty::DeviceLocal, true);
		
		Engine::s_Application->GetGpuAdapter()->AddResource(GpuDescriptorResourceType::Materials, nullptr, materialStorageBuffer, 0);

		int i = 0;
		for (const auto& tex : asset.textures) {
			const fastgltf::Image& image = asset.images[tex.imageIndex.value()];

			std::shared_ptr<Texture> texture = Texture::Create();
			std::static_pointer_cast<TextureVulkan>(texture)->Init(image, asset);

			Engine::s_Application->GetGpuAdapter()->AddResource(GpuDescriptorResourceType::BindlessTextures, texture, nullptr, i);
			i++;
		}
	}
}