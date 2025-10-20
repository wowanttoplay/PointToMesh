#include "ViewSettingsDialog.h"
#include "ui_ViewSettingsDialog.h"
#include "../Settings/ViewSettingsBinder.h"
#include "../Rendering/RenderView.h"

ViewSettingsDialog::ViewSettingsDialog(RenderView* view, QWidget* parent)
    : QDialog(parent), ui(new Ui::ViewSettingsDialog), m_view(view) {
    ui->setupUi(this);

    // Non-modal settings dialog
    setWindowModality(Qt::NonModal);
    // Keep this dialog always on top of its parent MainWindow
    // Qt::Tool makes it a tool window that stays above its parent and minimizes together.
    setWindowFlag(Qt::Tool, true);
    // If you want it to float above all app windows (not just the parent), also enable:
    // setWindowFlag(Qt::WindowStaysOnTopHint, true);

    // Bind the controls inside the dialog
    m_binder = std::make_unique<ViewSettingsBinder>(
        m_view,
        /*viewSettingsDock*/ nullptr,
        /*viewSettingsAction*/ nullptr,
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
