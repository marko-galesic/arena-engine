#pragma once

#include <glm/glm.hpp>

namespace arena {

class SunLighting {
public:
    // Time of day in hours (0-24, where 12 is noon)
    static constexpr float NOON = 12.0f;
    static constexpr float SUNRISE = 6.0f;
    static constexpr float SUNSET = 18.0f;
    
    SunLighting();
    
    // Set the time of day (0-24 hours)
    void setTimeOfDay(float timeHours);
    
    // Get the current sun direction (normalized)
    glm::vec3 getSunDirection() const;
    
    // Get the current sun color
    glm::vec3 getSunColor() const;
    
    // Get the current ambient light color
    glm::vec3 getAmbientColor() const;
    
    // Get the current time of day
    float getTimeOfDay() const { return timeOfDay_; }
    
    // Adjust time by delta (positive = forward in time, negative = backward)
    void adjustTime(float deltaHours);

private:
    float timeOfDay_; // 0-24 hours
    
    // Calculate sun position based on time
    void updateSunPosition();
    
    // Calculate sun color based on time
    void updateSunColor();
    
    // Calculate ambient color based on time
    void updateAmbientColor();
    
    // Current calculated values
    glm::vec3 sunDirection_;
    glm::vec3 sunColor_;
    glm::vec3 ambientColor_;
};

} // namespace arena
