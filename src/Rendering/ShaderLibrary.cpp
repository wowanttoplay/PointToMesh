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
        // 1) app dir/resources/shaders (deploy layout)
        paths << (QCoreApplication::applicationDirPath() + "/resources/shaders");
        // 2) current working dir/resources/shaders (dev layout)
        paths << (QDir::currentPath() + "/resources/shaders");
        // 3) just resources/shaders relative (for some IDE run configs)
        paths << QStringLiteral("resources/shaders");
        // 4) Optional Qt resource prefix if in a qrc
        paths << QStringLiteral(":/resources/shaders");
    }

    const QString vertFileName = name + ".vert";
    const QString fragFileName = name + ".frag";

    QString lastErr;
    for (const QString& base : paths) {
        const QString v = QDir(base).filePath(vertFileName);
        const QString f = QDir(base).filePath(fragFileName);
        // If it's a Qt resource path (:/...), QFileInfo on Qt resources may report exists as true
        const bool vExists = QFileInfo::exists(v);
        const bool fExists = QFileInfo::exists(f);
        if (!vExists || !fExists) {
            continue;
        }
        if (addProgramFromFiles(name, v, f, &lastErr)) {
            return true;
        }
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
