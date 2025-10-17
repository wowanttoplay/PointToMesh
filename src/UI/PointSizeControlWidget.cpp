#include "PointSizeControlWidget.h"

#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <algorithm>

PointSizeControlWidget::PointSizeControlWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);

    auto* label = new QLabel(tr("Point Size:"), this);
    m_slider = new QSlider(Qt::Horizontal, this);
    m_valueLabel = new QLabel(this);

    m_slider->setRange(m_min, m_max);

    layout->addWidget(label);
    layout->addWidget(m_slider, 1);
    layout->addWidget(m_valueLabel);

    connect(m_slider, &QSlider::valueChanged, this, &PointSizeControlWidget::onSliderChanged);

    // Initialize UI to a sensible default; binder will override with persisted value
    m_slider->setValue(3);
    m_valueLabel->setText(QString::number(m_slider->value()));
}

PointSizeControlWidget::~PointSizeControlWidget() = default;

void PointSizeControlWidget::setRange(int min, int max) {
    if (min > max) std::swap(min, max);
    m_min = min; m_max = max;
    if (m_slider) m_slider->setRange(m_min, m_max);
}

void PointSizeControlWidget::setValue(int v) {
    const int clamped = std::clamp(v, m_min, m_max);
    if (m_slider) m_slider->setValue(clamped);
}

int PointSizeControlWidget::value() const {
    return m_slider ? m_slider->value() : m_min;
}

void PointSizeControlWidget::onSliderChanged(int v) {
    if (m_valueLabel) m_valueLabel->setText(QString::number(v));
    emit valueChanged(v);
}
