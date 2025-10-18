#pragma once
#include <QByteArray>
#include <QVector3D>

class QMainWindow;

struct RenderSettings {
    bool showPoints {true};
    bool showMesh {true};
    bool wireframe {false};
    int  pointSize {3};
    QVector3D meshColor {0.85f, 0.85f, 0.9f};
    QVector3D pointColor {0.2f, 0.8f, 0.3f};
};

class SettingsManager {
public:
    static SettingsManager& instance();

    // Window state
    void saveMainWindowState(const QMainWindow& win) const;
    void restoreMainWindowState(QMainWindow& win) const;

    // Render settings (batch)
    [[nodiscard]] RenderSettings loadRenderSettings() const;
    void saveRenderSettings(const RenderSettings& rs) const;

    // Granular helpers
    [[nodiscard]] int  loadPointSize(int defaultValue = 3) const;
    void savePointSize(int v) const;

private:
    SettingsManager() = default;
};
