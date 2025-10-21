//
// Created by 范杰 on 2025/10/20.
//

// You may need to build the project (run Qt uic code generator) to get "ui_SplitPlaneDocker.h" resolved

#include "splitplanedocker.h"
#include "ui_SplitPlaneDocker.h"
#include "Rendering/RenderView.h"


SplitPlaneDocker::SplitPlaneDocker(QWidget *parent, RenderView *view) : QDockWidget(parent),
                                                                        ui(new Ui::SplitPlaneDocker), m_view(view) {
    ui->setupUi(this);

    BindUIWithRenderView();
}

SplitPlaneDocker::~SplitPlaneDocker() {
    delete ui;
}

void SplitPlaneDocker::BindUIWithRenderView() {
    connect(ui->SplitPlaneEnable, &QCheckBox::toggled, m_view, &RenderView::setClipEnabled);
    connect(ui->plane_nx, &QDoubleSpinBox::valueChanged, this, &SplitPlaneDocker::onPlaneEdited);
    connect(ui->plane_ny, &QDoubleSpinBox::valueChanged, this, &SplitPlaneDocker::onPlaneEdited);
    connect(ui->plane_nz, &QDoubleSpinBox::valueChanged, this, &SplitPlaneDocker::onPlaneEdited);
    connect(ui->plane_d, &QDoubleSpinBox::valueChanged, this, &SplitPlaneDocker::onPlaneEdited);

    connect(ui->btnNormaliza, &QPushButton::clicked, this, &SplitPlaneDocker::onNormalize);
    connect(ui->btnView, &QPushButton::clicked, this, &SplitPlaneDocker::onAlignToView);
    connect(ui->btnRest, &QPushButton::clicked, this, &SplitPlaneDocker::onResetClip);
    connect(ui->btnX, &QPushButton::clicked, this, &SplitPlaneDocker::onAlignToAxisX);
    connect(ui->btnY, &QPushButton::clicked, this, &SplitPlaneDocker::onALignToAxixY);
    connect(ui->btnZ, &QPushButton::clicked, this, &SplitPlaneDocker::onAlignToAxisZ);

    refreshFromView();
}

void SplitPlaneDocker::onPlaneEdited() {
    const double nx = ui->plane_nx->value();
    const double ny = ui->plane_ny->value();
    const double nz = ui->plane_nz->value();
    const double d = ui->plane_d->value();
    m_view->setClipPlane(QVector4D(float(nx), float(ny) ,float(nz), float(d)));
}

void SplitPlaneDocker::onNormalize() {
    double nx = ui->plane_nx->value(), ny = ui->plane_ny->value(), nz = ui->plane_nz->value();
    const double len = std::sqrt(nx*nx + ny*ny + nz*nz);
    if (len > 1e-8) {
        nx /= len; ny /= len; nz /= len;
        ui->plane_nx->setValue(nx);
        ui->plane_ny->setValue(ny);
        ui->plane_nz->setValue(nz);
        onPlaneEdited();
    }
}

void SplitPlaneDocker::onAlignToView() {
    if (!m_view) return;
    m_view->alignClipPlaneToCameraThroughSceneCenter();
    refreshFromView();
}

void SplitPlaneDocker::onAlignToAxisX() {
    ui->plane_nx->setValue(1.0); ui->plane_ny->setValue(0.0); ui->plane_nz->setValue(0.0);
    onPlaneEdited();
}

void SplitPlaneDocker::onALignToAxixY() {
    ui->plane_nx->setValue(0.0); ui->plane_ny->setValue(1.0); ui->plane_nz->setValue(0.0);
    onPlaneEdited();
}

void SplitPlaneDocker::onAlignToAxisZ() {
    ui->plane_nx->setValue(0.0); ui->plane_ny->setValue(0.0); ui->plane_nz->setValue(1.0);
    onPlaneEdited();
}

void SplitPlaneDocker::onResetClip() {
    // n=(0,0,1), d=0 =>  z=0
    ui->plane_nx->setValue(0.0); ui->plane_ny->setValue(0.0); ui->plane_nz->setValue(1.0);
    ui->plane_d->setValue(0.0);
    onPlaneEdited();
}

void SplitPlaneDocker::refreshFromView() {
    if (!m_view) return;
    ui->SplitPlaneEnable->setChecked(m_view->isEnabled());
    const auto p = m_view->clipPlane();
    ui->plane_nx->setValue(p.x());
    ui->plane_ny->setValue(p.y());
    ui->plane_nz->setValue(p.z());
    ui->plane_d->setValue(p.w());
}
