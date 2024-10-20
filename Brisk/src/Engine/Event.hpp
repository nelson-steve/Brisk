#pragma once

#include <string>
#include <functional>

// Base Event class
class Event
{
public:
    enum EventType
    {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    enum EventCategory
    {
        //None = 0,
        EventCategoryApplication = 1 << 0,
        EventCategoryInput = 1 << 1,
        EventCategoryKeyboard = 1 << 2,
        EventCategoryMouse = 1 << 3,
        EventCategoryMouseButton = 1 << 4
    };

    virtual EventType GetEventType() const = 0;
    virtual int GetCategoryFlags() const = 0;
    virtual std::string ToString() const { return "Event"; }

    inline bool IsInCategory(EventCategory category)
    {
        return GetCategoryFlags() & category;
    }

    bool Handled = false;
};
