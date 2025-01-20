// INCLUDES
#include "Scene.hpp"
#include "Entity.hpp"
#include "Component.hpp"
#include "Engine/Engine.hpp"
//--------------------------
#include "imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
//-------------------------------------

namespace Brisk 
{

	//extern std::unique_ptr<AssetSystem> m_AssetManager;

	Scene::Scene()
	{
	}

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

	void Scene::InitDefaults()
	{
	}

	void Scene::InitScene()
	{
		//m_PhysicsWorld3D = std::make_shared<PhysicsWorld>();
		//m_PhysicsWorld3D->Init();
	}

	Entity Scene::CreateMeshEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		std::shared_ptr<Mesh> model;
		model = std::make_shared<Mesh>();
		model->Load("../Data/Models/revolver/revolver.gltf");
		entity.AddComponent<MaterialComponent>();
		entity.AddComponent<MeshComponent>().pModel = model;
		auto& mat = entity.GetComponent<MaterialComponent>();

		mat.pMaterials.push_back(Shader::Create());
		mat.pMaterials[0]->Init(Engine::s_Application->GetRenderer()->pipeline, "material");
		mat.pMaterials[0]->SetMVPBuffer(Engine::s_Application->GetCamera()->mMVPBuffer);
		mat.pMaterials[0]->UpdateResources();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateCubeEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<ScriptComponent>();
		//entity.AddComponent<ModelComponent>().AddMesh(m_AssetManager->LoadGLTFModel("Assets/gltf_models/Cube/glTF/Cube.gltf"), (uint32_t)entity);
		//entity.AddComponent<ShaderComponent>().AddShader(m_AssetManager->LoadShader("Assets/Shaders/ModelShader.glsl"));
		entity.AddComponent<PhysicsComponent>();
		entity.AddComponent<BoxColliderComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreatePlaneEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<ScriptComponent>();
		//entity.AddComponent<ModelComponent>().AddMesh(m_AssetManager->LoadGLTFModel("Assets/models/board/board.obj"), (uint32_t)entity);
		//entity.AddComponent<TextureComponent>().AddTexture(m_AssetManager->LoadTexture("Assets/Models/board/albedo.png"), 0);
		//entity.GetComponent<TextureComponent>().AddTexture(m_AssetManager->LoadTexture("Assets/Models/board/ao.png"), 1);
		//entity.AddComponent<ShaderComponent>().AddShader(m_AssetManager->LoadShader("Assets/Shaders/ModelPBRShader.glsl"));


		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateLightEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<LightComponent>();
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

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::Step(int frames)
	{
		m_StepFrames = frames;
	}

	void Scene::EnableGravity(bool gravity)
	{
		//if (gravity)
		//	m_PhysicsWorld->SetGravity({ 0.0f, -9.8f });
		//else
		//	m_PhysicsWorld->SetGravity({ 0.0f, 0.0f });
	}

	void Scene::OnRuntimeStart()
	{
		m_IsRunning = true;

		{
			auto view = m_Registry.view<PhysicsComponent>();
			for (auto e : view) {
				Entity entity = { e, this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& physics = entity.GetComponent<PhysicsComponent>();

				//if (physics.bodyType == PhysicsComponent::BodyType::Static)
				//	physics.StaticRuntimeBody = m_PhysicsWorld3D->CreateStaticBody(transform.GetPosition(), glm::vec4(transform.GetRotation(), 1.0f));
				//else if (physics.bodyType == PhysicsComponent::BodyType::Dynamic)
				//{
				//	physics.DynamicRuntimeBody = m_PhysicsWorld3D->CreateDynamicBody(transform.GetPosition(), glm::vec4(transform.GetRotation(), 1.0f));
				//	physics.DynamicRuntimeBody->SetBodyType(DynamicBody::BodyType(physics.rigidBodyType));
				//}

				//if (entity.HasComponent<BoxColliderComponent>())
				//{
				//	auto& bc3d = entity.GetComponent<BoxColliderComponent>();

				//	if (physics.bodyType == PhysicsComponent::BodyType::Static) {
				//		Ref<BoxCollider> collider = m_PhysicsWorld3D->CreateBoxShape(bc3d.Size);
				//		physics.StaticRuntimeBody->AddBoxCollider(collider, bc3d.offset);
				//	}
				//	if (physics.bodyType == PhysicsComponent::BodyType::Dynamic) {
				//		Ref<BoxCollider> collider = m_PhysicsWorld3D->CreateBoxShape(bc3d.Size);
				//		physics.DynamicRuntimeBody->AddBoxCollider(collider, bc3d.offset);
				//	}
				//}
			}
		}

		{
			//m_PhysicsWorld = new b2World({ 0.0f, -9.81f });
			EnableGravity(m_SceneSetting.enableGravity);
			auto view = m_Registry.view<Physics2DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rb2d = entity.GetComponent<Physics2DComponent>();

				//b2BodyDef bodyDef;
				//bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
				//bodyDef.position.Set(transform.Position.x, transform.Position.y);
				//bodyDef.angle = transform.Rotation.z;

				//b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
				//body->SetFixedRotation(rb2d.FixedRotation);
				//rb2d.RuntimeBody = body;

				if (entity.HasComponent<BoxCollider2DComponent>())
				{
					auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

					//b2PolygonShape boxShape;
					//boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

					//b2FixtureDef fixtureDef;
					//fixtureDef.shape = &boxShape;
					//fixtureDef.density = bc2d.Density;
					//fixtureDef.friction = bc2d.Friction;
					//fixtureDef.restitution = bc2d.Restitution;
					//fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
					//body->CreateFixture(&fixtureDef);
				}
			}
		}

		// Scripting
		{
			//ScriptEngine::OnRuntimeStart(this);
			// Instantiate all script entities

			auto view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				//ScriptEngine::OnCreateEntity(entity);
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		m_IsRunning = false;;

		//delete m_PhysicsWorld;
		//m_PhysicsWorld = nullptr;

		//ScriptEngine::OnRuntimeStop();
	}

	void Scene::OnSimulationStart()
	{
		m_IsSimulating = true;

		{
			auto view = m_Registry.view<PhysicsComponent>();
			for (auto e : view) {
				Entity entity = { e, this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& physics = entity.GetComponent<PhysicsComponent>();

				//if (physics.bodyType == PhysicsComponent::BodyType::Static)
				//	physics.StaticRuntimeBody = m_PhysicsWorld3D->CreateStaticBody(transform.GetPosition(), glm::vec4(transform.GetRotation(), 1.0f));
				//else if (physics.bodyType == PhysicsComponent::BodyType::Dynamic)
				//{
				//	physics.DynamicRuntimeBody = m_PhysicsWorld3D->CreateDynamicBody(transform.GetPosition(), glm::vec4(transform.GetRotation(), 1.0f));
				//	physics.DynamicRuntimeBody->SetBodyType(DynamicBody::BodyType(physics.rigidBodyType));
				//}

				//if (entity.HasComponent<BoxColliderComponent>())
				//{
				//	auto& bc3d = entity.GetComponent<BoxColliderComponent>();

				//	if (physics.bodyType == PhysicsComponent::BodyType::Static) {
				//		Ref<BoxCollider> collider = m_PhysicsWorld3D->CreateBoxShape(bc3d.Size);
				//		physics.StaticRuntimeBody->AddBoxCollider(collider, bc3d.offset);
				//	}
				//	if (physics.bodyType == PhysicsComponent::BodyType::Dynamic) {
				//		Ref<BoxCollider> collider = m_PhysicsWorld3D->CreateBoxShape(bc3d.Size);
				//		physics.DynamicRuntimeBody->AddBoxCollider(collider, bc3d.offset);
				//	}
				//}
			}
		}

		//m_PhysicsWorld = new b2World({ 0.0f, -9.8f });
		EnableGravity(m_SceneSetting.enableGravity);
		{
			auto view = m_Registry.view<Physics2DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rb2d = entity.GetComponent<Physics2DComponent>();

				//b2BodyDef bodyDef;
				//bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
				//bodyDef.position.Set(transform.Position.x, transform.Position.y);
				//bodyDef.angle = transform.Rotation.z;

				//b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
				//body->SetFixedRotation(rb2d.FixedRotation);
				//rb2d.RuntimeBody = body;

				if (entity.HasComponent<BoxCollider2DComponent>())
				{
					auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

					//b2PolygonShape boxShape;
					//boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);
					//boxShape.SetAsBox(bc2d.Size.x, bc2d.Size.y);

					//b2FixtureDef fixtureDef;
					//fixtureDef.shape = &boxShape;
					//fixtureDef.density = bc2d.Density;
					//fixtureDef.friction = bc2d.Friction;
					//fixtureDef.restitution = bc2d.Restitution;
					//fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
					//body->CreateFixture(&fixtureDef);
				}
			}
		}

		//ScriptEngine::OnRuntimeStart(this);
		//// Instantiate all script entities
		//{
		//	auto view = m_Registry.view<ScriptComponent>();
		//	for (auto e : view)
		//	{
		//		Entity entity = { e, this };
		//		ScriptEngine::OnCreateEntity(entity);
		//	}
		//}
	}

	void Scene::OnSimulationStop()
	{
		//m_PhysicsWorld3D->Destroy();

		//delete m_PhysicsWorld;
		//m_PhysicsWorld = nullptr;

		m_IsSimulating = false;
	}

	//void Scene::OnUpdateSimulation(Ref<EditorCamera> camera, Timestep ts)
	//{
	//	m_PhysicsWorld3D->Update(ts);
	//	{
	//		auto view = m_Registry.view<PhysicsComponent>();
	//		for (auto e : view) {
	//			Entity entity = { e, this };
	//			auto& transform = entity.GetComponent<TransformComponent>();
	//			auto& physics = entity.GetComponent<PhysicsComponent>();

	//			if (physics.bodyType == PhysicsComponent::BodyType::Dynamic)
	//				transform.Position = physics.DynamicRuntimeBody->GetPosition();
	//		}
	//	}

	//	// Physics
	//	EnableGravity(m_SceneSetting.enableGravity);
	//	{
	//		const int32_t velocityIterations = 6;
	//		const int32_t positionIterations = 2;
	//		m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

	//		// Retrieve transform from Box2D
	//		auto view = m_Registry.view<Physics2DComponent>();
	//		for (auto e : view)
	//		{
	//			Entity entity = { e, this };
	//			auto& transform = entity.GetComponent<TransformComponent>();
	//			auto& rb2d = entity.GetComponent<Physics2DComponent>();

	//			b2Body* body = (b2Body*)rb2d.RuntimeBody;
	//			const auto& position = body->GetPosition();
	//			transform.Position.x = position.x;
	//			transform.Position.y = position.y;
	//			transform.Rotation.z = body->GetAngle();
	//		}
	//	}

	//	RenderScene(camera.get(), ts);
	//}

	//void Scene::OnUpdateResize(uint32_t width, uint32_t height)
	//{
	//	assert(false);
	//}

	//void Scene::OnUpdateRuntime(Timestep ts)
	//{
	//	Ref<SceneCamera> sceneCamera = nullptr;

	//	auto view = m_Registry.view<TransformComponent, CameraComponent>();
	//	for (auto entity : view)
	//	{
	//		auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

	//		if (camera.Primary)
	//		{
	//			sceneCamera = camera.Camera;
	//			sceneCamera->SetPosition(transform.Position);
	//			break;
	//		}
	//	}

	//	m_PhysicsWorld3D->Update(ts);
	//	{
	//		auto view = m_Registry.view<PhysicsComponent>();
	//		for (auto e : view) {
	//			Entity entity = { e, this };
	//			auto& transform = entity.GetComponent<TransformComponent>();
	//			auto& physics = entity.GetComponent<PhysicsComponent>();

	//			if (physics.bodyType == PhysicsComponent::BodyType::Dynamic)
	//				transform.Position = physics.DynamicRuntimeBody->GetPosition();
	//		}
	//	}

	//	// Physics
	//	EnableGravity(m_SceneSetting.enableGravity);
	//	{
	//		const int32_t velocityIterations = 6;
	//		const int32_t positionIterations = 2;
	//		m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

	//		// Retrieve transform from Box2D
	//		auto view = m_Registry.view<Physics2DComponent>();
	//		for (auto e : view)
	//		{
	//			Entity entity = { e, this };
	//			auto& transform = entity.GetComponent<TransformComponent>();
	//			auto& rb2d = entity.GetComponent<Physics2DComponent>();

	//			b2Body* body = (b2Body*)rb2d.RuntimeBody;
	//			const auto& position = body->GetPosition();
	//			transform.Position.x = position.x;
	//			transform.Position.y = position.y;
	//			transform.Rotation.z = body->GetAngle();
	//		}
	//	}

	//	RenderScene(sceneCamera.get(), ts);
	//}

	//void Scene::RenderScene(Camera* camera, Timestep ts)
	//{
	//	auto view = m_Registry.view<ShaderComponent, TransformComponent, ModelComponent,
	//		TextureComponent, MaterialComponent, ScriptComponent>();

	//	// Getting light components
	//	auto& lightComponents = m_Registry.view<LightComponent>();
	//	std::vector<LightComponent> lights;
	//	glm::vec3 lightDirection{ 0.0 };
	//	for (auto& entity : lightComponents)
	//	{
	//		auto& dirLight = lightComponents.get<LightComponent>(entity);
	//		if (dirLight.type == dirLight.LightType::Directional)
	//		{
	//			lightDirection = lightComponents.get<LightComponent>(entity).direction;
	//		}
	//		lights.push_back(lightComponents.get<LightComponent>(entity));
	//	}
	//	glm::vec3 l;
	//	if (lights.empty())
	//		l = glm::vec3(0.0f, 5.0f, 0.0f);
	//	else
	//		l = lights[0].position;

	//	m_Framebuffer->Bind();
	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//	auto skybox_view = m_Registry.view<ShaderComponent, SkyboxComponent, TransformComponent>();
	//	for (auto& entity : skybox_view)
	//	{
	//		glDepthMask(GL_FALSE);
	//		Renderer::SetupSkybox(camera, skybox_view.get<SkyboxComponent>(entity), skybox_view.get<ShaderComponent>(entity),
	//			skybox_view.get<TransformComponent>(entity).GetTransform());
	//		Renderer::Render(skybox_view.get<SkyboxComponent>(entity).m_Model, skybox_view.get<ShaderComponent>(entity).m_Shader);
	//		glDepthMask(GL_TRUE);
	//	}

	//	auto temp_view = m_Registry.view<ShaderComponent, ModelComponent, TransformComponent>();
	//	for (auto& entity : temp_view)
	//	{
	//		Renderer::Setup(camera, view.get<ShaderComponent>(entity).m_Shader, view.get<TransformComponent>(entity).GetTransform());
	//		Renderer::Render(view.get<ModelComponent>(entity).m_Model, view.get<ShaderComponent>(entity).m_Shader);
	//	}

	//	auto phyView = m_Registry.view<TransformComponent, BoxCollider2DComponent, Physics2DComponent>();

	//	for (auto& entity : phyView)
	//	{
	//		glm::mat4 transform;
	//		auto& tra = phyView.get<TransformComponent>(entity);
	//		auto& bc2d = phyView.get<BoxCollider2DComponent>(entity);
	//		transform = glm::translate(glm::mat4(1.0f), tra.Position)
	//			* glm::scale(glm::mat4(1.0f), glm::vec3(bc2d.Size.x * 2, bc2d.Size.y * 2, 1.0));

	//		if (m_SceneSetting.ShowCollider)
	//		{
	//			Renderer::Setup(camera, m_ColliderShader, transform);
	//			//Renderer::RenderOverlay(m_Collider);
	//		}
	//	}

	//	auto colView = m_Registry.view<TransformComponent, BoxColliderComponent, PhysicsComponent>();

	//	for (auto& entity : colView)
	//	{
	//		glm::mat4 transform{ 1.0f };
	//		auto& tra = colView.get<TransformComponent>(entity);
	//		auto& bc3d = colView.get<BoxColliderComponent>(entity);
	//		transform = glm::translate(glm::mat4(1.0f), tra.Position)
	//			* glm::scale(glm::mat4(1.0f), glm::vec3(bc3d.Size.x, bc3d.Size.y, bc3d.Size.z));

	//		if (m_SceneSetting.ShowCollider)
	//		{
	//			Renderer::SetupCollider(camera, m_ColliderShader, transform);
	//			Renderer::RenderCollider(m_Cube, m_ColliderShader);
	//		}
	//	}
	//}

	//void Scene::OnUpdateEditor(const Ref<EditorCamera> camera, Timestep ts)
	//{
	//	// reload shader
	//	auto view = m_Registry.view<ShaderComponent, TransformComponent, ModelComponent>();
	//	for (auto& entity : view)
	//	{
	//		auto& shader = view.get<ShaderComponent>(entity);
	//		if (shader.tobeinitialized) {
	//			shader.m_Shader = Shader::Create(shader.m_Shader->GetPath());
	//			shader.tobeinitialized = false;
	//		}
	//	}

	//	RenderScene(camera.get(), ts);
	//}

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

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}
		return {};
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
	void Scene::OnComponentAdded<LightComponent>(Entity entity, LightComponent& component)
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
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
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

	template<>
	void Scene::OnComponentAdded<Physics2DComponent>(Entity entity, Physics2DComponent& component)
	{
	}

}
