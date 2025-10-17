#include "Camera.h"
#include <QtMath>
#include <algorithm>

Camera::Camera() = default;

QMatrix4x4 Camera::viewMatrix() const {
    // Spherical coordinates around target
    const float yawRad = qDegreesToRadians(m_yaw);
    const float pitchRad = qDegreesToRadians(m_pitch);
    const float cx = m_distance * qCos(pitchRad) * qCos(yawRad);
    const float cy = m_distance * qSin(pitchRad);
    const float cz = m_distance * qCos(pitchRad) * qSin(yawRad);
    const QVector3D eye = m_target + QVector3D(cx, cy, cz);

    QMatrix4x4 v; v.setToIdentity();
    v.lookAt(eye, m_target, QVector3D(0,1,0));
    return v;
}

QMatrix4x4 Camera::projMatrix(float aspect) const {
    QMatrix4x4 p; p.setToIdentity();
    if (m_perspective) {
        p.perspective(m_fov, std::max(0.01f, aspect), m_near, m_far);
    } else {
        // Simple ortho based on distance and fov
        float halfH = qTan(qDegreesToRadians(m_fov*0.5f)) * m_distance;
        float halfW = halfH * aspect;
        p.ortho(-halfW, halfW, -halfH, halfH, m_near, m_far);
    }
    return p;
}

void Camera::orbit(float dx, float dy) {
    // dx,dy are pixel deltas normalized by widget size typically; scale for sensitivity
    const float sens = 180.0f; // degrees per normalized unit
    m_yaw   = std::fmod(m_yaw   + dx * sens, 360.0f);
    m_pitch = std::clamp(m_pitch + dy * sens, -89.0f, 89.0f);
}

void Camera::zoom(float delta) {
    // delta ~ wheel steps; scale exponentially for smoothness
    const float scale = std::pow(0.9f, delta);
    m_distance = std::clamp(m_distance * scale, 0.05f, 1e6f);
    m_near = std::min(m_near, m_distance * 0.1f);
    m_far  = std::max(m_far,  m_distance * 10.0f);
}

void Camera::pan(float dx, float dy) {
    // Pan in view space relative to distance
    const float yawRad = qDegreesToRadians(m_yaw);
    QVector3D right(qCos(yawRad), 0.0f, qSin(yawRad));
    QVector3D up(0,1,0);
    float scale = m_distance * 1.0f; // proportional to distance
    m_target += (-dx * scale) * right + (dy * scale) * up;
}

QVector3D Camera::position() const {
    const float yawRad = qDegreesToRadians(m_yaw);
    const float pitchRad = qDegreesToRadians(m_pitch);
    const float cx = m_distance * qCos(pitchRad) * qCos(yawRad);
    const float cy = m_distance * qSin(pitchRad);
    const float cz = m_distance * qCos(pitchRad) * qSin(yawRad);
    return m_target + QVector3D(cx, cy, cz);
}

