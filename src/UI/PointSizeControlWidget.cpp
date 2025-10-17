#include "PointSizeControlWidget.h"

#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QSettings>
#include <algorithm>

static constexpr const char* kSettingsGroup = "render";
static constexpr const char* kPointSizeKey  = "pointSize";

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

    loadSettings();
}

PointSizeControlWidget::~PointSizeControlWidget() {
    saveSettings();
}

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
    saveSettings();
    emit valueChanged(v);
}

void PointSizeControlWidget::loadSettings() {
    QSettings s;
    s.beginGroup(kSettingsGroup);
    const int saved = s.value(kPointSizeKey, 3).toInt();
    s.endGroup();
    setValue(saved);
}

void PointSizeControlWidget::saveSettings() {
    QSettings s;
    s.beginGroup(kSettingsGroup);
    s.setValue(kPointSizeKey, value());
    s.endGroup();
}

