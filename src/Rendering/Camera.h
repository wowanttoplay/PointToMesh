#pragma once
#include <QMatrix4x4>
#include <QVector3D>

class Camera {
public:
    Camera();

    // Parameters
    void setTarget(const QVector3D& t) { m_target = t; }
    void setDistance(float d) { m_distance = std::max(0.01f, d); }
    void setYawPitch(float yawDeg, float pitchDeg) { m_yaw = yawDeg; m_pitch = std::clamp(pitchDeg, -89.0f, 89.0f); }
    void setFov(float fovDeg) { m_fov = std::clamp(fovDeg, 10.0f, 120.0f); }
    void setNearFar(float n, float f) { m_near = std::max(1e-3f, n); m_far = std::max(f, m_near + 1e-3f); }
    void setPerspective(bool p) { m_perspective = p; }

    // Derived
    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projMatrix(float aspect) const;

    // Interaction
    void orbit(float dx, float dy);    // dx,dy in pixels normalized by widget size
    void zoom(float delta);            // delta: wheel steps (positive = zoom in)
    void pan(float dx, float dy);      // screen-space pan

    QVector3D position() const;

private:
    QVector3D m_target {0,0,0};
    float m_distance {3.0f};
    float m_yaw {45.0f};     // degrees
    float m_pitch {-20.0f};  // degrees
    float m_fov {45.0f};     // degrees
    float m_near {0.01f};
    float m_far {1000.0f};
    bool  m_perspective {true};
};

