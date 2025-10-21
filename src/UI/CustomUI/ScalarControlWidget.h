#ifndef POINTTOMESH_SCALARCONTROLWIDGET_H
#define POINTTOMESH_SCALARCONTROLWIDGET_H

#include <QWidget>

class QSlider;
class QLabel;

// Generic scalar control with label + slider + value readout.
// Double-only internal representation with configurable decimals.
class ScalarControlWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString labelText READ labelText WRITE setLabelText)
public:
    explicit ScalarControlWidget(QWidget* parent = nullptr);
    ~ScalarControlWidget() override;

    // Label text
    void setLabelText(const QString& text);
    [[nodiscard]] QString labelText() const;

    // Double-only API
    void setRange(double min, double max, int decimals = 0);
    void setValue(double v);
    [[nodiscard]] double value() const;

    // Convenience: int view of current value (rounded)
    [[nodiscard]] int valueInt() const;

signals:
    void valueChanged(double v);

private slots:
    void onSliderChanged(int tick);

private:
    // UI elements
    QSlider* m_slider {nullptr};
    QLabel*  m_valueLabel {nullptr};
    QLabel*  m_label {nullptr};

    // Double mode state
    double m_min {0.0};
    double m_max {1.0};
    int m_decimals {0};

    // Helpers for mapping between slider ticks and doubles
    int doubleToTick(double v) const;
    double tickToDouble(int tick) const;
};

#endif // POINTTOMESH_SCALARCONTROLWIDGET_H

