#ifndef POINTTOMESH_POINTSIZECONTROLWIDGET_H
#define POINTTOMESH_POINTSIZECONTROLWIDGET_H

#include <QWidget>

class QSlider;
class QLabel;

class PointSizeControlWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString labelText READ labelText WRITE setLabelText)
public:
    explicit PointSizeControlWidget(QWidget* parent = nullptr);
    ~PointSizeControlWidget() override;

    // Backward-compatible int-based API (used for point size)
    void setRange(int min, int max);
    void setValue(int v);
    int  value() const;

    // Reusable: label text
    void setLabelText(const QString& text);
    [[nodiscard]] QString labelText() const;

    // Reusable: double-based API (used for camera speed, etc.)
    void setRangeDouble(double min, double max, int decimals = 0);
    void setValueDouble(double v);
    [[nodiscard]] double valueDouble() const;

signals:
    void valueChanged(int v);
    void valueChangedDouble(double v);

private slots:
    void onSliderChanged(int v);

private:
    // UI elements
    QSlider* m_slider {nullptr};
    QLabel*  m_valueLabel {nullptr};
    QLabel*  m_label {nullptr};

    // Int mode state
    int m_min {1};
    int m_max {20};

    // Double mode state
    bool m_useDouble {false};
    double m_dmin {0.0};
    double m_dmax {1.0};
    int m_decimals {0};

    // Helpers for mapping between slider ticks and doubles
    int doubleToTick(double v) const;
    double tickToDouble(int tick) const;
};

#endif // POINTTOMESH_POINTSIZECONTROLWIDGET_H
