#include "ScalarControlWidget.h"

#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <algorithm>
#include <cmath>

ScalarControlWidget::ScalarControlWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);

    m_label = new QLabel(tr("Value:"), this);
    m_slider = new QSlider(Qt::Horizontal, this);
    m_valueLabel = new QLabel(this);

    // Default range 0..1 with 0 decimals (i.e., integer-like)
    setRange(0.0, 1.0, 0);

    layout->addWidget(m_label);
    layout->addWidget(m_slider, 1);
    layout->addWidget(m_valueLabel);

    connect(m_slider, &QSlider::valueChanged, this, &ScalarControlWidget::onSliderChanged);

    // Initialize UI to default
    setValue(0.0);
}

ScalarControlWidget::~ScalarControlWidget() = default;

void ScalarControlWidget::setLabelText(const QString& text) {
    if (m_label) m_label->setText(text);
}

QString ScalarControlWidget::labelText() const {
    return m_label ? m_label->text() : QString();
}

void ScalarControlWidget::setRange(double min, double max, int decimals) {
    if (min > max) std::swap(min, max);
    m_min = min;
    m_max = max;
    m_decimals = std::clamp(decimals, 0, 6);

    // Slider ticks go from 0..(max-min)*10^decimals
    const int maxTick = doubleToTick(m_max);
    if (m_slider) m_slider->setRange(0, std::max(1, maxTick));

    // Clamp current value in new range
    if (m_slider) {
        const int clamped = std::clamp(m_slider->value(), 0, std::max(1, maxTick));
        m_slider->setValue(clamped);
        const double v = tickToDouble(clamped);
        if (m_valueLabel) m_valueLabel->setText(QString::number(v, 'f', m_decimals));
    }
}

void ScalarControlWidget::setValue(double v) {
    const double clamped = std::clamp(v, m_min, m_max);
    const int tick = doubleToTick(clamped);
    if (m_slider) m_slider->setValue(tick);
    if (m_valueLabel) m_valueLabel->setText(QString::number(clamped, 'f', m_decimals));
}

double ScalarControlWidget::value() const {
    if (!m_slider) return m_min;
    return tickToDouble(m_slider->value());
}

int ScalarControlWidget::valueInt() const {
    return static_cast<int>(std::lround(value()));
}

void ScalarControlWidget::onSliderChanged(int tick) {
    const double d = tickToDouble(tick);
    if (m_valueLabel) m_valueLabel->setText(QString::number(d, 'f', m_decimals));
    emit valueChanged(d);
}

int ScalarControlWidget::doubleToTick(double v) const {
    const double scale = std::pow(10.0, m_decimals);
    return static_cast<int>(std::lround((v - m_min) * scale));
}

double ScalarControlWidget::tickToDouble(int tick) const {
    const double scale = std::pow(10.0, m_decimals);
    return m_min + static_cast<double>(tick) / scale;
}

