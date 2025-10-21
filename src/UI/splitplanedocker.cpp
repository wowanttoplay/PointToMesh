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
    if (m_view) {
        connect(ui->SplitPlaneEnable, &QCheckBox::toggled, m_view, &RenderView::setClipEnabled);
    } else {
        // No render view attached: disable the enable checkbox to avoid null receiver connects
        ui->SplitPlaneEnable->setEnabled(false);
    }

    // Rotation and location controls -> unified transform edited handler
    connect(ui->rotation_x, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SplitPlaneDocker::onTransformEdited);
    connect(ui->rotation_y, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SplitPlaneDocker::onTransformEdited);
    connect(ui->rotation_z, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SplitPlaneDocker::onTransformEdited);
    connect(ui->loc_x, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SplitPlaneDocker::onTransformEdited);
    connect(ui->loc_y, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SplitPlaneDocker::onTransformEdited);
    connect(ui->loc_z, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SplitPlaneDocker::onTransformEdited);

    connect(ui->btnRest, &QPushButton::clicked, this, &SplitPlaneDocker::onResetClip);
    connect(ui->btnX, &QPushButton::clicked, this, &SplitPlaneDocker::onAlignToAxisX);
    connect(ui->btnY, &QPushButton::clicked, this, &SplitPlaneDocker::onALignToAxixY);
    connect(ui->btnZ, &QPushButton::clicked, this, &SplitPlaneDocker::onAlignToAxisZ);

    // Initialize UI to a sensible default plane
    onResetClip();
}

// Helper: convert degrees to radians
static inline float deg2rad(float d) { return d * (3.14159265358979323846f / 180.0f); }

void SplitPlaneDocker::onTransformEdited() {
    if (!m_view) return;
    // Read Euler rotations (degrees) and location
    const float rx = float(ui->rotation_x->value());
    const float ry = float(ui->rotation_y->value());
    const float rz = float(ui->rotation_z->value());
    const QVector3D loc(float(ui->loc_x->value()), float(ui->loc_y->value()), float(ui->loc_z->value()));

    // Build rotation matrix: apply rotations in X then Y then Z (roll, pitch, yaw)
    const float sx = std::sin(deg2rad(rx)); const float cx = std::cos(deg2rad(rx));
    const float sy = std::sin(deg2rad(ry)); const float cy = std::cos(deg2rad(ry));
    const float sz = std::sin(deg2rad(rz)); const float cz = std::cos(deg2rad(rz));

    // Precompute terms
    const float r11 = cz*cy;
    const float r21 = sz*cy;
    const float r31 = -sy;

    const float r12 = cz*sy*sx - sz*cx;
    const float r22 = sz*sy*sx + cz*cx;
    const float r32 = cy*sx;

    const float r13 = cz*sy*cx + sz*sx;
    const float r23 = sz*sy*cx - cz*sx;
    const float r33 = cy*cx;

    // The third column corresponds to R * (0,0,1) = (r13, r23, r33)
    QVector3D normal(r13, r23, r33);
    if (normal.lengthSquared() > 0.0f) normal.normalize();

    // Set plane by normal and a point (location)
    m_view->setClipPlaneFromNormalAndPoint(normal, loc);
}

void SplitPlaneDocker::onAlignToAxisX() {
    ui->rotation_x->setValue(0.0); ui->rotation_y->setValue(90.0); ui->rotation_z->setValue(0.0);
    onTransformEdited();
}

void SplitPlaneDocker::onALignToAxixY() {
    // Align normal to +Y: use spherical mapping -> rotation_y = 90, rotation_z = 90 (rx kept 0)
    ui->rotation_x->setValue(0.0); ui->rotation_y->setValue(90.0); ui->rotation_z->setValue(90.0);
    onTransformEdited();
}

void SplitPlaneDocker::onAlignToAxisZ() {
    ui->rotation_x->setValue(0.0); ui->rotation_y->setValue(0.0); ui->rotation_z->setValue(0.0);
    onTransformEdited();
}

void SplitPlaneDocker::onResetClip() {
    // Reset to Z plane at origin: rotation = identity, loc = (0,0,0)
    ui->rotation_x->setValue(0.0); ui->rotation_y->setValue(0.0); ui->rotation_z->setValue(0.0);
    ui->loc_x->setValue(0.0); ui->loc_y->setValue(0.0); ui->loc_z->setValue(0.0);
    onTransformEdited();
}
