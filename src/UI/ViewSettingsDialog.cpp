#include "ViewSettingsDialog.h"
#include "ui_ViewSettingsDialog.h"
#include "../Settings/ViewSettingsBinder.h"
#include "../Rendering/RenderView.h"

ViewSettingsDialog::ViewSettingsDialog(RenderView* view, QWidget* parent)
    : QDialog(parent), ui(new Ui::ViewSettingsDialog), m_view(view) {
    ui->setupUi(this);

    // Non-modal settings dialog
    setWindowModality(Qt::NonModal);

    // Bind the controls inside the dialog
    m_binder = std::make_unique<ViewSettingsBinder>(
        m_view,
        /*viewSettingsDock*/ nullptr,
        /*viewSettingsAction*/ nullptr,
        ui->chkShowPoints,
        ui->chkShowMesh,
        ui->chkWireframe,
        ui->pointSizeControl,
        ui->swatchPointColor,
        ui->swatchMeshColor,
        ui->swatchWireColor,
        this
    );
    m_binder->initialize();
}

ViewSettingsDialog::~ViewSettingsDialog() {
    delete ui;
}
