//
// Created by Fan Jie on 2025/10/16.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "mainwindow.h"
#include "ui_MainWindow.h"
#include "CustomUI/LogPanel.h"
#include "../Rendering/RenderView.h"
#include "../Presentation/PointCloudController.h"
#include "../DataProcess/CGALPointCloudProcessor.h"
#include <QFileDialog>
#include <QtGlobal>
#include <QAction>
#include <QDockWidget>

#include "splitplanedocker.h"
#include "../Settings//WindowStateGuard.h"
#include "ViewSettingsDialog.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(std::make_unique<Ui::MainWindow>()) {
    ui->setupUi(this);

    ConnectLogView();
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

    // Create View Settings dock before restoring window state, so visibility/layout can be restored
    ConnectViewSettings();
    ConnectSplitPlaneControls();

    // RAII restore/sync of window geometry and dock layout
    m_windowStateGuard = std::make_unique<WindowStateGuard>(this);
}

MainWindow::~MainWindow() = default;

void MainWindow::ConnectViewSettings() {
    // Create the dock if not existing yet
    if (!m_viewSettingsDialog) {
        m_viewSettingsDialog = new ViewSettingsDialog(this, m_renderView);
        addDockWidget(Qt::RightDockWidgetArea, m_viewSettingsDialog);

        // Prefer Qt's built-in toggle action for show/hide
        if (ui->menuView) {
            if (ui->actionViewSettings) {
                ui->menuView->removeAction(ui->actionViewSettings);
                ui->actionViewSettings->setVisible(false);
            }
            ui->menuView->addAction(m_viewSettingsDialog->toggleViewAction());
        }
    }
}

void MainWindow::ConnectSplitPlaneControls() {
    if (!m_splitPlaneDocker) {
        m_splitPlaneDocker = new SplitPlaneDocker(this, m_renderView);
        addDockWidget(Qt::LeftDockWidgetArea, m_splitPlaneDocker);

        if (ui->menuView) {
            if (ui->actionSplitPlaneSettings) {
                ui->menuView->removeAction(ui->actionSplitPlaneSettings);
                ui->actionSplitPlaneSettings->setVisible(false);
            }
            ui->menuView->addAction(m_splitPlaneDocker->toggleViewAction());
        }
    }
}

void MainWindow::ConnectLogView() {
    if (!m_logPanel) {
        m_logPanel = new LogPanel("Output", this);
        addDockWidget(Qt::BottomDockWidgetArea, m_logPanel);

        if (ui->menuView) {
            if (ui->actionOutputView) {
                ui->menuView->removeAction(ui->actionOutputView);
                ui->actionOutputView->setVisible(false);
            }
            ui->menuView->addAction(m_logPanel->toggleViewAction());
        }
    }
}
