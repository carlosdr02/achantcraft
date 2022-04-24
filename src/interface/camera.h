#include <glm/glm.hpp>

float& getCameraSpeed();
float& getAspectRatio();

class Camera {
public:
    glm::vec3 translation;
    glm::vec3 orientation;
    float fov;

    Camera(const glm::vec3& translation, const glm::vec3& target, float fov);

    glm::vec3 getRightVector(const glm::vec3& upVector) const;
    glm::vec3 getUpVector() const;
    glm::vec3 getFrontVector(const glm::vec3& rightVector, const glm::vec3& upVector) const;

    glm::mat4 getViewMatrix(const glm::vec3& upVector) const;
    glm::mat4 getProjectionMatrix() const;
};
