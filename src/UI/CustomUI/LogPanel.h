#ifndef POINTTOMESH_LOGDOCKWIDGET_H
#define POINTTOMESH_LOGDOCKWIDGET_H

#include <QDockWidget>
#include <QPointer>

class QPlainTextEdit;

class LogPanel : public QDockWidget {
    Q_OBJECT
public:
    explicit LogPanel(const QString &title, QWidget* parent = nullptr);
    ~LogPanel() override;

public slots:
    void appendLog(const QString& line);
    void clearLog();

private:
    QPointer<QPlainTextEdit> m_text;
};

#endif // POINTTOMESH_LOGDOCKWIDGET_H
