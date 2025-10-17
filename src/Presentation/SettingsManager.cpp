#include "SettingsManager.h"

#include <QSettings>
#include <QMainWindow>

static constexpr const char* kGroupMainWindow = "MainWindow";
static constexpr const char* kKeyGeometry     = "geometry";
static constexpr const char* kKeyState        = "state";

static constexpr const char* kGroupRender = "render";
static constexpr const char* kKeyShowPoints = "showPoints";
static constexpr const char* kKeyShowMesh   = "showMesh";
static constexpr const char* kKeyWireframe  = "wireframe";
static constexpr const char* kKeyPointSize  = "pointSize";

SettingsManager& SettingsManager::instance() {
    static SettingsManager inst;
    return inst;
}

void SettingsManager::saveMainWindowState(const QMainWindow& win) const {
    QSettings s;
    s.beginGroup(kGroupMainWindow);
    s.setValue(kKeyGeometry, win.saveGeometry());
    s.setValue(kKeyState, win.saveState());
    s.endGroup();
}

void SettingsManager::restoreMainWindowState(QMainWindow& win) const {
    QSettings s;
    s.beginGroup(kGroupMainWindow);
    const QByteArray geom = s.value(kKeyGeometry).toByteArray();
    const QByteArray st   = s.value(kKeyState).toByteArray();
    s.endGroup();
    if (!geom.isEmpty()) win.restoreGeometry(geom);
    if (!st.isEmpty())   win.restoreState(st);
}

RenderSettings SettingsManager::loadRenderSettings() const {
    QSettings s;
    s.beginGroup(kGroupRender);
    RenderSettings rs;
    rs.showPoints = s.value(kKeyShowPoints, true).toBool();
    rs.showMesh   = s.value(kKeyShowMesh,   true).toBool();
    rs.wireframe  = s.value(kKeyWireframe,  false).toBool();
    rs.pointSize  = s.value(kKeyPointSize,  3).toInt();
    s.endGroup();
    return rs;
}

void SettingsManager::saveRenderSettings(const RenderSettings& rs) const {
    QSettings s;
    s.beginGroup(kGroupRender);
    s.setValue(kKeyShowPoints, rs.showPoints);
    s.setValue(kKeyShowMesh,   rs.showMesh);
    s.setValue(kKeyWireframe,  rs.wireframe);
    s.setValue(kKeyPointSize,  rs.pointSize);
    s.endGroup();
}

int SettingsManager::loadPointSize(int defaultValue) const {
    QSettings s;
    s.beginGroup(kGroupRender);
    const int v = s.value(kKeyPointSize, defaultValue).toInt();
    s.endGroup();
    return v;
}

void SettingsManager::savePointSize(int v) const {
    QSettings s;
    s.beginGroup(kGroupRender);
    s.setValue(kKeyPointSize, v);
    s.endGroup();
}

