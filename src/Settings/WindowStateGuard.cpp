#include "WindowStateGuard.h"
#include <QMainWindow>
#include "SettingsManager.h"

WindowStateGuard::WindowStateGuard(QMainWindow* win) : m_win(win) {
    if (m_win) {
        SettingsManager::instance().restoreMainWindowState(*m_win);
    }
}

WindowStateGuard::~WindowStateGuard() {
    if (m_win) {
        SettingsManager::instance().saveMainWindowState(*m_win);
    }
}


