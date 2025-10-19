#pragma once
#include <QHash>
#include <QString>
#include <QStringList>
#include <QSharedPointer>
class QOpenGLShaderProgram;

class ShaderLibrary {
public:
    ShaderLibrary() = default;
    ~ShaderLibrary();

    bool addProgram(const QString& name, const char* vertSrc, const char* fragSrc, QString* error = nullptr);
    bool addProgramFromFiles(const QString& name, const QString& vertFile, const QString& fragFile, QString* error = nullptr);
    // Ensure a program named `name` is available. If not loaded yet, try to find `<path>/<name>.vert` and `<path>/<name>.frag`
    // in the configured search paths (or sensible defaults) and load it.
    bool ensureProgram(const QString& name, QString* error = nullptr);

    // Configure search paths for shader files. Paths are checked in order.
    void setSearchPaths(const QStringList& paths) { m_searchPaths = paths; }
    void addSearchPath(const QString& path) { m_searchPaths.append(path); }

    QOpenGLShaderProgram* get(const QString& name) const;
    void clear();

private:
    QHash<QString, QSharedPointer<QOpenGLShaderProgram>> m_programs;
    QStringList m_searchPaths;
};
