//
// Created by 范杰 on 2025/10/16.
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
#include "PointSizeControlWidget.h"
#include <QCheckBox>
#include <QAction>
#include <QDockWidget>
#include "../Presentation/SettingsManager.h"
#include "../Presentation/ViewSettingsBinder.h"
#include "../Presentation/WindowStateGuard.h"

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
    if (auto a = findChild<QAction*>("actionReconstruct")) {
        connect(a, &QAction::triggered, this, [this]{ m_controller->runReconstruction(); });
    }

    // RAII restore/sync of window geometry and dock layout
    m_windowStateGuard = std::make_unique<WindowStateGuard>(this);

    // Bind all view-related settings and controls in one place
    m_viewSettingsBinder = std::make_unique<ViewSettingsBinder>(
        m_renderView,
        ui->viewSettingsDock,
        ui->actionViewSettings,
        findChild<QCheckBox*>("chkShowPoints"),
        findChild<QCheckBox*>("chkShowMesh"),
        findChild<QCheckBox*>("chkWireframe"),
        findChild<PointSizeControlWidget*>("pointSizeControl"),
        this
    );
    m_viewSettingsBinder->initialize();
}

MainWindow::~MainWindow() = default;
