//
// Created by Fan Jie on 2025/10/16.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "mainwindow.h"
#include "ui_MainWindow.h"
#include "LogPanel.h"
#include "../Rendering/RenderView.h"
#include "../Presentation/PointCloudController.h"
#include "../DataProcess/CGALPointCloudProcessor.h"
#include <QFileDialog>
#include <QtGlobal>
#include <QAction>
#include <QDockWidget>
#include "../Presentation/WindowStateGuard.h"
#include "ViewSettingsDialog.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(std::make_unique<Ui::MainWindow>()) {
    ui->setupUi(this);

    // Replace dock content with LogPanel so logging is encapsulated
    m_logPanel = new LogPanel(this);
    ui->dockWidget->setWidget(m_logPanel);

    // Replace central widget with our RenderView
    m_renderView = new RenderView(this);
    if (!m_renderView) {
        qFatal("Failed to create RenderView. Exiting.");
    }
    setCentralWidget(m_renderView);

    // Create controller with CGAL backend
    m_controller = new PointCloudController(std::make_unique<CGALPointCloudProcessor>(), this);

    // Wire controller to UI components
    connect(m_controller, &PointCloudController::logMessage, m_logPanel, &LogPanel::appendLog);
    connect(m_controller, &PointCloudController::pointCloudUpdated, m_renderView, &RenderView::setPointCloud);
    connect(m_controller, &PointCloudController::meshUpdated, m_renderView, &RenderView::setMesh);

    // Menu actions
    connect(ui->actionImport, &QAction::triggered, this, [this]{
        const QString path = QFileDialog::getOpenFileName(this, tr("Open point cloud"), QString(), tr("Point clouds (*.xyz *.ply *.off *.pts);;All Files (*.*)"));
        if (!path.isEmpty()) m_controller->importFromFile(path);
    });

    connect(ui->actionExport, &QAction::triggered, this, [this]{
        const QString path = QFileDialog::getSaveFileName(this, tr("Export mesh"), QString(), tr("Meshes (*.ply *.obj *.off *.stl);;All Files (*.*)"));
        if (!path.isEmpty()) m_controller->exportMesh(path, /*withNormals=*/true);
    });

    // Optional reconstruct action if present
    if (auto a = findChild<QAction*>("actionReconstructPoisson")) {
        connect(a, &QAction::triggered, this, [this]{ m_controller->runReconstructionWith(MeshGenerationMethod::POISSON_RECONSTRUCTION); });
    }
    if (auto a = findChild<QAction*>("actionReconstructScaleSpace")) {
        connect(a, &QAction::triggered, this, [this]{ m_controller->runReconstructionWith(MeshGenerationMethod::SCALE_SPACE_RECONSTRUCTION); });
    }
    if (auto a = findChild<QAction*>("actionReconstructAdvancingFront")) {
        connect(a, &QAction::triggered, this, [this]{ m_controller->runReconstructionWith(MeshGenerationMethod::ADVANCING_FRONT_RECONSTRUCTION); });
    }

    // RAII restore/sync of window geometry and dock layout
    m_windowStateGuard = std::make_unique<WindowStateGuard>(this);

    // View Settings as separate dialog
    ConnectViewSettings();
}

MainWindow::~MainWindow() = default;

void MainWindow::ConnectViewSettings() {
    if (ui->actionViewSettings) {
        ui->actionViewSettings->setChecked(false);
        connect(ui->actionViewSettings, &QAction::toggled, this, [this](bool on){
            if (!m_viewSettingsDialog) {
                m_viewSettingsDialog = new ViewSettingsDialog(m_renderView, this);
                // When dialog closes, uncheck the action
                connect(m_viewSettingsDialog, &QDialog::finished, this, [this](int){
                    if (ui && ui->actionViewSettings) ui->actionViewSettings->setChecked(false);
                });
            }
            if (on) {
                m_viewSettingsDialog->show();
                m_viewSettingsDialog->raise();
                m_viewSettingsDialog->activateWindow();
            } else if (m_viewSettingsDialog) {
                m_viewSettingsDialog->hide();
            }
        });
    }
}
