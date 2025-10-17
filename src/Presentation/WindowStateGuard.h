#pragma once
#include <QPointer>

class QMainWindow;

class WindowStateGuard {
public:
    explicit WindowStateGuard(QMainWindow* win);
    ~WindowStateGuard();

    WindowStateGuard(const WindowStateGuard&) = delete;
    WindowStateGuard& operator=(const WindowStateGuard&) = delete;

private:
    QPointer<QMainWindow> m_win;
};

