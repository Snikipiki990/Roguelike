#ifndef FUNC_HPP
#define FUNC_HPP

#include "shader.hpp"
#include <iomanip>
#include <fstream>

[[nodiscard]] inline bool checkRange(const glm::vec3 &position, const float radius, const glm::vec3 &player) noexcept {

    glm::vec3 diff = player - position;
    float distanceSquared = glm::dot(diff, diff);
    return distanceSquared < (radius * radius);
}

#define FILE_NAME "log.txt"
#define SETTING_NAME "settings.txt"



inline void logAction(std::string_view msg) noexcept {
    std::ofstream log_file(FILE_NAME, std::ios::app);
    if (log_file.is_open()) {
        std::time_t const now = std::time(nullptr);
        std::tm const *tm_info = std::localtime(&now);
        log_file << "[" << std::put_time(tm_info, "%H:%M:%S") << "]: " << msg << std::endl;
        log_file.close();
    }
}

[[nodiscard]] inline glm::vec3 calcFront(const float &pitch , const float &yaw) noexcept{
    glm::vec3 front;
    front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);
    return front;
}


[[nodiscard]] inline float findDistance(const glm::vec3 &first , const glm::vec3 &second) noexcept {
    const float x = first.x - second.x;
    const float z = first.z - second.z;
    return sqrt((x * x) + (z * z));
}

[[nodiscard]] inline float getPercentage(const double &number , const double &maxNumber) noexcept{
    [[unlikely]] if(maxNumber <= 0 ) return 0;
    return (float)(number / (maxNumber/100.0f))/100.0f;
}

#endif