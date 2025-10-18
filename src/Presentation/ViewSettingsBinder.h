#pragma once
#include <QObject>

class QDockWidget;
class QAction;
class QCheckBox;
class QPushButton;
class PointSizeControlWidget;
class RenderView;

class ViewSettingsBinder : public QObject {
    Q_OBJECT
public:
    ViewSettingsBinder(RenderView* view,
                       QDockWidget* viewSettingsDock,
                       QAction* viewSettingsAction,
                       QCheckBox* chkShowPoints,
                       QCheckBox* chkShowMesh,
                       QCheckBox* chkWireframe,
                       PointSizeControlWidget* pointSizeWidget,
                       QPushButton* btnPointColor,
                       QPushButton* btnMeshColor,
                       QObject* parent = nullptr);

    void initialize();

private:
    RenderView* m_view {nullptr};
    QDockWidget* m_dock {nullptr};
    QAction* m_action {nullptr};
    QCheckBox* m_chkPoints {nullptr};
    QCheckBox* m_chkMesh {nullptr};
    QCheckBox* m_chkWire {nullptr};
    PointSizeControlWidget* m_psc {nullptr};
    QPushButton* m_btnPointColor {nullptr};
    QPushButton* m_btnMeshColor {nullptr};
};