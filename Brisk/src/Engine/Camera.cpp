// INCLUDES
#include "Camera.hpp"
//-------------------
#include "glfw3.h"
//----------------

namespace Brisk 
{
	Camera::Camera(GLFWwindow* window) 
		: m_Window(window), 
		m_Projection(glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip)) {
		UpdateView();

		//mMVPBuffer = Buffer::Create();
		//mMVPBuffer->Init(sizeof(MVP), nullptr,
		//	{
		//		Core::BufferUsage::BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		//	},
		//	{
		//		Core::MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		//		Core::MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT
		//	},
		//	false
		//);
	}

	Camera::Camera(float fov, float aspectRatio, float nearClip, float farClip, GLFWwindow* window)
		: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), m_Window(window) {
		m_Projection = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
		UpdateView();
	}

	void Camera::UpdateProjection() {
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void Camera::UpdateView() {
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	std::pair<float, float> Camera::PanSpeed() const {
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float Camera::RotationSpeed() const {
		return 0.8f;
	}

	float Camera::ZoomSpeed() const {
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void Camera::OnUpdate(float t) {
		assert(m_Window != nullptr);
		if (glfwGetKey(m_Window, GLFW_KEY_LEFT_ALT) && m_MouseMoved) {
			glm::vec2 delta = m_MouseOffset * 0.003f;
		
			if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_MIDDLE))
				MousePan(delta);
			else if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT))
				MouseRotate(delta);
			else if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT))
				MouseZoom(delta.y);
			m_MouseMoved = false;
		}
		UpdateView();

		MVP mvp{};
		mvp.Model = glm::mat4(1.0f);
		mvp.View = glm::mat4(1.0f);
		mvp.Projection = glm::mat4(1.0f);

		mMVPBuffer->UpdatePersistantData(sizeof(mvp), &mvp);
	}

	bool Camera::OnMouseScroll(float yOffset) {
		float delta = yOffset * 0.1f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void Camera::MousePan(const glm::vec2& delta) {
		std::pair<float, float> Speed = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * Speed.first * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * Speed.second * m_Distance;
	}

	void Camera::MouseRotate(const glm::vec2& delta) {
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void Camera::MouseZoom(float delta) {
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 Camera::GetUpDirection() const {
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 Camera::GetRightDirection() const {
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 Camera::GetForwardDirection() const {
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 Camera::CalculatePosition() const {
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::quat Camera::GetOrientation() const {
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}
}