#ifndef POINTTOMESH_VIEWSETTINGSDIALOG_H
#define POINTTOMESH_VIEWSETTINGSDIALOG_H

#include <QDialog>
#include <memory>

class RenderView;
class ViewSettingsBinder;

QT_BEGIN_NAMESPACE
namespace Ui { class ViewSettingsDialog; }
QT_END_NAMESPACE

class ViewSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ViewSettingsDialog(RenderView* view, QWidget* parent = nullptr);
    ~ViewSettingsDialog() override;

private:
    Ui::ViewSettingsDialog* ui {nullptr};
    RenderView* m_view {nullptr};
    std::unique_ptr<ViewSettingsBinder> m_binder;
};

#endif // POINTTOMESH_VIEWSETTINGSDIALOG_H

