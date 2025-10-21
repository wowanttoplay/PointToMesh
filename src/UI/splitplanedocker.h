//
// Created by 范杰 on 2025/10/20.
//

#ifndef POINTTOMESH_SPLITPLANEDOCKER_H
#define POINTTOMESH_SPLITPLANEDOCKER_H

#include <QDockWidget>


class RenderView;
QT_BEGIN_NAMESPACE

namespace Ui {
    class SplitPlaneDocker;
}

QT_END_NAMESPACE

class SplitPlaneDocker : public QDockWidget {
    Q_OBJECT

public:
    explicit SplitPlaneDocker(QWidget *parent = nullptr, RenderView* view = nullptr);

    ~SplitPlaneDocker() override;

private:
    void BindUIWithRenderView();

private slots:
    // When rotation or location values change, recompute plane (normal,d) and send to RenderView
    void onTransformEdited();
    void onAlignToAxisX();
    void onALignToAxixY();
    void onAlignToAxisZ();
    void onResetClip();

private:
    Ui::SplitPlaneDocker *ui;
    RenderView* m_view {nullptr};
};


#endif //POINTTOMESH_SPLITPLANEDOCKER_H