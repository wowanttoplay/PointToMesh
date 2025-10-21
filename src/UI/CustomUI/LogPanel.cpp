#include "LogPanel.h"
#include <QPlainTextEdit>

LogPanel::LogPanel(const QString &title, QWidget* parent)
    : QDockWidget(title, parent)
{
    m_text = new QPlainTextEdit;
    m_text->setReadOnly(true);
    setWidget(m_text);
    // Ensure QDockWidget has an objectName so QMainWindow::saveState can save/restore it.
    // Use a sanitized version of the title (replace spaces) to make a valid objectName.
    QString obj = title;
    obj.replace(' ', '_');
    setObjectName(obj);
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
