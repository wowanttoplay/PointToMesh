#include <QApplication>
#include <QLabel>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Point_2 p(1.0, 2.0);

    QString labelText = QString("Hello from Qt and CGAL!\\nCGAL Point: (%1, %2)").arg(p.x()).arg(p.y());

    QLabel label(labelText);
    label.show();

    return app.exec();
}