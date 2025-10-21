// ParameterDialog.h
// Dynamically build a dialog from a BaseInputParameter's Q_PROPERTIES

#ifndef POINTTOMESH_PARAMETERDIALOG_H
#define POINTTOMESH_PARAMETERDIALOG_H

#include <QDialog>
#include <QMap>

class BaseInputParameter;

class ParameterDialog : public QDialog {
    Q_OBJECT
public:
    explicit ParameterDialog(BaseInputParameter *params, QWidget *parent = nullptr);

    // Convenience: show modal and apply on OK. Returns true if accepted.
    static bool editParameters(BaseInputParameter *params, QWidget *parent = nullptr) {
        ParameterDialog dlg(params, parent);
        return dlg.exec() == QDialog::Accepted;
    }

signals:
    // Emitted when the user clicks Apply (does not close the dialog)
    void applyClicked(BaseInputParameter* params);

private slots:
    void accept() override;
    void onApplyClicked();

private:
    BaseInputParameter *m_params = nullptr;
    QMap<QString, QWidget *> m_editors; // property name -> editor widget
};

#endif //POINTTOMESH_PARAMETERDIALOG_H
