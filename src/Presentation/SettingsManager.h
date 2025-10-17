#pragma once
#include <QByteArray>

class QMainWindow;

struct RenderSettings {
    bool showPoints {true};
    bool showMesh {true};
    bool wireframe {false};
    int  pointSize {3};
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
