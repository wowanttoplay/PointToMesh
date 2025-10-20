#include "SettingsManager.h"

#include <QSettings>
#include <QMainWindow>

static constexpr const char* kGroupMainWindow = "MainWindow";
static constexpr const char* kKeyGeometry     = "geometry";
static constexpr const char* kKeyState        = "state";

static constexpr const char* kGroupRender = "render";
static constexpr const char* kKeyShowPoints = "showPoints";
static constexpr const char* kKeyShowNormals   = "showNormals";
static constexpr const char* kKeyShowMesh   = "showMesh";
static constexpr const char* kKeyWireframe  = "wireframe";
static constexpr const char* kKeyPointSize  = "pointSize";
static constexpr const char* kKeyMeshColor  = "meshColor";   // QVariantList [r,g,b]
static constexpr const char* kKeyPointColor = "pointColor";  // QVariantList [r,g,b]
static constexpr const char* kKeyWireColor  = "wireColor";   // QVariantList [r,g,b]
static constexpr const char* kKeyCameraSpeed = "cameraSpeed"; // double

static inline QVariantList toVarList(const QVector3D& v) {
    return QVariantList{ v.x(), v.y(), v.z() };
}
static inline QVector3D toVec3(const QVariant& var, const QVector3D& defVal) {
    const auto list = var.toList();
    if (list.size() != 3) return defVal;
    return QVector3D(list.at(0).toFloat(), list.at(1).toFloat(), list.at(2).toFloat());
}

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
    rs.showNormals = s.value(kKeyShowNormals, true).toBool();
    rs.showMesh   = s.value(kKeyShowMesh,   true).toBool();
    rs.wireframe  = s.value(kKeyWireframe,  false).toBool();
    rs.pointSize  = s.value(kKeyPointSize,  3).toInt();
    rs.meshColor  = toVec3(s.value(kKeyMeshColor),  QVector3D(0.85f, 0.85f, 0.9f));
    rs.pointColor = toVec3(s.value(kKeyPointColor), QVector3D(0.2f, 0.8f, 0.3f));
    rs.wireColor  = toVec3(s.value(kKeyWireColor),  QVector3D(0.1f, 0.1f, 0.1f));
    rs.cameraSpeed = s.value(kKeyCameraSpeed, 3.0).toDouble();
    s.endGroup();
    return rs;
}

void SettingsManager::saveRenderSettings(const RenderSettings& rs) const {
    QSettings s;
    s.beginGroup(kGroupRender);
    s.setValue(kKeyShowPoints, rs.showPoints);
    s.setValue(kKeyShowNormals, rs.showNormals);
    s.setValue(kKeyShowMesh,   rs.showMesh);
    s.setValue(kKeyWireframe,  rs.wireframe);
    s.setValue(kKeyPointSize,  rs.pointSize);
    s.setValue(kKeyMeshColor,  toVarList(rs.meshColor));
    s.setValue(kKeyPointColor, toVarList(rs.pointColor));
    s.setValue(kKeyWireColor,  toVarList(rs.wireColor));
    s.setValue(kKeyCameraSpeed, rs.cameraSpeed);
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
