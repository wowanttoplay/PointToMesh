#include "ViewSettingsBinder.h"
#include <QDockWidget>
#include <QAction>
#include <QCheckBox>
#include "SettingsManager.h"
#include "../Rendering/RenderView.h"
#include "../UI/PointSizeControlWidget.h"
#include "../UI/ColorSwatch.h"
#include <algorithm>
#include <QColor>

namespace {
static inline QColor toColor(const QVector3D& v) {
    return QColor::fromRgbF(std::clamp(v.x(), 0.0f, 1.0f), std::clamp(v.y(), 0.0f, 1.0f), std::clamp(v.z(), 0.0f, 1.0f));
}
static inline QVector3D toVec3(const QColor& c) {
    return QVector3D(c.redF(), c.greenF(), c.blueF());
}
}

ViewSettingsBinder::ViewSettingsBinder(RenderView* view,
                                       QDockWidget* viewSettingsDock,
                                       QAction* viewSettingsAction,
                                       QCheckBox* chkShowPoints,
                                       QCheckBox* chkShowMesh,
                                       QCheckBox* chkWireframe,
                                       PointSizeControlWidget* pointSizeWidget,
                                       ColorSwatch* swatchPointColor,
                                       ColorSwatch* swatchMeshColor,
                                       QObject* parent)
    : QObject(parent), m_view(view), m_dock(viewSettingsDock), m_action(viewSettingsAction),
      m_chkPoints(chkShowPoints), m_chkMesh(chkShowMesh), m_chkWire(chkWireframe), m_psc(pointSizeWidget),
      m_swatchPoint(swatchPointColor), m_swatchMesh(swatchMeshColor) {}

void ViewSettingsBinder::initialize() {
    // Sync action <-> dock visibility
    if (m_action && m_dock) {
        m_action->setChecked(m_dock->isVisible());
        QObject::connect(m_action, &QAction::toggled, m_dock, &QDockWidget::setVisible);
        QObject::connect(m_dock, &QDockWidget::visibilityChanged, m_action, &QAction::setChecked);
    }

    // Load render settings
    RenderSettings rs = SettingsManager::instance().loadRenderSettings();

    // Apply to UI and renderer, then wire persistence
    if (m_chkPoints && m_view) {
        m_chkPoints->setChecked(rs.showPoints);
        m_view->setShowPoints(m_chkPoints->isChecked());
        QObject::connect(m_chkPoints, &QCheckBox::toggled, this, [this](bool on){
            if (m_view) m_view->setShowPoints(on);
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.showPoints = on;
            SettingsManager::instance().saveRenderSettings(cur);
        });
    }
    if (m_chkMesh && m_view) {
        m_chkMesh->setChecked(rs.showMesh);
        m_view->setShowMesh(m_chkMesh->isChecked());
        QObject::connect(m_chkMesh, &QCheckBox::toggled, this, [this](bool on){
            if (m_view) m_view->setShowMesh(on);
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.showMesh = on;
            SettingsManager::instance().saveRenderSettings(cur);
        });
    }
    if (m_chkWire && m_view) {
        m_chkWire->setChecked(rs.wireframe);
        m_view->setWireframe(m_chkWire->isChecked());
        QObject::connect(m_chkWire, &QCheckBox::toggled, this, [this](bool on){
            if (m_view) m_view->setWireframe(on);
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.wireframe = on;
            SettingsManager::instance().saveRenderSettings(cur);
        });
    }

    if (m_psc && m_view) {
        // Initialize from settings and then wire changes
        const int initial = rs.pointSize;
        m_psc->setValue(initial);
        m_view->setPointSize(static_cast<float>(initial));
        QObject::connect(m_psc, &PointSizeControlWidget::valueChanged, this, [this](int v){
            if (m_view) m_view->setPointSize(static_cast<float>(v));
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.pointSize = v;
            SettingsManager::instance().saveRenderSettings(cur);
        });
    }

    // Colors via ColorSwatch widgets
    if (m_swatchPoint && m_view) {
        m_swatchPoint->setLabel(tr("Point Color"));
        m_swatchPoint->setDialogTitle(tr("Choose Point Color"));
        m_swatchPoint->setColor(toColor(rs.pointColor));
        QObject::connect(m_swatchPoint, &ColorSwatch::colorChanged, this, [this](const QColor& c){
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.pointColor = toVec3(c);
            SettingsManager::instance().saveRenderSettings(cur);
            if (m_view) m_view->setPointColor(cur.pointColor);
        });
        // Apply initially to view
        m_view->setPointColor(rs.pointColor);
    }

    if (m_swatchMesh && m_view) {
        m_swatchMesh->setLabel(tr("Mesh Color"));
        m_swatchMesh->setDialogTitle(tr("Choose Mesh Color"));
        m_swatchMesh->setColor(toColor(rs.meshColor));
        QObject::connect(m_swatchMesh, &ColorSwatch::colorChanged, this, [this](const QColor& c){
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.meshColor = toVec3(c);
            SettingsManager::instance().saveRenderSettings(cur);
            if (m_view) m_view->setMeshColor(cur.meshColor);
        });
        // Apply initially to view
        m_view->setMeshColor(rs.meshColor);
    }
}
