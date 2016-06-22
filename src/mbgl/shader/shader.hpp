#pragma once

#include <mbgl/gl/gl.hpp>
#include <mbgl/gl/object_store.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/optional.hpp>
#include <mbgl/util/traits.hpp>

namespace mbgl {

class Shader : private util::noncopyable {
public:
    ~Shader();
    const char* name;

    GLuint getID() const {
        return program.get();
    }

    GLuint getOverdrawID() const {
        return programOverdraw ? *programOverdraw : 0;
    }

    virtual void bind(GLbyte *offset) = 0;

private:
    enum class Attribute : GLint {
        Invalid = 0,
        Position,
        Extrude,
        Offset,
        Data,
        Data1,
        Data2,
    };

protected:
    Shader(const char* name_, const char* vertex, const char* fragment, gl::ObjectStore&);

    static constexpr GLint     a_pos = underlying_type(Attribute::Position);
    static constexpr GLint a_extrude = underlying_type(Attribute::Extrude);
    static constexpr GLint  a_offset = underlying_type(Attribute::Offset);
    static constexpr GLint    a_data = underlying_type(Attribute::Data);
    static constexpr GLint   a_data1 = underlying_type(Attribute::Data1);
    static constexpr GLint   a_data2 = underlying_type(Attribute::Data2);

private:
    bool compileShader(gl::UniqueShader&, const GLchar *source);
    void linkProgram(gl::UniqueProgram&);

    gl::UniqueProgram program;
    gl::UniqueShader vertexShader;
    gl::UniqueShader fragmentShader;

    mbgl::optional<gl::UniqueProgram> programOverdraw;
    mbgl::optional<gl::UniqueShader> overdrawShader;
};

} // namespace mbgl
