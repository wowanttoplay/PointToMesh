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
}

SplitPlaneDocker::~SplitPlaneDocker() {
    delete ui;
}
