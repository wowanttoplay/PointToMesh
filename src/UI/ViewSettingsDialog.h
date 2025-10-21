#ifndef POINTTOMESH_VIEWSETTINGSDIALOG_H
#define POINTTOMESH_VIEWSETTINGSDIALOG_H

#include <QDockWidget>

class RenderView;

QT_BEGIN_NAMESPACE
namespace Ui { class ViewSettingsDialog; }
QT_END_NAMESPACE

class ViewSettingsDialog : public QDockWidget {
    Q_OBJECT
public:
    explicit ViewSettingsDialog(QWidget* parent = nullptr, RenderView* view = nullptr);
    ~ViewSettingsDialog() override;

private:
    Ui::ViewSettingsDialog* ui {nullptr};
    RenderView* m_view {nullptr};
};

#endif // POINTTOMESH_VIEWSETTINGSDIALOG_H
