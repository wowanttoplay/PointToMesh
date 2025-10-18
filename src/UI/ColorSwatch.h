#ifndef POINTTOMESH_COLORSWATCH_H
#define POINTTOMESH_COLORSWATCH_H

#include <QWidget>
#include <QColor>

class ColorSwatch : public QWidget {
    Q_OBJECT
public:
    explicit ColorSwatch(QWidget* parent = nullptr);
    ~ColorSwatch() override;

    void setColor(const QColor& c);
    QColor color() const { return m_color; }

    void setDialogTitle(const QString& t) { m_dialogTitle = t; }

signals:
    void colorChanged(const QColor& c);

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QColor m_color { Qt::white };
    QString m_dialogTitle { tr("Choose Color") };
};

#endif // POINTTOMESH_COLORSWATCH_H

