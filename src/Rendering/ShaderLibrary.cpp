#include "ShaderLibrary.h"
#include <QOpenGLShaderProgram>

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

QOpenGLShaderProgram* ShaderLibrary::get(const QString& name) const {
    auto it = m_programs.find(name);
    return it == m_programs.end() ? nullptr : it.value().data();
}

void ShaderLibrary::clear() {
    m_programs.clear();
}
