#ifndef POINTTOMESH_COLORSWATCH_H
#define POINTTOMESH_COLORSWATCH_H

#include <QWidget>
#include <QColor>
#include <QString>

class ColorSwatch : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString label READ label WRITE setLabel)
public:
    explicit ColorSwatch(QWidget* parent = nullptr);
    ~ColorSwatch() override;

    void setColor(const QColor& c);
    [[nodiscard]] QColor color() const { return m_color; }

    // Label shown on the left side
    void setLabel(const QString& t) { m_label = t; update(); }
    [[nodiscard]] QString label() const { return m_label; }

    void setDialogTitle(const QString& t) { m_dialogTitle = t; }

signals:
    void colorChanged(const QColor& c);

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

private:
    QColor m_color { Qt::white };
    QString m_dialogTitle { tr("Choose Color") };
    QString m_label; // text displayed on the left
};

#endif // POINTTOMESH_COLORSWATCH_H
