#ifndef POINTTOMESH_POINTSIZECONTROLWIDGET_H
#define POINTTOMESH_POINTSIZECONTROLWIDGET_H

#include <QWidget>

class QSlider;
class QLabel;

class PointSizeControlWidget : public QWidget {
    Q_OBJECT
public:
    explicit PointSizeControlWidget(QWidget* parent = nullptr);
    ~PointSizeControlWidget() override;

    void setRange(int min, int max);
    void setValue(int v);
    int  value() const;

signals:
    void valueChanged(int v);

private slots:
    void onSliderChanged(int v);

private:
    void loadSettings();
    void saveSettings();

    QSlider* m_slider {nullptr};
    QLabel*  m_valueLabel {nullptr};
    int m_min {1};
    int m_max {20};
};

#endif // POINTTOMESH_POINTSIZECONTROLWIDGET_H

