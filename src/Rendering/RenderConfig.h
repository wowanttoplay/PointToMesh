#pragma once
#include <QVector3D>

struct RenderConfig {
    bool showPoints {true};
    bool showMesh {true};
    bool wireframe {false};
    float pointSize {3.0f};
    QVector3D meshColor {0.85f, 0.85f, 0.9f};
    QVector3D pointColor {0.2f, 0.8f, 0.3f};
};

