#pragma once
#include <QHash>
#include <QString>
#include <QSharedPointer>
class QOpenGLShaderProgram;

class ShaderLibrary {
public:
    ShaderLibrary() = default;
    ~ShaderLibrary();

    bool addProgram(const QString& name, const char* vertSrc, const char* fragSrc, QString* error = nullptr);
    QOpenGLShaderProgram* get(const QString& name) const;
    void clear();

private:
    QHash<QString, QSharedPointer<QOpenGLShaderProgram>> m_programs;
};
