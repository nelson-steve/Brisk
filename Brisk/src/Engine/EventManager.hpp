#pragma once

#include "EventListener.hpp"

class EventManager {
public:
    static void AddListener(EventListener* listener) {
        s_Listeners.push_back(listener);
    }

    static void RemoveListener(EventListener* listener) {
        s_Listeners.erase(std::remove(s_Listeners.begin(), s_Listeners.end(), listener), s_Listeners.end());
    }

    static void Dispatch(Event& event) {
        for (auto listener : s_Listeners) {
            listener->OnEvent(event);
            if (event.Handled) {
                break;  // Stop propagating if the event is handled
            }
        }
    }

private:
    static std::vector<EventListener*> s_Listeners;
};
