// ParameterDialog.cpp
// Build a dialog dynamically from a BaseInputParameter's Q_PROPERTIES

#include "ParameterDialog.h"
#include "../DataProcess/BaseInputParameter.h"

#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMetaProperty>
#include <QLabel>
#include <QPushButton>

#include <limits>

ParameterDialog::ParameterDialog(BaseInputParameter *params, QWidget *parent)
    : QDialog(parent), m_params(params) {
    QFormLayout *form = new QFormLayout(this);

    const QMetaObject *mo = params ? params->metaObject() : nullptr;
    if (mo) {
        // start from propertyOffset() to skip inherited properties (like QObject::objectName)
        int start = mo->propertyOffset();
        for (int i = start; i < mo->propertyCount(); ++i) {
            QMetaProperty prop = mo->property(i);
            if (!prop.isWritable()) continue;
            const char *cname = prop.name();
            if (!cname) continue;
            QString name = QString::fromLatin1(cname);

            QVariant value = params->property(cname);

            QWidget *editor = nullptr;
            QString typeName = QString::fromLatin1(prop.typeName());

            if (typeName == "int" || typeName == "long" || typeName == "short") {
                QSpinBox *sb = new QSpinBox(this);
                sb->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
                sb->setValue(value.toInt());
                editor = sb;
            } else if (typeName == "double" || typeName == "float") {
                QDoubleSpinBox *db = new QDoubleSpinBox(this);
                db->setRange(-1e9, 1e9);
                db->setDecimals(6);
                db->setValue(value.toDouble());
                editor = db;
            } else if (typeName == "bool") {
                QCheckBox *cb = new QCheckBox(this);
                cb->setChecked(value.toBool());
                editor = cb;
            } else {
                QLineEdit *le = new QLineEdit(this);
                le->setText(value.toString());
                editor = le;
            }

            form->addRow(new QLabel(name + ":", this), editor);
            m_editors.insert(name, editor);
        }
    }

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    // Add Apply button that applies without closing
    QPushButton *applyButton = new QPushButton(tr("Apply"), this);
    buttons->addButton(applyButton, QDialogButtonBox::ActionRole);
    connect(buttons, &QDialogButtonBox::rejected, this, &ParameterDialog::reject);
    connect(applyButton, &QPushButton::clicked, this, &ParameterDialog::onApplyClicked);

    form->addRow(buttons);
    setLayout(form);
    setWindowTitle("Edit Parameters");
}

void ParameterDialog::onApplyClicked() {
    if (!m_params) return;

    // Same apply logic as accept but do not close the dialog
    for (auto it = m_editors.constBegin(); it != m_editors.constEnd(); ++it) {
        QString name = it.key();
        QWidget *w = it.value();
        QVariant newVal;

        if (auto sb = qobject_cast<QSpinBox *>(w)) {
            newVal = sb->value();
        } else if (auto db = qobject_cast<QDoubleSpinBox *>(w)) {
            newVal = db->value();
        } else if (auto cb = qobject_cast<QCheckBox *>(w)) {
            newVal = cb->isChecked();
        } else if (auto le = qobject_cast<QLineEdit *>(w)) {
            newVal = le->text();
        }

        if (!newVal.isNull()) {
            m_params->setProperty(name.toLatin1().constData(), newVal);
        }
    }

    emit applyClicked(m_params);
}
