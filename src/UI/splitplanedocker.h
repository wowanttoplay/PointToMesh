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
    void onPlaneEdited();
    void onNormalize();
    void onAlignToView();
    void onAlignToAxisX();
    void onALignToAxixY();
    void onAlignToAxisZ();
    void onResetClip();
    void refreshFromView();

private:
    Ui::SplitPlaneDocker *ui;
    RenderView* m_view {nullptr};
};


#endif //POINTTOMESH_SPLITPLANEDOCKER_H