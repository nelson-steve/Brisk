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

		uint32_t indexPos = 0;
		uint32_t vertexPos = 0;
		std::vector<MeshData> verticesData;
		std::vector<uint32_t> indicesData;
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
	}

	//void MeshAsset::Load(const std::string& path) {
		//tinygltf::TinyGLTF loader;
		//tinygltf::Model model;
		//std::string error;
		//std::string warning;

		//bool binary = false;
		//size_t extpos = path.rfind('.', path.length());
		//if (extpos != std::string::npos) {
		//	binary = (path.substr(extpos + 1, path.length() - extpos) == "glb");
		//}

		//bool file_loaded = false;
		//if (binary) {
		//	file_loaded = loader.LoadBinaryFromFile(&model, &error, &warning, path.c_str());
		//}
		//else {
		//	file_loaded = loader.LoadASCIIFromFile(&model, &error, &warning, path.c_str());
		//}

		//size_t vertex_count = 0;
		//size_t index_count = 0;
		//if (file_loaded) {
			//for (tinygltf::Sampler smpl : model.samplers) {
			//	TextureSampler texture_sampler{};
			//	texture_sampler.min_filter = Texture::GetVkFilterMode(smpl.minFilter);
			//	texture_sampler.mag_filter = Texture::GetVkFilterMode(smpl.magFilter);
			//	texture_sampler.address_modeU = Texture::GetVkWrapMode(smpl.wrapS);
			//	texture_sampler.address_modeV = Texture::GetVkWrapMode(smpl.wrapT);
			//	texture_sampler.address_modeW = texture_sampler.address_modeV;
			//	m_texture_samplers.push_back(texture_sampler);
			//}
			//for (tinygltf::Texture& tex : model.textures) {
			//	tinygltf::Image image = model.images[tex.source];
			//	TextureSampler texture_sampler{};
			//	if (tex.sampler == -1)
			//	{
			//		// No sampler specified, use a default one
			//		texture_sampler.min_filter = FILTER_LINEAR;
			//		texture_sampler.mag_filter = FILTER_LINEAR;
			//		texture_sampler.address_modeU = SAMPLER_ADDRESS_MODE_REPEAT;
			//		texture_sampler.address_modeV = SAMPLER_ADDRESS_MODE_REPEAT;
			//		texture_sampler.address_modeW = SAMPLER_ADDRESS_MODE_REPEAT;
			//	}
			//	else {
			//		texture_sampler = m_texture_samplers[tex.sampler];
			//	}

			//	std::shared_ptr<Texture> texture;
			//	texture = Texture::Create();
			//	texture->Init(image, texture_sampler);
			//	m_textures.push_back(texture);
			//}
			//Load Materials
			//LoadMaterials(model);

			//const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
			//for (auto& node_index : scene.nodes) {
			//	GetNodeProps(model.nodes[node_index], model, vertex_count, index_count);
			//}
			//assert(vertex_count > 0);
			//m_vertex_buffer = new MeshData[vertex_count];
			//m_index_buffer = new uint32_t[index_count];

			//for (auto& node_index : scene.nodes) {
			//	const tinygltf::Node node = model.nodes[node_index];
			//	LoadNode(nullptr, node, node_index, model);
			//}
		//}

		//size_t vertexBufferSize = vertex_count * sizeof(MeshData);
		//size_t indexBufferSize = index_count * sizeof(uint32_t);
		//assert(vertexBufferSize > 0);

		//m_VertexBuffer = Buffer::Create();
		//m_VertexBuffer->Init(sizeof(m_vertex_buffer[0]) * vertex_count,
		//	m_vertex_buffer,
		//	Core::BufferUsage::VertexBuffer,
		//	Core::MemoryProperty::HostVisible | Core::MemoryProperty::HostCoherent,
		//	true);

		//if (indexBufferSize > 0) {
		//	m_IndexBuffer = Buffer::Create();
		//	m_IndexBuffer->Init(sizeof(m_index_buffer[0]) * index_count,
		//		m_index_buffer,
		//		Core::BufferUsage::IndexBuffer,
		//		Core::MemoryProperty::HostVisible | Core::MemoryProperty::HostCoherent,
		//		true);
		//}
	//}

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

	//void MeshAsset::GetNodeProps(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertex_count, size_t& index_count) {
	//	if (node.children.size() > 0) {
	//		for (size_t i = 0; i < node.children.size(); i++) {
	//			GetNodeProps(model.nodes[node.children[i]], model, vertex_count, index_count);
	//		}
	//	}
	//	if (node.mesh > -1) {
	//		const tinygltf::Mesh mesh = model.meshes[node.mesh];
	//		for (size_t i = 0; i < mesh.primitives.size(); i++) {
	//			auto primitive = mesh.primitives[i];
	//			vertex_count += model.accessors[primitive.attributes.find("POSITION")->second].count;
	//			if (primitive.indices > -1) {
	//				index_count += model.accessors[primitive.indices].count;
	//			}
	//		}
	//	}
	//}

	//void MeshAsset::LoadNode(GLTF_Node* parent, const tinygltf::Node& node, uint32_t node_index, const tinygltf::Model& model) {
	//	GLTF_Node* new_node = new GLTF_Node();
	//	new_node->parent = parent;
	//	new_node->index = node_index;
	//	new_node->name = node.name;
	//	new_node->matrix = glm::mat4(1.0f);

	//	if (node.translation.size() == 3) {
	//		new_node->translation = glm::make_vec3(node.translation.data());
	//	}
	//	if (node.rotation.size() == 4) {
	//		new_node->rotation = glm::make_quat(node.rotation.data());
	//	}
	//	if (node.scale.size() == 4) {
	//		new_node->scale = glm::make_vec3(node.scale.data());
	//	}
	//	if (node.matrix.size() == 16) {
	//		new_node->matrix = glm::make_mat4x4(node.matrix.data());
	//	}

	//	if (node.children.size() > 0) {
	//		for (auto& node_index : node.children)
	//			LoadNode(new_node, model.nodes[node_index], node_index, model);
	//	}

	//	if (node.mesh > -1) {
	//		const tinygltf::Mesh mesh = model.meshes[node.mesh];
	//		GLTF_Mesh* new_mesh = new GLTF_Mesh(new_node->matrix);
	//		for (auto& primitive : mesh.primitives) {
	//			uint32_t vertex_start = m_vertex_pos;
	//			uint32_t index_start = m_index_pos;
	//			uint32_t vertex_count = 0;
	//			uint32_t index_count = 0;
	//			// Vertices
	//			{
	//				const float* buffer_pos = nullptr;
	//				const float* buffer_normals = nullptr;
	//				const float* buffer_uv_set0 = nullptr;
	//				const float* buffer_uv_set1 = nullptr;
	//				const float* buffer_color_set0 = nullptr;
	//				const void* buffer_joints = nullptr;
	//				const float* buffer_weights = nullptr;

	//				int posByteStride = 0;
	//				int normByteStride = 0;
	//				int uv0ByteStride = 0;
	//				int uv1ByteStride = 0;
	//				int color0ByteStride = 0;
	//				//int jointByteStride;
	//				//int weightByteStride;

	//				if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
	//					const tinygltf::Accessor& pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
	//					const tinygltf::BufferView& pos_view = model.bufferViews[pos_accessor.bufferView];
	//					vertex_count = static_cast<uint32_t>(pos_accessor.count);
	//					buffer_pos = reinterpret_cast<const float*>(&(model.buffers[pos_view.buffer].data[pos_accessor.byteOffset + pos_view.byteOffset]));
	//					posByteStride = pos_accessor.ByteStride(pos_view) ? (pos_accessor.ByteStride(pos_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
	//				}
	//				else {
	//					assert(primitive.attributes.find("POSITION") != primitive.attributes.end());
	//				}

	//				if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
	//					const tinygltf::Accessor& normal_accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
	//					const tinygltf::BufferView& normal_view = model.bufferViews[normal_accessor.bufferView];
	//					buffer_normals = reinterpret_cast<const float*>(&(model.buffers[normal_view.buffer].data[normal_accessor.byteOffset + normal_view.byteOffset]));
	//					normByteStride = normal_accessor.ByteStride(normal_view) ? (normal_accessor.ByteStride(normal_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
	//				}

	//				if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
	//					const tinygltf::Accessor& uv0_accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
	//					const tinygltf::BufferView& uv0_view = model.bufferViews[uv0_accessor.bufferView];
	//					buffer_uv_set0 = reinterpret_cast<const float*>(&(model.buffers[uv0_view.buffer].data[uv0_accessor.byteOffset + uv0_view.byteOffset]));
	//					uv0ByteStride = uv0_accessor.ByteStride(uv0_view) ? (uv0_accessor.ByteStride(uv0_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
	//				}

	//				if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
	//					const tinygltf::Accessor& uv1_accessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
	//					const tinygltf::BufferView& uv1_view = model.bufferViews[uv1_accessor.bufferView];
	//					buffer_uv_set1 = reinterpret_cast<const float*>(&(model.buffers[uv1_view.buffer].data[uv1_accessor.byteOffset + uv1_view.byteOffset]));
	//					uv1ByteStride = uv1_accessor.ByteStride(uv1_view) ? (uv1_accessor.ByteStride(uv1_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
	//				}

	//				if (primitive.attributes.find("COLOR_0") != primitive.attributes.end()) {
	//					const tinygltf::Accessor& color0_accessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
	//					const tinygltf::BufferView& uv1_view = model.bufferViews[color0_accessor.bufferView];
	//					buffer_color_set0 = reinterpret_cast<const float*>(&(model.buffers[uv1_view.buffer].data[color0_accessor.byteOffset + uv1_view.byteOffset]));
	//					color0ByteStride = color0_accessor.ByteStride(uv1_view) ? (color0_accessor.ByteStride(uv1_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
	//				}

	//					const tinygltf::Accessor& pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
	//					for (size_t v = 0; v < pos_accessor.count; v++) {
	//						MeshData& vert = m_vertex_buffer[m_vertex_pos];
	//						vert.Position = glm::vec4( glm::make_vec3(&buffer_pos[v * posByteStride]), 1.0f);
	//						vert.Normal = glm::normalize(glm::vec3(buffer_normals ? glm::make_vec3(&buffer_normals[v * normByteStride]) : glm::vec3(0.0f)));
	//						vert.UV0 = buffer_uv_set0 ? glm::make_vec2(&buffer_uv_set0[v * uv0ByteStride]) : glm::vec2(0.0f);
	//						//vert.UV1 = buffer_uv_set1 ? glm::make_vec2(&buffer_uv_set1[v * uv1ByteStride]) : glm::vec2(0.0f);
	//						//vert.Color = buffer_color_set0 ? glm::make_vec3(&buffer_color_set0[v * color0ByteStride]) : glm::vec3(1.0f);

	//						m_vertex_pos++;
	//					}

	//			}
	//			bool has_indices = primitive.indices > -1;
	//			if (has_indices) {
	//				const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
	//				const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
	//				const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];

	//				index_count = static_cast<uint32_t>(accessor.count);
	//				const void* data_ptr = &(buffer.data[accessor.byteOffset + buffer_view.byteOffset]);

	//				switch (accessor.componentType) {
	//				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
	//					const uint32_t* buf = static_cast<const uint32_t*>(data_ptr);
	//					for (size_t index = 0; index < accessor.count; index++) {
	//						m_index_buffer[m_index_pos] = buf[index] + vertex_start;
	//						m_index_pos++;
	//					}
	//					break;
	//				}
	//				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
	//					const uint16_t* buf = static_cast<const uint16_t*>(data_ptr);
	//					for (size_t index = 0; index < accessor.count; index++) {
	//						m_index_buffer[m_index_pos] = buf[index] + vertex_start;
	//						m_index_pos++;
	//					}
	//					break;
	//				}
	//				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
	//					const uint8_t* buf = static_cast<const uint8_t*>(data_ptr);
	//					for (size_t index = 0; index < accessor.count; index++) {
	//						m_index_buffer[m_index_pos] = buf[index] + vertex_start;
	//						m_index_pos++;
	//					}
	//					break;
	//				}
	//				default:
	//					std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
	//					return;
	//				}
	//			}
	//			else {
	//				assert(false);
	//			}
	//			uint32_t mat_index = primitive.material > -1 ? primitive.material : -1;
	//			Primitive* new_primitive = new Primitive(index_start, index_count, vertex_count, mat_index);
	//			new_mesh->primitives.push_back(new_primitive);
	//		}
	//		new_node->mesh = new_mesh;
	//	}
	//	if (parent) {
	//		parent->children.push_back(new_node);
	//	}
	//	else {
	//		m_nodes.push_back(new_node);
	//	}
	//	m_linear_nodes.push_back(new_node);
	//}
}