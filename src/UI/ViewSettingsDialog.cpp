#include "ViewSettingsDialog.h"
#include "ui_ViewSettingsDialog.h"
#include "../Settings/SettingsManager.h"
#include "../Rendering/RenderView.h"
#include "CustomUI/ScalarControlWidget.h"
#include "CustomUI/ColorSwatch.h"
#include <QAction>
#include <QCheckBox>
#include <QColor>
#include <algorithm>
#include <cmath>

namespace {
static inline QColor toColor(const QVector3D& v) {
    return QColor::fromRgbF(std::clamp(v.x(), 0.0f, 1.0f), std::clamp(v.y(), 0.0f, 1.0f), std::clamp(v.z(), 0.0f, 1.0f));
}
static inline QVector3D toVec3(const QColor& c) {
    return QVector3D(c.redF(), c.greenF(), c.blueF());
}
}

ViewSettingsDialog::ViewSettingsDialog(RenderView* view, QWidget* parent)
    : QDockWidget(parent), ui(new Ui::ViewSettingsDialog), m_view(view) {
    ui->setupUi(this);

    // Dock widget appearance/behavior
    setWindowTitle(tr("View Settings"));
    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    setAllowedAreas(Qt::AllDockWidgetAreas);

    // Load current render settings
    RenderSettings rs = SettingsManager::instance().loadRenderSettings();

    // Show Points
    if (ui->chkShowPoints) {
        ui->chkShowPoints->setChecked(rs.showPoints);
        if (m_view) m_view->setShowPoints(ui->chkShowPoints->isChecked());
        connect(ui->chkShowPoints, &QCheckBox::toggled, this, [this](bool on){
            if (m_view) m_view->setShowPoints(on);
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.showPoints = on;
            SettingsManager::instance().saveRenderSettings(cur);
        });
    }

    // Show Normals
    if (ui->chkShowNormals) {
        ui->chkShowNormals->setChecked(rs.showNormals);
        connect(ui->chkShowNormals, &QCheckBox::toggled, this, [this](bool on){
            if (m_view) m_view->setShowNormals(on);
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.showNormals = on;
            SettingsManager::instance().saveRenderSettings(cur);
        });
    }

    // Show Mesh
    if (ui->chkShowMesh) {
        ui->chkShowMesh->setChecked(rs.showMesh);
        if (m_view) m_view->setShowMesh(ui->chkShowMesh->isChecked());
        connect(ui->chkShowMesh, &QCheckBox::toggled, this, [this](bool on){
            if (m_view) m_view->setShowMesh(on);
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.showMesh = on;
            SettingsManager::instance().saveRenderSettings(cur);
        });
    }

    // Wireframe
    if (ui->chkWireframe) {
        ui->chkWireframe->setChecked(rs.wireframe);
        if (m_view) m_view->setWireframe(ui->chkWireframe->isChecked());
        connect(ui->chkWireframe, &QCheckBox::toggled, this, [this](bool on){
            if (m_view) m_view->setWireframe(on);
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.wireframe = on;
            SettingsManager::instance().saveRenderSettings(cur);
        });
    }

    // Point size control
    if (ui->pointSizeControl) {
        ui->pointSizeControl->setLabelText(tr("Point Size:"));
        ui->pointSizeControl->setRange(1.0, 20.0, 0);
        ui->pointSizeControl->setValue(static_cast<double>(rs.pointSize));
        if (m_view) m_view->setPointSize(static_cast<float>(rs.pointSize));
        connect(ui->pointSizeControl, &ScalarControlWidget::valueChanged, this, [this](double v){
            const int vi = static_cast<int>(std::lround(v));
            if (m_view) m_view->setPointSize(static_cast<float>(vi));
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.pointSize = vi;
            SettingsManager::instance().saveRenderSettings(cur);
        });
    }

    // Color swatches
    if (ui->swatchPointColor) {
        ui->swatchPointColor->setLabel(tr("Point Color"));
        ui->swatchPointColor->setDialogTitle(tr("Choose Point Color"));
        ui->swatchPointColor->setColor(toColor(rs.pointColor));
        if (m_view) m_view->setPointColor(rs.pointColor);
        connect(ui->swatchPointColor, &ColorSwatch::colorChanged, this, [this](const QColor& c){
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.pointColor = toVec3(c);
            SettingsManager::instance().saveRenderSettings(cur);
            if (m_view) m_view->setPointColor(cur.pointColor);
        });
    }

    if (ui->swatchMeshColor) {
        ui->swatchMeshColor->setLabel(tr("Mesh Color"));
        ui->swatchMeshColor->setDialogTitle(tr("Choose Mesh Color"));
        ui->swatchMeshColor->setColor(toColor(rs.meshColor));
        if (m_view) m_view->setMeshColor(rs.meshColor);
        connect(ui->swatchMeshColor, &ColorSwatch::colorChanged, this, [this](const QColor& c){
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.meshColor = toVec3(c);
            SettingsManager::instance().saveRenderSettings(cur);
            if (m_view) m_view->setMeshColor(cur.meshColor);
        });
    }

    if (ui->swatchWireColor) {
        ui->swatchWireColor->setLabel(tr("Wireframe Color"));
        ui->swatchWireColor->setDialogTitle(tr("Choose Wireframe Color"));
        ui->swatchWireColor->setColor(toColor(rs.wireColor));
        if (m_view) m_view->setWireColor(rs.wireColor);
        connect(ui->swatchWireColor, &ColorSwatch::colorChanged, this, [this](const QColor& c){
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.wireColor = toVec3(c);
            SettingsManager::instance().saveRenderSettings(cur);
            if (m_view) m_view->setWireColor(cur.wireColor);
        });
    }

    // Camera speed
    if (ui->cameraSpeedControl) {
        ui->cameraSpeedControl->setLabelText(tr("Camera Speed:"));
        ui->cameraSpeedControl->setRange(0.01, 20.0, 1);
        ui->cameraSpeedControl->setValue(rs.cameraSpeed);
        if (m_view) m_view->setCameraSpeed(static_cast<float>(rs.cameraSpeed));
        connect(ui->cameraSpeedControl, &ScalarControlWidget::valueChanged, this, [this](double v){
            if (m_view) m_view->setCameraSpeed(static_cast<float>(v));
            RenderSettings cur = SettingsManager::instance().loadRenderSettings();
            cur.cameraSpeed = static_cast<float>(v);
            SettingsManager::instance().saveRenderSettings(cur);
        });
    }
}

ViewSettingsDialog::~ViewSettingsDialog() {
    delete ui;
}
