#pragma once

#include "Event.hpp"

#include <functional>

using EventCallback = std::function<void(Event&)>;

class EventListener {
public:
    virtual ~EventListener() = default;

    void SetCallback(EventCallback callback) {
        m_Callback = callback;
    }

    void OnEvent(Event& e) {
        if (m_Callback)
            m_Callback(e);
    }

private:
    EventCallback m_Callback;
};
