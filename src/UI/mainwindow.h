//
// Created by 范杰 on 2025/10/16.
//

#ifndef POINTTOMESH_MAINWINDOW_H
#define POINTTOMESH_MAINWINDOW_H

#include <QMainWindow>


QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
};


#endif //POINTTOMESH_MAINWINDOW_H