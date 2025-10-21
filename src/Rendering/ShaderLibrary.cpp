#include "ShaderLibrary.h"
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

ShaderLibrary::~ShaderLibrary() { clear(); }

bool ShaderLibrary::addProgram(const QString& name, const char* vertSrc, const char* fragSrc, QString* error) {
    auto prog = QSharedPointer<QOpenGLShaderProgram>::create();
    if (!prog->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc)) {
        if (error) *error = prog->log();
        return false;
    }
    if (!prog->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc)) {
        if (error) *error = prog->log();
        return false;
    }
    if (!prog->link()) {
        if (error) *error = prog->log();
        return false;
    }
    m_programs.insert(name, prog);
    return true;
}

bool ShaderLibrary::addProgramFromFiles(const QString& name, const QString& vertFile, const QString& fragFile, QString* error) {
    auto prog = QSharedPointer<QOpenGLShaderProgram>::create();
    if (!prog->addShaderFromSourceFile(QOpenGLShader::Vertex, vertFile)) {
        if (error) *error = prog->log();
        return false;
    }
    if (!prog->addShaderFromSourceFile(QOpenGLShader::Fragment, fragFile)) {
        if (error) *error = prog->log();
        return false;
    }
    if (!prog->link()) {
        if (error) *error = prog->log();
        return false;
    }
    m_programs.insert(name, prog);
    return true;
}

bool ShaderLibrary::ensureProgram(const QString& name, QString* error) {
    if (get(name)) return true;

    // Build default search paths if none specified
    QStringList paths = m_searchPaths;
    if (paths.isEmpty()) {
        // Prefer embedded Qt resource first to avoid stale on-disk files
        paths << QStringLiteral(":/resources/shaders");
    }

    const QString vertFileName = name + ".vert";
    const QString fragFileName = name + ".frag";
    const QString geomFileName = name + ".geom";

    QString lastErr;
    for (const QString& base : paths) {
        const QString v = QDir(base).filePath(vertFileName);
        const QString f = QDir(base).filePath(fragFileName);
        const QString g = QDir(base).filePath(geomFileName);
        const bool vExists = QFileInfo::exists(v);
        const bool fExists = QFileInfo::exists(f);
        if (!vExists || !fExists) {
            continue;
        }

        auto prog = QSharedPointer<QOpenGLShaderProgram>::create();
        if (!prog->addShaderFromSourceFile(QOpenGLShader::Vertex, v)) {
            lastErr = prog->log();
            continue;
        }
        // Optional geometry shader
        if (QFileInfo::exists(g)) {
            if (!prog->addShaderFromSourceFile(QOpenGLShader::Geometry, g)) {
                lastErr = prog->log();
                continue;
            }
        }
        if (!prog->addShaderFromSourceFile(QOpenGLShader::Fragment, f)) {
            lastErr = prog->log();
            continue;
        }
        if (!prog->link()) {
            lastErr = prog->log();
            continue;
        }
        m_programs.insert(name, prog);
        return true;
    }

    if (error) {
        if (!lastErr.isEmpty()) *error = lastErr;
        else *error = QStringLiteral("Shader '%1' not found in search paths").arg(name);
    }
    return false;
}

QOpenGLShaderProgram* ShaderLibrary::get(const QString& name) const {
    auto it = m_programs.find(name);
    return it == m_programs.end() ? nullptr : it.value().data();
}

void ShaderLibrary::clear() {
    m_programs.clear();
}
