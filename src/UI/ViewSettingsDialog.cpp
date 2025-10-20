#include "ViewSettingsDialog.h"
#include "ui_ViewSettingsDialog.h"
#include "../Settings/ViewSettingsBinder.h"
#include "../Rendering/RenderView.h"
#include <QAction>

ViewSettingsDialog::ViewSettingsDialog(RenderView* view, QAction* toggleAction, QWidget* parent)
    : QDockWidget(parent), ui(new Ui::ViewSettingsDialog), m_view(view) {
    ui->setupUi(this);

    // Dock widget appearance/behavior
    setWindowTitle(tr("View Settings"));
    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    // Bind the controls inside the dock
    m_binder = std::make_unique<ViewSettingsBinder>(
        m_view,
        /*viewSettingsDock*/ this,
        /*viewSettingsAction*/ toggleAction,
        ui->chkShowPoints,
        ui->chkShowNormals,
        ui->chkShowMesh,
        ui->chkWireframe,
        ui->pointSizeControl,
        ui->swatchPointColor,
        ui->swatchMeshColor,
        ui->swatchWireColor,
        ui->cameraSpeedControl,
        this
    );
    m_binder->initialize();
}

ViewSettingsDialog::~ViewSettingsDialog() {
    delete ui;
}
