#include "LogPanel.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>

LogPanel::LogPanel(QWidget* parent)
    : QWidget(parent)
    , m_text(new QPlainTextEdit(this))
{
    m_text->setObjectName("logOutput");
    m_text->setReadOnly(true);
    m_text->setLineWrapMode(QPlainTextEdit::NoWrap);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_text);
}

LogPanel::~LogPanel() = default;

void LogPanel::appendLog(const QString& line) {
    if (!m_text) return;
    m_text->appendPlainText(line);
}

void LogPanel::clearLog() {
    if (!m_text) return;
    m_text->clear();
}

