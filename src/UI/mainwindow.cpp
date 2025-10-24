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
#include <functional>

#include "splitplanedocker.h"
#include "../Settings//WindowStateGuard.h"
#include "ViewSettingsDialog.h"
#include "ParameterDialog.h"
#include "../DataProcess/BaseInputParameter.h"



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

    // Ensure reset action exists; disable until a file is imported
    if (auto reset = findChild<QAction*>("actionResetPointCloud")) reset->setEnabled(false);

    // Menu actions
    connect(ui->actionImport, &QAction::triggered, this, [this]{
        const QString path = QFileDialog::getOpenFileName(this, tr("Open point cloud"), QString(), tr("Point clouds (*.xyz *.ply *.off *.pts);;All Files (*.*)"));
        if (!path.isEmpty()) {
            m_controller->importFromFile(path);
            if (auto reset = findChild<QAction*>("actionResetPointCloud")) reset->setEnabled(true);
        }
    });

    connect(ui->actionExport, &QAction::triggered, this, [this]{
        const QString path = QFileDialog::getSaveFileName(this, tr("Export mesh"), QString(), tr("Meshes (*.ply *.obj *.off *.stl);;All Files (*.*)"));
        if (!path.isEmpty()) m_controller->exportMesh(path, /*withNormals=*/true);
    });

    // Optional reconstruct action if present
    // When reconstruction actions are triggered, open a persistent parameter dialog for the selected method.
    ConnectReconstructions();
    ConnectNormalEstimations();
    ConnectMeshTools();

    // Wire reset action to controller
    if (auto reset = findChild<QAction*>("actionResetPointCloud")) {
        connect(reset, &QAction::triggered, this, [this]{ m_controller->resetToOriginal(); });
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


// Helper to centralize ParameterDialog lifecycle and Apply wiring
void MainWindow::openOrCreateParamDialog(QPointer<ParameterDialog>& slot,
                                         std::function<BaseInputParameter*()> factory,
                                         std::function<void(BaseInputParameter*)> onApply) {
    if (!slot) {
        BaseInputParameter* params = factory ? factory() : nullptr;
        slot = new ParameterDialog(params, this);
        if (onApply) {
            connect(slot, &ParameterDialog::applyClicked, this, onApply);
        }
    }
    slot->show();
    slot->raise();
    slot->activateWindow();
}

void MainWindow::ConnectReconstructions() {
    if (auto a = findChild<QAction*>("actionReconstructPoisson")) {
        connect(a, &QAction::triggered, this, [this]{
            openOrCreateParamDialog(
                m_poissonParamDialog,
                [this]() { return new PoissonReconstructionParameter(this); },
                [this](BaseInputParameter* p){
                    if (!m_controller) return;
                    std::unique_ptr<BaseInputParameter> snapshot;
                    if (p) snapshot = p->clone();
                    m_controller->runReconstructionWith(MeshGenerationMethod::POISSON_RECONSTRUCTION, std::move(snapshot));
                }
            );
        });
    }
    if (auto a = findChild<QAction*>("actionReconstructScaleSpace")) {
        connect(a, &QAction::triggered, this, [this]{
            openOrCreateParamDialog(
                m_scaleSpaceParamDialog,
                [this]() { return new ScaleSpaceReconstructionParameter(this); },
                [this](BaseInputParameter* p){
                    if (!m_controller) return;
                    std::unique_ptr<BaseInputParameter> snapshot;
                    if (p) snapshot = p->clone();
                    m_controller->runReconstructionWith(MeshGenerationMethod::SCALE_SPACE_RECONSTRUCTION, std::move(snapshot));
                }
            );
        });
    }
    if (auto a = findChild<QAction*>("actionReconstructAdvancingFront")) {
        connect(a, &QAction::triggered, this, [this]{
            openOrCreateParamDialog(
                m_advancingFrontParamDialog,
                [this]() { return new AdvancingFrontReconstructionParameter(this); },
                [this](BaseInputParameter* p){
                    if (!m_controller) return;
                    std::unique_ptr<BaseInputParameter> snapshot;
                    if (p) snapshot = p->clone();
                    m_controller->runReconstructionWith(MeshGenerationMethod::ADVANCING_FRONT_RECONSTRUCTION, std::move(snapshot));
                }
            );
        });
    }
}

void MainWindow::ConnectNormalEstimations() {
    if (auto a = findChild<QAction*>("actionEstimateNormalsJet")) {
        connect(a, &QAction::triggered, this, [this]{ if (m_controller) m_controller->runNormalEstimation(NormalEstimationMethod::JET_ESTIMATION); });
    }
    if (auto a = findChild<QAction*>("actionEstimateNormalsUniformCentroid")) {
        connect(a, &QAction::triggered, this, [this]{ if (m_controller) m_controller->runNormalEstimation(NormalEstimationMethod::UNIFORM_VOLUME_CENTROID); });
    }
    if (auto a = findChild<QAction*>("actionEstimateNormalsVCM")) {
        connect(a, &QAction::triggered, this, [this]{ if (m_controller) m_controller->runNormalEstimation(NormalEstimationMethod::VCM_ESTIMATION); });
    }
}

void MainWindow::ConnectMeshTools() {
    if (auto a = findChild<QAction*>("actionPostProcessMesh")) {
        connect(a, &QAction::triggered, this, [this]{
            openOrCreateParamDialog(
                m_postProcessParamDialog,
                [this]() { return new MeshPostprocessParameter(this); },
                [this](BaseInputParameter* p){
                    if (!m_controller) return;
                    std::unique_ptr<BaseInputParameter> snapshot;
                    if (p) snapshot = p->clone();
                    m_controller->runPostProcessMesh(std::move(snapshot));
                }
            );
        });
    }
    // Wire Point Cloud tools as well
    if (auto a = findChild<QAction*>("actionVoxelDownsample")) {
        connect(a, &QAction::triggered, this, [this]{
            openOrCreateParamDialog(
                m_voxelDownsampleDialog,
                [this]() { auto* p = new VoxelDownsampleParameter(this); p->cell_size = 0.0; return p; },
                [this](BaseInputParameter* p){ if (m_controller) { auto snapshot = p ? p->clone() : nullptr; m_controller->runDownsampleVoxel(std::move(snapshot)); } }
            );
        });
    }
    if (auto a = findChild<QAction*>("actionFilterAABB")) {
        connect(a, &QAction::triggered, this, [this]{
            openOrCreateParamDialog(
                m_filterAABBDialog,
                [this]() { return new AABBFilterParameter(this); },
                [this](BaseInputParameter* p){ if (m_controller) { auto s = p ? p->clone() : nullptr; m_controller->runFilterAABB(std::move(s)); } }
            );
        });
    }
    if (auto a = findChild<QAction*>("actionFilterSphere")) {
        connect(a, &QAction::triggered, this, [this]{
            openOrCreateParamDialog(
                m_filterSphereDialog,
                [this]() { return new SphereFilterParameter(this); },
                [this](BaseInputParameter* p){ if (m_controller) { auto s = p ? p->clone() : nullptr; m_controller->runFilterSphere(std::move(s)); } }
            );
        });
    }
    if (auto a = findChild<QAction*>("actionFilterSurfaceFromUniformVolume")) {
        connect(a, &QAction::triggered, this, [this]{
            openOrCreateParamDialog(
                m_uniformSurfaceDialog,
                [this]() { return new UniformVolumeSurfaceFilterParameter(this); },
                [this](BaseInputParameter* p){ if (m_controller) { auto s = p ? p->clone() : nullptr; m_controller->runFilterUniformVolumeSurface(std::move(s)); } }
            );
        });
    }
}
