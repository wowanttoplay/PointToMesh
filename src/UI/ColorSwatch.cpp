#include "ColorSwatch.h"
#include <QPainter>
#include <QMouseEvent>
#include <QColorDialog>

ColorSwatch::ColorSwatch(QWidget* parent) : QWidget(parent) {
    setAutoFillBackground(false);
    setCursor(Qt::PointingHandCursor);
}

ColorSwatch::~ColorSwatch() = default;

void ColorSwatch::setColor(const QColor& c) {
    if (c == m_color) return;
    m_color = c;
    update();
    emit colorChanged(m_color);
}

void ColorSwatch::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRect r = rect().adjusted(2, 2, -2, -2);
    QPen pen(Qt::black);
    pen.setWidth(1);
    p.setPen(pen);
    p.setBrush(m_color);
    p.drawRoundedRect(r, 4, 4);
}

void ColorSwatch::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        const QColor chosen = QColorDialog::getColor(m_color, this, m_dialogTitle, QColorDialog::ShowAlphaChannel);
        if (chosen.isValid()) {
            setColor(chosen);
        }
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

QSize ColorSwatch::sizeHint() const {
    return { 80, 24 };
}

QSize ColorSwatch::minimumSizeHint() const {
    return { 40, 20 };
}

