#include <glm/glm.hpp>

class Camera {
public:
    glm::vec3 translation;
    glm::vec3 orientation;
    float fov;

    Camera(const glm::vec3& translation, const glm::vec3& target, float fov);

    glm::mat4 getView();
    glm::mat4 getProjection();
};
