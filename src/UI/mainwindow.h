//
// Created by Fan Jie on 2025/10/16.
//

#ifndef POINTTOMESH_MAINWINDOW_H
#define POINTTOMESH_MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include <memory>

class SplitPlaneDocker;
class QCloseEvent;

class LogPanel; // forward declaration
class RenderView; // forward declaration
class PointCloudController; // forward declaration
class WindowStateGuard; // forward declaration
class ViewSettingsDialog; // forward declaration
class ParameterDialog; // forward declaration

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    void ConnectReconstructions();
    void ConnectNormalEstimations();

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QPointer<LogPanel> m_logPanel {nullptr};
    QPointer<RenderView> m_renderView {nullptr};
    QPointer<PointCloudController> m_controller {nullptr};
    std::unique_ptr<WindowStateGuard> m_windowStateGuard; // RAII for geometry/state
    QPointer<ViewSettingsDialog> m_viewSettingsDialog {nullptr}; // Separate dialog for view settings
    QPointer<SplitPlaneDocker> m_splitPlaneDocker {nullptr}; // Dock widget for split plane controls
    // Keep parameter dialogs alive so they remain open until user closes them
    QPointer<ParameterDialog> m_poissonParamDialog {nullptr};
    QPointer<ParameterDialog> m_scaleSpaceParamDialog {nullptr};
    QPointer<ParameterDialog> m_advancingFrontParamDialog {nullptr};
private:
    void ConnectViewSettings();
    void ConnectSplitPlaneControls();
    void ConnectLogView();
};


#endif //POINTTOMESH_MAINWINDOW_H