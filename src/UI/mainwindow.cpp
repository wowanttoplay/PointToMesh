//
// Created by 范杰 on 2025/10/16.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "mainwindow.h"
#include "ui_MainWindow.h"
#include "LogPanel.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(std::make_unique<Ui::MainWindow>()) {
    ui->setupUi(this);

    // Replace dock content with LogPanel so logging is encapsulated
    m_logPanel = new LogPanel(this);
    ui->dockWidget->setWidget(m_logPanel);
}

MainWindow::~MainWindow() = default;
