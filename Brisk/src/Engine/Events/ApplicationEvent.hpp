#pragma once

#include "Event.hpp"

#include <sstream>

namespace Brisk 
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			:m_Width(width), m_Height(height) {}

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		unsigned int m_Width, m_Height;
	};

	class WindowFocusEvent : public Event
	{
	public:
		WindowFocusEvent(int focus)
			:m_Focus(focus) {}

		inline int GetFocus() const { return m_Focus; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowFocusEvent: " << m_Focus;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowFocus)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		int m_Focus;
	};

	class WindowLoseFocusEvent : public Event
	{
	public:
		WindowLoseFocusEvent(int focus)
			:m_Focus(focus) {}

		inline int GetFocus() const { return m_Focus; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowLoseFocusEvent: " << m_Focus;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowLoseFocus)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		int m_Focus;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() {}

		EVENT_CLASS_TYPE(AppTick)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() {}

		EVENT_CLASS_TYPE(AppRender)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() {}

		EVENT_CLASS_TYPE(AppUpdate)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};

}