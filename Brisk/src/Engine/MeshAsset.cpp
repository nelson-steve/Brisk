// INCLUDES
#include "MeshAsset.hpp"
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
			uint32_t indexStart = indexPos;
			uint32_t vertexStart = vertexPos;
			uint32_t indexCount = 0;
			uint32_t vertexCount = 0;
			for (auto it = gltfMesh.primitives.begin(); it != gltfMesh.primitives.end(); ++it) {
				Primitive outPrimitive{};
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

					if (hasTexcoords1 && i < texcoords1.size())
						data.UV1 = glm::vec2(texcoords1[i].x(), texcoords1[i].y());
					else
						data.UV1 = glm::vec2(0.0f);

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
						for (uint16_t i : indices) {
							indicesData.push_back(static_cast<uint32_t>(i));
							indexPos++;
						}
						break;
					}
					case fastgltf::ComponentType::UnsignedShort: {
						std::vector<uint16_t> indices(indexAccessor.count);
						fastgltf::copyFromAccessor<uint16_t>(asset, indexAccessor, indices.data());
						for (uint16_t i : indices) {
							indicesData.push_back(static_cast<uint32_t>(i));
							indexPos++;
						}
						break;
					}
					case fastgltf::ComponentType::UnsignedInt: {
						std::vector<uint32_t> indices(indexAccessor.count);
						fastgltf::copyFromAccessor<uint32_t>(asset, indexAccessor, indices.data());
						for (uint16_t i : indices) {
							indicesData.push_back(static_cast<uint32_t>(i));
							indexPos++;
						}
						break;
					}
					default:
						throw std::runtime_error("Unsupported index component type.");
					}
					outPrimitive.has_indices = true;
				}

				outPrimitive.first_index = indexStart;
				outPrimitive.index_count = indexStart;
				outPrimitive.vertex_count = indexStart;

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
	}

	//void MeshAsset::LoadMaterials(tinygltf::Model model) {
	//	for (tinygltf::Material& mat : model.materials) {
	//		MaterialData material{};
	//		material.doubleSided = mat.doubleSided;
	//		if (mat.values.find("baseColorTexture") != mat.values.end()) {
	//			material.baseColorTexture = m_textures[mat.values["baseColorTexture"].TextureIndex()];
	//			material.texCoordSets.baseColor = mat.values["baseColorTexture"].TextureTexCoord();
	//		}
	//		if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
	//			material.metallicRoughnessTexture = m_textures[mat.values["metallicRoughnessTexture"].TextureIndex()];
	//			material.texCoordSets.metallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
	//		}
	//		if (mat.values.find("roughnessFactor") != mat.values.end()) {
	//			material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
	//		}
	//		if (mat.values.find("metallicFactor") != mat.values.end()) {
	//			material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
	//		}
	//		if (mat.values.find("baseColorFactor") != mat.values.end()) {
	//			material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
	//		}
	//		if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
	//			material.normalTexture = m_textures[mat.additionalValues["normalTexture"].TextureIndex()];
	//			material.texCoordSets.normal = mat.additionalValues["normalTexture"].TextureTexCoord();
	//		}
	//		if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
	//			material.emissiveTexture = m_textures[mat.additionalValues["emissiveTexture"].TextureIndex()];
	//			material.texCoordSets.emissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
	//		}
	//		if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
	//			material.occlusionTexture = m_textures[mat.additionalValues["occlusionTexture"].TextureIndex()];
	//			material.texCoordSets.occlusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
	//		}
	//		if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
	//			tinygltf::Parameter param = mat.additionalValues["alphaMode"];
	//			if (param.string_value == "BLEND") {
	//				material.alphaMode = MaterialData::ALPHAMODE_BLEND;
	//			}
	//			if (param.string_value == "MASK") {
	//				material.alphaCutoff = 0.5f;
	//				material.alphaMode = MaterialData::ALPHAMODE_MASK;
	//			}
	//		}
	//		if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
	//			material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
	//		}
	//		if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
	//			material.emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
	//		}

	//		// Extensions
	//		// @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
	//		if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) {
	//			auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
	//			if (ext->second.Has("specularGlossinessTexture")) {
	//				auto index = ext->second.Get("specularGlossinessTexture").Get("index");
	//				material.extension.specularGlossinessTexture = m_textures[index.Get<int>()];
	//				auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
	//				material.texCoordSets.specularGlossiness = texCoordSet.Get<int>();
	//				material.pbrWorkflows.specularGlossiness = true;
	//			}
	//			if (ext->second.Has("diffuseTexture")) {
	//				auto index = ext->second.Get("diffuseTexture").Get("index");
	//				material.extension.diffuseTexture = m_textures[index.Get<int>()];
	//			}
	//			if (ext->second.Has("diffuseFactor")) {
	//				auto factor = ext->second.Get("diffuseFactor");
	//				for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
	//					auto val = factor.Get(i);
	//					material.extension.diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
	//				}
	//			}
	//			if (ext->second.Has("specularFactor")) {
	//				auto factor = ext->second.Get("specularFactor");
	//				for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
	//					auto val = factor.Get(i);
	//					material.extension.specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
	//				}
	//			}
	//		}

	//		if (mat.extensions.find("KHR_materials_unlit") != mat.extensions.end()) {
	//			material.unlit = true;
	//		}

	//		if (mat.extensions.find("KHR_materials_emissive_strength") != mat.extensions.end()) {
	//			auto ext = mat.extensions.find("KHR_materials_emissive_strength");
	//			if (ext->second.Has("emissiveStrength")) {
	//				auto value = ext->second.Get("emissiveStrength");
	//				material.emissiveStrength = (float)value.Get<double>();
	//			}
	//		}
	//			
	//		material.index = static_cast<uint32_t>(m_materials.size());
	//		m_materials.push_back(material);
	//	}
	//	// Push a default material at the end of the list for meshes with no material assigned
	//	m_materials.push_back(MaterialData());
	//}
}