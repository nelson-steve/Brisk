#pragma once
#include <chrono>

namespace Brisk 
{
    namespace Core {
        class Timer
        {
        public:
            Timer()
                : m_StartTime(std::chrono::high_resolution_clock::now()),
                m_LastTime(m_StartTime),
                m_DeltaTime(0.0f),
                m_GameTime(0.0f),
                m_TimeScale(1.0f),
                m_IsPaused(false)
            {}

            // Resets the timer
            void Reset()
            {
                m_StartTime = std::chrono::high_resolution_clock::now();
                m_LastTime = m_StartTime;
                m_GameTime = 0.0f;
                m_DeltaTime = 0.0f;
            }

            // Updates the timer, should be called every frame
            void Update()
            {
                if (m_IsPaused)
                    return;

                auto currentTime = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> elapsedTime = currentTime - m_LastTime;

                m_DeltaTime = elapsedTime.count() * m_TimeScale;
                m_GameTime += m_DeltaTime;

                m_LastTime = currentTime;
            }

            // Pauses the game time
            void Pause()
            {
                m_IsPaused = true;
            }

            // Resumes the game time
            void Resume()
            {
                m_IsPaused = false;
                m_LastTime = std::chrono::high_resolution_clock::now();  // Reset last time to avoid jump
            }

            // Sets the time scale (1.0 = normal speed, 0.5 = slow motion, 2.0 = fast motion)
            void SetTimeScale(float scale)
            {
                m_TimeScale = scale;
            }

            // Returns the delta time (time between frames) in seconds
            float GetDeltaTime() const
            {
                return m_DeltaTime;
            }

            // Returns the total game time in seconds
            float GetGameTime() const
            {
                return m_GameTime;
            }

            // Returns the real-world time since the start of the game in seconds
            float GetRealTime() const
            {
                auto currentTime = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> realTimeElapsed = currentTime - m_StartTime;
                return realTimeElapsed.count();
            }

            // Returns whether the timer is paused
            bool IsPaused() const
            {
                return m_IsPaused;
            }

        private:
            std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
            std::chrono::time_point<std::chrono::high_resolution_clock> m_LastTime;
            float m_DeltaTime;    // Time between frames
            float m_GameTime;     // Scaled game time
            float m_TimeScale;    // Time scale for game time (can be modified for slow-motion or fast-motion effects)
            bool m_IsPaused;      // Indicates whether the game time is paused
        };

    }
}