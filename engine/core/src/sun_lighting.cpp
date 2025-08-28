#include "arena/sun_lighting.hpp"
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <cmath>

namespace arena {

SunLighting::SunLighting() : timeOfDay_(15.0f) { // Start at 3 PM
    updateSunPosition();
    updateSunColor();
    updateAmbientColor();
}

void SunLighting::setTimeOfDay(float timeHours) {
    timeOfDay_ = std::clamp(timeHours, 0.0f, 24.0f);
    updateSunPosition();
    updateSunColor();
    updateAmbientColor();
}

void SunLighting::adjustTime(float deltaHours) {
    setTimeOfDay(timeOfDay_ + deltaHours);
}

glm::vec3 SunLighting::getSunDirection() const {
    return sunDirection_;
}

glm::vec3 SunLighting::getSunColor() const {
    return sunColor_;
}

glm::vec3 SunLighting::getAmbientColor() const {
    return ambientColor_;
}

void SunLighting::updateSunPosition() {
    // Convert time to angle (0-24 hours -> 0-2π radians)
    // 6 AM = 0 radians (east), 12 PM = π/2 radians (south), 6 PM = π radians (west)
    float timeAngle = (timeOfDay_ - 6.0f) * glm::pi<float>() / 12.0f;
    
    // Calculate sun height based on time
    // Peak height at noon, lower at sunrise/sunset
    float sunHeight;
    if (timeOfDay_ >= 6.0f && timeOfDay_ <= 18.0f) {
        // Daytime: sun rises from 0 to 1 and back to 0
        float dayProgress = (timeOfDay_ - 6.0f) / 12.0f; // 0 to 1
        float heightProgress = 2.0f * std::abs(dayProgress - 0.5f); // 0 to 1 to 0
        sunHeight = 1.0f - heightProgress; // 1 to 0 to 1
    } else {
        // Nighttime: sun is below horizon
        sunHeight = -0.5f;
    }
    
    // Calculate sun direction
    // X-axis is east-west, Y-axis is up-down, Z-axis is north-south
    // We want sun to move from east (left) to west (right) as viewed from camera
    float sunX = -std::cos(timeAngle); // Negative for left side of camera
    float sunY = sunHeight;
    float sunZ = -std::sin(timeAngle); // Negative for south direction
    
    sunDirection_ = glm::normalize(glm::vec3(sunX, sunY, sunZ));
}

void SunLighting::updateSunColor() {
    if (timeOfDay_ >= 6.0f && timeOfDay_ <= 18.0f) {
        // Daytime: bright white-yellow sun
        if (timeOfDay_ >= 10.0f && timeOfDay_ <= 14.0f) {
            // Peak sun: bright white
            sunColor_ = glm::vec3(1.0f, 1.0f, 0.95f);
        } else if (timeOfDay_ >= 8.0f && timeOfDay_ <= 16.0f) {
            // Morning/afternoon: warm yellow
            sunColor_ = glm::vec3(1.0f, 0.95f, 0.8f);
        } else {
            // Sunrise/sunset: warm orange
            sunColor_ = glm::vec3(1.0f, 0.8f, 0.6f);
        }
    } else {
        // Nighttime: very dim blue-ish light
        sunColor_ = glm::vec3(0.1f, 0.15f, 0.3f);
    }
}

void SunLighting::updateAmbientColor() {
    if (timeOfDay_ >= 6.0f && timeOfDay_ <= 18.0f) {
        // Daytime: bright blue sky
        if (timeOfDay_ >= 10.0f && timeOfDay_ <= 14.0f) {
            // Peak sun: bright blue sky
            ambientColor_ = glm::vec3(0.3f, 0.5f, 0.8f);
        } else if (timeOfDay_ >= 8.0f && timeOfDay_ <= 16.0f) {
            // Morning/afternoon: medium blue sky
            ambientColor_ = glm::vec3(0.25f, 0.4f, 0.7f);
        } else {
            // Sunrise/sunset: warm sky
            ambientColor_ = glm::vec3(0.4f, 0.3f, 0.5f);
        }
    } else {
        // Nighttime: dark blue sky
        ambientColor_ = glm::vec3(0.05f, 0.08f, 0.15f);
    }
}

} // namespace arena
