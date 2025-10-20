#pragma once
#include <QCheckBox>
#include <QObject>

class QDockWidget;
class QAction;
class QCheckBox;
class ScalarControlWidget;
class RenderView;
class ColorSwatch;

class ViewSettingsBinder : public QObject {
    Q_OBJECT
public:
    ViewSettingsBinder(RenderView* view,
                       QDockWidget* viewSettingsDock,
                       QAction* viewSettingsAction,
                       QCheckBox* chkShowPoints,
                       QCheckBox* chkShowNormals,
                       QCheckBox* chkShowMesh,
                       QCheckBox* chkWireframe,
                       ScalarControlWidget* pointSizeWidget,
                       ColorSwatch* swatchPointColor,
                       ColorSwatch* swatchMeshColor,
                       ColorSwatch* swatchWireColor,
                       ScalarControlWidget* cameraSpeedWidget,
                       QObject* parent = nullptr);

    void initialize();

private:
    RenderView* m_view {nullptr};
    QDockWidget* m_dock {nullptr};
    QAction* m_action {nullptr};
    QCheckBox* m_chkPoints {nullptr};
    QCheckBox* m_chkNormals {nullptr};
    QCheckBox* m_chkMesh {nullptr};
    QCheckBox* m_chkWire {nullptr};
    ScalarControlWidget* m_psc {nullptr};
    ColorSwatch* m_swatchPoint {nullptr};
    ColorSwatch* m_swatchMesh {nullptr};
    ColorSwatch* m_swatchWire {nullptr};
    ScalarControlWidget* m_cameraSpeedCtrl {nullptr};
};