#pragma once

#include "Camera.hpp"

#include "glm/glm.hpp"

namespace s 
{

	class OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);

		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		float GetRotation() { return m_Rotation; }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		virtual const glm::mat4& GetView() const override { return m_View; }
		virtual const glm::mat4& GetProjectionView() const override { return m_Projection * m_View; }
		virtual const glm::mat4& GetProjection() const override { return m_Projection; }
		virtual const glm::vec3& GetPosition() const override { return m_Position; }
	private:
		void RecalculateViewMatrix();
	private:
		float m_Rotation = 0.0f;
	};

}