#include <QApplication>
#include <QSurfaceFormat>
#include "src/UI/mainwindow.h"

int main(int argc, char *argv[]) {
    // Request a modern OpenGL context
    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setVersion(4, 1);
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("PointToMeshProject");
    QCoreApplication::setOrganizationDomain("point2mesh.local");
    QCoreApplication::setApplicationName("PointToMesh");

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}