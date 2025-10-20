#include "LogPanel.h"
#include <QPlainTextEdit>

LogPanel::LogPanel(const QString &title, QWidget* parent)
    : QDockWidget(title, parent)
{
    m_text = new QPlainTextEdit;
    m_text->setReadOnly(true);
    setWidget(m_text);
}

LogPanel::~LogPanel() = default;

void LogPanel::appendLog(const QString& line)
{
    if (m_text) {
        m_text->appendPlainText(line);
    }
}

void LogPanel::clearLog()
{
    if (m_text) {
        m_text->clear();
    }
}
