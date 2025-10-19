#include "PointSizeControlWidget.h"

#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <algorithm>
#include <cmath>

PointSizeControlWidget::PointSizeControlWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);

    m_label = new QLabel(tr("Point Size:"), this);
    m_slider = new QSlider(Qt::Horizontal, this);
    m_valueLabel = new QLabel(this);

    m_slider->setRange(m_min, m_max);

    layout->addWidget(m_label);
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
    m_useDouble = false;
    if (m_slider) m_slider->setRange(m_min, m_max);
}

void PointSizeControlWidget::setValue(int v) {
    m_useDouble = false;
    const int clamped = std::clamp(v, m_min, m_max);
    if (m_slider) m_slider->setValue(clamped);
}

int PointSizeControlWidget::value() const {
    return m_slider ? m_slider->value() : m_min;
}

void PointSizeControlWidget::setLabelText(const QString& text) {
    if (m_label) m_label->setText(text);
}

QString PointSizeControlWidget::labelText() const {
    return m_label ? m_label->text() : QString();
}

void PointSizeControlWidget::setRangeDouble(double min, double max, int decimals) {
    if (min > max) std::swap(min, max);
    m_useDouble = true;
    m_dmin = min;
    m_dmax = max;
    m_decimals = std::clamp(decimals, 0, 6);
    const int maxTick = doubleToTick(m_dmax);
    if (m_slider) m_slider->setRange(0, std::max(1, maxTick));
}

void PointSizeControlWidget::setValueDouble(double v) {
    m_useDouble = true;
    const double clamped = std::clamp(v, m_dmin, m_dmax);
    const int tick = doubleToTick(clamped);
    if (m_slider) m_slider->setValue(tick);
    if (m_valueLabel) m_valueLabel->setText(QString::number(clamped, 'f', m_decimals));
}

double PointSizeControlWidget::valueDouble() const {
    if (!m_slider) return m_dmin;
    return tickToDouble(m_slider->value());
}

void PointSizeControlWidget::onSliderChanged(int v) {
    if (m_useDouble) {
        const double d = tickToDouble(v);
        if (m_valueLabel) m_valueLabel->setText(QString::number(d, 'f', m_decimals));
        emit valueChangedDouble(d);
        emit valueChanged(static_cast<int>(std::lround(d))); // optional compatibility
    } else {
        if (m_valueLabel) m_valueLabel->setText(QString::number(v));
        emit valueChanged(v);
        emit valueChangedDouble(static_cast<double>(v)); // optional for listeners that only use double
    }
}

int PointSizeControlWidget::doubleToTick(double v) const {
    const double scale = std::pow(10.0, m_decimals);
    return static_cast<int>(std::lround((v - m_dmin) * scale));
}

double PointSizeControlWidget::tickToDouble(int tick) const {
    const double scale = std::pow(10.0, m_decimals);
    return m_dmin + static_cast<double>(tick) / scale;
}
