#ifndef POINTTOMESH_VIEWSETTINGSDIALOG_H
#define POINTTOMESH_VIEWSETTINGSDIALOG_H

#include <QDockWidget>
#include <memory>

class RenderView;
class ViewSettingsBinder;
class QAction;

QT_BEGIN_NAMESPACE
namespace Ui { class ViewSettingsDialog; }
QT_END_NAMESPACE

class ViewSettingsDialog : public QDockWidget {
    Q_OBJECT
public:
    // Accept an optional toggle action so binder can sync dock visibility <-> action
    explicit ViewSettingsDialog(RenderView* view, QAction* toggleAction = nullptr, QWidget* parent = nullptr);
    ~ViewSettingsDialog() override;

private:
    Ui::ViewSettingsDialog* ui {nullptr};
    RenderView* m_view {nullptr};
    std::unique_ptr<ViewSettingsBinder> m_binder;
};

#endif // POINTTOMESH_VIEWSETTINGSDIALOG_H
