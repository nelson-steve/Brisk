#include "InspectorPanel.hpp"
#include "Engine/Engine.hpp"

#include <imgui_internal.h>

namespace Brisk
{
	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

    void InspectorPanel::OnCreate() {
    }

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, entt::entity e, UIFunction uiFunction)
	{
		Entity entity = { e, SceneManager::pActiveScene.get() };
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

    void InspectorPanel::OnUpdate() {
        ImGui::Begin("Inspector");
		if (SceneManager::pActiveScene->GetSelectedEntity() != entt::null /*&& SceneManager::pActiveScene->Reg().valid(SceneManager::pActiveScene->GetSelectedEntity())*/)
		{
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), treeNodeFlags, "Transform");
			ImGui::PopStyleVar(
			);
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				DrawVec3Control("Position", SceneManager::pActiveScene->Reg().get<TransformComponent>(SceneManager::pActiveScene->GetSelectedEntity()).Position);

				glm::quat quat = SceneManager::pActiveScene->Reg().get<TransformComponent>(SceneManager::pActiveScene->GetSelectedEntity()).Rotation;
				glm::vec3 eulerAngles = glm::eulerAngles(quat);
				glm::vec3 rotation = glm::degrees(eulerAngles);
				DrawVec3Control("Rotation", rotation);
				glm::vec3 euler = glm::vec3(rotation.x, rotation.y, rotation.z);
				glm::quat q = glm::quat(euler);
				//SceneManager::pActiveScene->Reg().get<WorldTransformComponent>(SceneManager::pActiveScene->GetSelectedEntity()).Rotation = q;

				DrawVec3Control("Scale", SceneManager::pActiveScene->Reg().get<TransformComponent>(SceneManager::pActiveScene->GetSelectedEntity()).Scale);
				ImGui::TreePop();
			}

			//if (removeComponent)
			//	entity.RemoveComponent<T>();

			DrawComponent<DirectionalLightComponent>("Transform", SceneManager::pActiveScene->GetSelectedEntity(), [](DirectionalLightComponent& component)
				{
					DrawVec3Control("Direction", component.Direction, 0, 70);
				});

			ImVec2 avail = ImGui::GetContentRegionAvail();
			ImVec2 cellSize = { avail.x * 0.5f, avail.y * 0.5f }; // 2x2 grid

			auto ShowShadowMap = [&](ImTextureID texId, const char* label)
				{
					float aspect = (float)2048 / (float)2048;

					ImVec2 imageSize = cellSize;
					if (imageSize.x / imageSize.y > aspect) {
						imageSize.x = imageSize.y * aspect;
					}
					else {
						imageSize.y = imageSize.x / aspect;
					}

					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
					ImGui::Image(texId, imageSize);
					ImGui::PopStyleVar();
				};

			ImGui::BeginChild("ShadowMapDebug");
			{
				// Row 0
				ShowShadowMap((ImTextureID)Application::GetRenderer()->m_ImGuiIdShadowMap0, "Cascade0");
				ImGui::SameLine();
				ShowShadowMap((ImTextureID)Application::GetRenderer()->m_ImGuiIdShadowMap1, "Cascade1");

				// Row 1
				ShowShadowMap((ImTextureID)Application::GetRenderer()->m_ImGuiIdShadowMap2, "Cascade2");
				ImGui::SameLine();
				ShowShadowMap((ImTextureID)Application::GetRenderer()->m_ImGuiIdShadowMap3, "Cascade3");
			}
			ImGui::EndChild();
		}
        ImGui::End();
    }

    void InspectorPanel::OnDestroy() {
        //m_ActiveSceneTexture->Release();
    }
}