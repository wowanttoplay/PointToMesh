#include "ViewSettingsBinder.h"
#include <QDockWidget>
#include <QAction>
#include <QCheckBox>
#include <QPushButton>
#include <QColorDialog>
#include <QColor>
#include <algorithm>
#include "SettingsManager.h"
#include "../Rendering/RenderView.h"
#include "../UI/PointSizeControlWidget.h"

namespace {
static inline QColor toColor(const QVector3D& v) {
    return QColor::fromRgbF(std::clamp(v.x(), 0.0f, 1.0f), std::clamp(v.y(), 0.0f, 1.0f), std::clamp(v.z(), 0.0f, 1.0f));
}
static inline QVector3D toVec3(const QColor& c) {
    return QVector3D(c.redF(), c.greenF(), c.blueF());
}
static inline void setButtonSwatch(QPushButton* btn, const QColor& c) {
    if (!btn) return;
    btn->setText(QString("%1, %2, %3").arg(int(c.redF()*255)).arg(int(c.greenF()*255)).arg(int(c.blueF()*255)));
    btn->setStyleSheet(QString("QPushButton{ background-color: %1; color: %2; }")
                       .arg(c.name())
                       .arg(c.lightnessF() < 0.5 ? "white" : "black"));
}
}

ViewSettingsBinder::ViewSettingsBinder(RenderView* view,
                                       QDockWidget* viewSettingsDock,
                                       QAction* viewSettingsAction,
                                       QCheckBox* chkShowPoints,
                                       QCheckBox* chkShowMesh,
                                       QCheckBox* chkWireframe,
                                       PointSizeControlWidget* pointSizeWidget,
                                       QPushButton* btnPointColor,
                                       QPushButton* btnMeshColor,
                                       QObject* parent)
    : QObject(parent), m_view(view), m_dock(viewSettingsDock), m_action(viewSettingsAction),
      m_chkPoints(chkShowPoints), m_chkMesh(chkShowMesh), m_chkWire(chkWireframe), m_psc(pointSizeWidget),
      m_btnPointColor(btnPointColor), m_btnMeshColor(btnMeshColor) {}

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

    // Colors
    if (m_btnPointColor && m_view) {
        setButtonSwatch(m_btnPointColor, toColor(rs.pointColor));
        QObject::connect(m_btnPointColor, &QPushButton::clicked, this, [this](){
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            const QColor start = toColor(cur.pointColor);
            QColor chosen = QColorDialog::getColor(start, m_btnPointColor, tr("Choose Point Color"), QColorDialog::ShowAlphaChannel);
            if (!chosen.isValid()) return;
            setButtonSwatch(m_btnPointColor, chosen);
            cur.pointColor = toVec3(chosen);
            SettingsManager::instance().saveRenderSettings(cur);
            if (m_view) m_view->setPointColor(cur.pointColor);
        });
        // Apply initially to view
        m_view->setPointColor(rs.pointColor);
    }

    if (m_btnMeshColor && m_view) {
        setButtonSwatch(m_btnMeshColor, toColor(rs.meshColor));
        QObject::connect(m_btnMeshColor, &QPushButton::clicked, this, [this](){
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            const QColor start = toColor(cur.meshColor);
            QColor chosen = QColorDialog::getColor(start, m_btnMeshColor, tr("Choose Mesh Color"), QColorDialog::ShowAlphaChannel);
            if (!chosen.isValid()) return;
            setButtonSwatch(m_btnMeshColor, chosen);
            cur.meshColor = toVec3(chosen);
            SettingsManager::instance().saveRenderSettings(cur);
            if (m_view) m_view->setMeshColor(cur.meshColor);
        });
        // Apply initially to view
        m_view->setMeshColor(rs.meshColor);
    }
}
