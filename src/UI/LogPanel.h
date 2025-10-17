#ifndef POINTTOMESH_LOGPANEL_H
#define POINTTOMESH_LOGPANEL_H

#include <QWidget>
#include <QPointer>

class QPlainTextEdit;

class LogPanel : public QWidget {
    Q_OBJECT
public:
    explicit LogPanel(QWidget* parent = nullptr);
    ~LogPanel() override;

public slots:
    void appendLog(const QString& line);
    void clearLog();

private:
    QPointer<QPlainTextEdit> m_text;
};

#endif // POINTTOMESH_LOGPANEL_H
