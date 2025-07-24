#pragma once

// INCLUDES
#include "Camera.hpp"
//------------------
#include "glm/glm.hpp"
//-------------------

namespace Brisk 
{
	class OrthographicCamera : public Camera
	{
	public:
		void SetProjection(float left, float right, float bottom, float top);

		float GetRotation() { return m_Rotation; }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }
	private:
		void RecalculateViewMatrix();
	private:
		float m_Rotation = 0.0f;
	};

}