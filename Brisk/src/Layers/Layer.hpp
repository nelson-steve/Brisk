#pragma once

#include "Engine/Events/Event.hpp"

#include <string>

namespace Brisk 
{
	class Layer {
	public:
		Layer(const std::string& debugName = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& e) {}

		inline const std::string GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}