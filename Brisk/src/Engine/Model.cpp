#include "Model.hpp"

#include "tiny_gltf.h"

namespace Brisk {
	Mesh::~Mesh() {
		for (auto node : m_nodes) {
			delete node;
		}
	}

	void Mesh::Load(const std::string& path) {
		tinygltf::TinyGLTF loader;
		tinygltf::Model model;
		std::string error;
		std::string warning;

		bool binary = false;
		size_t extpos = path.rfind('.', path.length());
		if (extpos != std::string::npos) {
			binary = (path.substr(extpos + 1, path.length() - extpos) == "glb");
		}

		bool file_loaded = false;
		if (binary) {
			file_loaded = loader.LoadBinaryFromFile(&model, &error, &warning, path.c_str());
		}
		else {
			file_loaded = loader.LoadASCIIFromFile(&model, &error, &warning, path.c_str());
		}

		uint32_t vertex_count = 0;
		uint32_t index_count = 0;
		if (file_loaded) {
			for (tinygltf::Sampler smpl : model.samplers) {
				TextureSampler texture_sampler{};
				texture_sampler.min_filter = Texture::GetVkFilterMode(smpl.minFilter);
				texture_sampler.mag_filter = Texture::GetVkFilterMode(smpl.magFilter);
				texture_sampler.address_modeU = Texture::GetVkWrapMode(smpl.wrapS);
				texture_sampler.address_modeV = Texture::GetVkWrapMode(smpl.wrapT);
				texture_sampler.address_modeW = texture_sampler.address_modeV;
				m_texture_samplers.push_back(texture_sampler);
			}
			for (tinygltf::Texture& tex : model.textures) {
				tinygltf::Image image = model.images[tex.source];
				TextureSampler texture_sampler{};
				if (tex.sampler == -1)
				{
					// No sampler specified, use a default one
					texture_sampler.min_filter = FILTER_LINEAR;
					texture_sampler.mag_filter = FILTER_LINEAR;
					texture_sampler.address_modeU = SAMPLER_ADDRESS_MODE_REPEAT;
					texture_sampler.address_modeV = SAMPLER_ADDRESS_MODE_REPEAT;
					texture_sampler.address_modeW = SAMPLER_ADDRESS_MODE_REPEAT;
				}
				else {
					texture_sampler = m_texture_samplers[tex.sampler];
				}

				std::shared_ptr<Texture> texture;
				texture = Texture::Create();
				texture->Init();
				m_textures.push_back(texture);
			}
			//Load Materials
			LoadMaterials(model);

			const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
			for (auto& node_index : scene.nodes) {
				GetNodeProps(model.nodes[node_index], model, vertex_count, index_count);
			}
			assert(vertex_count > 0);
			m_vertex_buffer = new MeshData[vertex_count];
			m_index_buffer = new uint32_t[index_count];

			for (auto& node_index : scene.nodes) {
				const tinygltf::Node node = model.nodes[node_index];
				LoadNode(nullptr, node, node_index, model);
			}
		}

		size_t vertexBufferSize = vertex_count * sizeof(MeshData);
		size_t indexBufferSize = index_count * sizeof(uint32_t);
		assert(vertexBufferSize > 0);

		m_VertexBuffer = Buffer::Create();
		m_VertexBuffer->Init(sizeof(m_vertex_buffer[0])* vertexBufferSize,
			m_vertex_buffer,
			{ Core::BufferUsage::BUFFER_USAGE_VERTEX_BUFFER_BIT },
			{ Core::MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT, Core::MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT },
			false);

		if (indexBufferSize > 0) {
			m_IndexBuffer = Buffer::Create();
			m_IndexBuffer->Init(sizeof(m_index_buffer[0]) * indexBufferSize,
				m_index_buffer,
				{ Core::BufferUsage::BUFFER_USAGE_INDEX_BUFFER_BIT },
				{ Core::MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT, Core::MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT },
				false);
		}
	}

	void Mesh::LoadMaterials(tinygltf::Model model) {
		for (tinygltf::Material& mat : model.materials) {
			MaterialData material{};
			material.doubleSided = mat.doubleSided;
			if (mat.values.find("baseColorTexture") != mat.values.end()) {
				material.baseColorTexture = m_textures[mat.values["baseColorTexture"].TextureIndex()];
				material.texCoordSets.baseColor = mat.values["baseColorTexture"].TextureTexCoord();
			}
			if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
				material.metallicRoughnessTexture = m_textures[mat.values["metallicRoughnessTexture"].TextureIndex()];
				material.texCoordSets.metallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
			}
			if (mat.values.find("roughnessFactor") != mat.values.end()) {
				material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
			}
			if (mat.values.find("metallicFactor") != mat.values.end()) {
				material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
			}
			if (mat.values.find("baseColorFactor") != mat.values.end()) {
				material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
			}
			if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
				material.normalTexture = m_textures[mat.additionalValues["normalTexture"].TextureIndex()];
				material.texCoordSets.normal = mat.additionalValues["normalTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
				material.emissiveTexture = m_textures[mat.additionalValues["emissiveTexture"].TextureIndex()];
				material.texCoordSets.emissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
				material.occlusionTexture = m_textures[mat.additionalValues["occlusionTexture"].TextureIndex()];
				material.texCoordSets.occlusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
				tinygltf::Parameter param = mat.additionalValues["alphaMode"];
				if (param.string_value == "BLEND") {
					material.alphaMode = MaterialData::ALPHAMODE_BLEND;
				}
				if (param.string_value == "MASK") {
					material.alphaCutoff = 0.5f;
					material.alphaMode = MaterialData::ALPHAMODE_MASK;
				}
			}
			if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
				material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
			}
			if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
				material.emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
			}

			// Extensions
			// @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
			if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) {
				auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
				if (ext->second.Has("specularGlossinessTexture")) {
					auto index = ext->second.Get("specularGlossinessTexture").Get("index");
					material.extension.specularGlossinessTexture = m_textures[index.Get<int>()];
					auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
					material.texCoordSets.specularGlossiness = texCoordSet.Get<int>();
					material.pbrWorkflows.specularGlossiness = true;
				}
				if (ext->second.Has("diffuseTexture")) {
					auto index = ext->second.Get("diffuseTexture").Get("index");
					material.extension.diffuseTexture = m_textures[index.Get<int>()];
				}
				if (ext->second.Has("diffuseFactor")) {
					auto factor = ext->second.Get("diffuseFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
						auto val = factor.Get(i);
						material.extension.diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}
				if (ext->second.Has("specularFactor")) {
					auto factor = ext->second.Get("specularFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
						auto val = factor.Get(i);
						material.extension.specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}
			}

			if (mat.extensions.find("KHR_materials_unlit") != mat.extensions.end()) {
				material.unlit = true;
			}

			if (mat.extensions.find("KHR_materials_emissive_strength") != mat.extensions.end()) {
				auto ext = mat.extensions.find("KHR_materials_emissive_strength");
				if (ext->second.Has("emissiveStrength")) {
					auto value = ext->second.Get("emissiveStrength");
					material.emissiveStrength = (float)value.Get<double>();
				}
			}
				
			material.index = static_cast<uint32_t>(m_materials.size());
			m_materials.push_back(material);
		}
		// Push a default material at the end of the list for meshes with no material assigned
		m_materials.push_back(MaterialData());
	}

	void Mesh::GetNodeProps(const tinygltf::Node& node, const tinygltf::Model& model, uint32_t& vertex_count, uint32_t& index_count) {
		if (node.children.size() > 0) {
			for (size_t i = 0; i < node.children.size(); i++) {
				GetNodeProps(model.nodes[node.children[i]], model, vertex_count, index_count);
			}
		}
		if (node.mesh > -1) {
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
			for (size_t i = 0; i < mesh.primitives.size(); i++) {
				auto primitive = mesh.primitives[i];
				vertex_count += model.accessors[primitive.attributes.find("POSITION")->second].count;
				if (primitive.indices > -1) {
					index_count += model.accessors[primitive.indices].count;
				}
			}
		}
	}

	void Mesh::LoadNode(GLTF_Node* parent, const tinygltf::Node& node, uint32_t node_index, const tinygltf::Model& model) {
		

		GLTF_Node* new_node = new GLTF_Node();
		new_node->parent = parent;
		new_node->index = node_index;
		new_node->name = node.name;
		new_node->matrix = glm::mat4(1.0f);

		if (node.translation.size() == 3) {
			new_node->translation = glm::make_vec3(node.translation.data());
		}
		if (node.rotation.size() == 4) {
			new_node->rotation = glm::make_quat(node.rotation.data());
		}
		if (node.scale.size() == 4) {
			new_node->scale = glm::make_vec3(node.scale.data());
		}
		if (node.matrix.size() == 16) {
			new_node->matrix = glm::make_mat4x4(node.matrix.data());
		}

		if (node.children.size() > 0) {
			for (auto& node_index : node.children)
				LoadNode(new_node, model.nodes[node_index], node_index, model);
		}

		if (node.mesh > -1) {
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
			GLTF_Mesh* new_mesh = new GLTF_Mesh(new_node->matrix);
			for (auto& primitive : mesh.primitives) {
				uint32_t vertex_start = m_vertex_pos;
				uint32_t index_start = m_index_pos;
				uint32_t vertex_count = 0;
				uint32_t index_count = 0;
				// Vertices
				{
					const float* buffer_pos = nullptr;
					const float* buffer_normals = nullptr;
					const float* buffer_uv_set0 = nullptr;
					const float* buffer_uv_set1 = nullptr;
					const float* buffer_color_set0 = nullptr;
					const void* buffer_joints = nullptr;
					const float* buffer_weights = nullptr;

					int posByteStride;
					int normByteStride;
					int uv0ByteStride;
					int uv1ByteStride;
					int color0ByteStride;
					int jointByteStride;
					int weightByteStride;

					if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
						const tinygltf::Accessor& pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
						const tinygltf::BufferView& pos_view = model.bufferViews[pos_accessor.bufferView];
						vertex_count = static_cast<uint32_t>(pos_accessor.count);
						buffer_pos = reinterpret_cast<const float*>(&(model.buffers[pos_view.buffer].data[pos_accessor.byteOffset + pos_view.byteOffset]));
						posByteStride = pos_accessor.ByteStride(pos_view) ? (pos_accessor.ByteStride(pos_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}
					else {
						assert(primitive.attributes.find("POSITION") != primitive.attributes.end());
					}

					if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
						const tinygltf::Accessor& normal_accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& normal_view = model.bufferViews[normal_accessor.bufferView];
						buffer_normals = reinterpret_cast<const float*>(&(model.buffers[normal_view.buffer].data[normal_accessor.byteOffset + normal_view.byteOffset]));
						normByteStride = normal_accessor.ByteStride(normal_view) ? (normal_accessor.ByteStride(normal_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& uv0_accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& uv0_view = model.bufferViews[uv0_accessor.bufferView];
						buffer_uv_set0 = reinterpret_cast<const float*>(&(model.buffers[uv0_view.buffer].data[uv0_accessor.byteOffset + uv0_view.byteOffset]));
						uv0ByteStride = uv0_accessor.ByteStride(uv0_view) ? (uv0_accessor.ByteStride(uv0_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}

					if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
						const tinygltf::Accessor& uv1_accessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
						const tinygltf::BufferView& uv1_view = model.bufferViews[uv1_accessor.bufferView];
						buffer_uv_set1 = reinterpret_cast<const float*>(&(model.buffers[uv1_view.buffer].data[uv1_accessor.byteOffset + uv1_view.byteOffset]));
						uv1ByteStride = uv1_accessor.ByteStride(uv1_view) ? (uv1_accessor.ByteStride(uv1_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}

					if (primitive.attributes.find("COLOR_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& color0_accessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
						const tinygltf::BufferView& uv1_view = model.bufferViews[color0_accessor.bufferView];
						buffer_color_set0 = reinterpret_cast<const float*>(&(model.buffers[uv1_view.buffer].data[color0_accessor.byteOffset + uv1_view.byteOffset]));
						color0ByteStride = color0_accessor.ByteStride(uv1_view) ? (color0_accessor.ByteStride(uv1_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					const tinygltf::Accessor& pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
					for (size_t v = 0; v < pos_accessor.count; v++) {
						MeshData& vert = m_vertex_buffer[m_vertex_pos];
						vert.Position = glm::vec4(glm::make_vec3(&buffer_pos[v * posByteStride]), 1.0f);
						vert.Normal = glm::normalize(glm::vec3(buffer_normals ? glm::make_vec3(&buffer_normals[v * normByteStride]) : glm::vec3(0.0f)));
						vert.UV0 = buffer_uv_set0 ? glm::make_vec2(&buffer_uv_set0[v * uv0ByteStride]) : glm::vec3(0.0f);
						vert.UV1 = buffer_uv_set1 ? glm::make_vec2(&buffer_uv_set1[v * uv1ByteStride]) : glm::vec3(0.0f);
						vert.Color = buffer_color_set0 ? glm::make_vec4(&buffer_color_set0[v * color0ByteStride]) : glm::vec4(1.0f);

						m_vertex_pos++;
					}

				}
				bool has_indices = primitive.indices > -1;
				if (has_indices) {
					const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
					const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];

					index_count = static_cast<uint32_t>(accessor.count);
					const void* data_ptr = &(buffer.data[accessor.byteOffset + buffer_view.byteOffset]);

					switch (accessor.componentType) {
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
						const uint32_t* buf = static_cast<const uint32_t*>(data_ptr);
						for (size_t index = 0; index < accessor.count; index++) {
							m_index_buffer[m_index_pos] = buf[index] + vertex_start;
							m_index_pos++;
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
						const uint16_t* buf = static_cast<const uint16_t*>(data_ptr);
						for (size_t index = 0; index < accessor.count; index++) {
							m_index_buffer[m_index_pos] = buf[index] + vertex_start;
							m_index_pos++;
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
						const uint8_t* buf = static_cast<const uint8_t*>(data_ptr);
						for (size_t index = 0; index < accessor.count; index++) {
							m_index_buffer[m_index_pos] = buf[index] + vertex_start;
							m_index_pos++;
						}
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						return;
					}
				}
				else {
					assert(false);
				}
				uint32_t mat_index = primitive.material > -1 ? primitive.material : -1;
				Primitive* new_primitive = new Primitive(index_start, index_count, vertex_count, mat_index);
				new_mesh->primitives.push_back(new_primitive);
			}
			new_node->mesh = new_mesh;
		}
		if (parent) {
			parent->children.push_back(new_node);
		}
		else {
			m_nodes.push_back(new_node);
		}
		m_linear_nodes.push_back(new_node);
	}
}