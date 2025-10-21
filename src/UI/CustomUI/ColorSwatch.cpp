#include "ColorSwatch.h"
#include <QPainter>
#include <QMouseEvent>
#include <QColorDialog>
#include <QFontMetrics>
#include <QStyleOption>

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

    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    // let style draw the background if needed
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    const QRect r = rect();
    const int margin = 6;
    const int spacing = 8;

    // Determine circle size
    const int availH = qMax(0, r.height() - 2 * margin);
    const int diameter = qMax(12, availH);
    const int circleX = r.right() - margin - diameter;
    const int circleY = r.top() + (r.height() - diameter) / 2;
    const QRect circleRect(circleX, circleY, diameter, diameter);

    // Text area to the left of the circle
    QFontMetrics fm(font());
    const int textLeft = r.left() + margin;
    const int textRight = circleRect.left() - spacing;
    const int textWidth = qMax(0, textRight - textLeft);
    const QRect textRect(textLeft, r.top(), textWidth, r.height());

    if (!m_label.isEmpty() && textRect.width() > 0) {
        const QString elided = fm.elidedText(m_label, Qt::ElideRight, textRect.width());
        p.setPen(palette().color(QPalette::WindowText));
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elided);
    }

    // Draw the color circle with a border
    QPen pen(Qt::black);
    pen.setWidth(1);
    p.setPen(pen);
    p.setBrush(m_color);
    p.drawEllipse(circleRect);
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
    const int margin = 6;
    const int spacing = 8;
    QFontMetrics fm(font());
    const int baseH = qMax(24, fm.height() + margin * 2);
    const int circle = baseH - margin * 2; // match paint layout
    const int textW = m_label.isEmpty() ? 0 : fm.horizontalAdvance(m_label);
    const int w = margin + textW + (m_label.isEmpty() ? 0 : spacing) + circle + margin;
    return { w, baseH };
}

QSize ColorSwatch::minimumSizeHint() const {
    const int margin = 6;
    const int minH = 22;
    const int circle = minH - margin * 2;
    const int w = margin + (m_label.isEmpty() ? 0 : 40) + (m_label.isEmpty() ? 0 : 8) + circle + margin; // allow some text
    return { w, minH };
}
