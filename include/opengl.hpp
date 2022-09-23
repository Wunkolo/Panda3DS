/***************************************************************************
 *   Copyright (C) 2022 PCSX-Redux authors                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#pragma once
#include <array>
#include <cassert>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "gl3w.h"

namespace OpenGL {

    // Workaround for using static_assert inside constexpr if
    // https://stackoverflow.com/questions/53945490/how-to-assert-that-a-constexpr-if-else-clause-never-happen
    template <class...>
    constexpr std::false_type AlwaysFalse{};

    struct VertexArray {
        GLuint m_handle = 0;

        void create() {
            if (m_handle == 0) {
                glGenVertexArrays(1, &m_handle);
            }
        }
        VertexArray(bool shouldCreate = false) {
            if (shouldCreate) {
                create();
            }
        }

        ~VertexArray() { glDeleteVertexArrays(1, &m_handle); }
        GLuint handle() { return m_handle; }
        bool exists() { return m_handle != 0; }
        void bind() { glBindVertexArray(m_handle); }

        template <typename T>
        void setAttributeFloat(GLuint index, GLint size, GLsizei stride, const void* offset, bool normalized = GL_FALSE) {
            if constexpr (std::is_same<T, GLfloat>()) {
                glVertexAttribPointer(index, size, GL_FLOAT, normalized, stride, offset);
            }
            else if constexpr (std::is_same<T, GLbyte>()) {
                glVertexAttribPointer(index, size, GL_BYTE, normalized, stride, offset);
            }
            else if constexpr (std::is_same<T, GLubyte>()) {
                glVertexAttribPointer(index, size, GL_UNSIGNED_BYTE, normalized, stride, offset);
            }
            else if constexpr (std::is_same<T, GLshort>()) {
                glVertexAttribPointer(index, size, GL_SHORT, normalized, stride, offset);
            }
            else if constexpr (std::is_same<T, GLushort>()) {
                glVertexAttribPointer(index, size, GL_UNSIGNED_SHORT, normalized, stride, offset);
            }
            else if constexpr (std::is_same<T, GLint>()) {
                glVertexAttribPointer(index, size, GL_INT, normalized, stride, offset);
            }
            else if constexpr (std::is_same<T, GLuint>()) {
                glVertexAttribPointer(index, size, GL_UNSIGNED_INT, normalized, stride, offset);
            }
            else {
                static_assert(AlwaysFalse<T>, "Unimplemented type for OpenGL::setAttributeFloat");
            }
        }

        template <typename T>
        void setAttributeInt(GLuint index, GLint size, GLsizei stride, const void* offset) {
            if constexpr (std::is_same<T, GLbyte>()) {
                glVertexAttribIPointer(index, size, GL_BYTE, stride, offset);
            }
            else if constexpr (std::is_same<T, GLubyte>()) {
                glVertexAttribIPointer(index, size, GL_UNSIGNED_BYTE, stride, offset);
            }
            else if constexpr (std::is_same<T, GLshort>()) {
                glVertexAttribIPointer(index, size, GL_SHORT, stride, offset);
            }
            else if constexpr (std::is_same<T, GLushort>()) {
                glVertexAttribIPointer(index, size, GL_UNSIGNED_SHORT, stride, offset);
            }
            else if constexpr (std::is_same<T, GLint>()) {
                glVertexAttribIPointer(index, size, GL_INT, stride, offset);
            }
            else if constexpr (std::is_same<T, GLuint>()) {
                glVertexAttribIPointer(index, size, GL_UNSIGNED_INT, stride, offset);
            }
            else {
                static_assert(AlwaysFalse<T>, "Unimplemented type for OpenGL::setAttributeInt");
            }
        }

        template <typename T>
        void setAttributeFloat(GLuint index, GLint size, GLsizei stride, size_t offset, bool normalized = false) {
            setAttributeFloat<T>(index, size, stride, reinterpret_cast<const void*>(offset), normalized);
        }

        template <typename T>
        void setAttributeInt(GLuint index, GLint size, GLsizei stride, size_t offset) {
            setAttributeInt<T>(index, size, stride, reinterpret_cast<const void*>(offset));
        }

        void enableAttribute(GLuint index) { glEnableVertexAttribArray(index); }
        void disableAttribute(GLuint index) { glDisableVertexAttribArray(index); }
    };

    enum FramebufferTypes {
        DrawFramebuffer = GL_DRAW_FRAMEBUFFER,
        ReadFramebuffer = GL_READ_FRAMEBUFFER,
        DrawAndReadFramebuffer = GL_FRAMEBUFFER
    };

    struct Texture {
        GLuint m_handle = 0;
        int m_width, m_height;
        GLenum m_binding;
        int m_samples = 1;  // For MSAA

        void create(int width, int height, GLint internalFormat, GLenum binding = GL_TEXTURE_2D, int samples = 1) {
            m_width = width;
            m_height = height;
            m_binding = binding;

            glGenTextures(1, &m_handle);
            bind();

            if (binding == GL_TEXTURE_2D_MULTISAMPLE) {
                if (!glTexStorage2DMultisample) {  // Assert that glTexStorage2DMultisample has been loaded
                    throw std::runtime_error("MSAA is not supported on this platform");
                }

                int maxSampleCount;
                glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &maxSampleCount);
                if (samples > maxSampleCount) {
                    samples = maxSampleCount;
                }

                glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_TRUE);
            }
            else {
                glTexStorage2D(binding, 1, internalFormat, width, height);
            }
        }

        void createMSAA(int width, int height, GLint internalFormat, int samples) {
            create(width, height, internalFormat, GL_TEXTURE_2D_MULTISAMPLE, samples);
        }

        ~Texture() { glDeleteTextures(1, &m_handle); }
        GLuint handle() { return m_handle; }
        bool exists() { return m_handle != 0; }
        void bind() { glBindTexture(m_binding, m_handle); }
        int width() { return m_width; }
        int height() { return m_height; }
    };

    struct Framebuffer {
        GLuint m_handle = 0;
        GLenum m_textureType;  // GL_TEXTURE_2D or GL_TEXTURE_2D_MULTISAMPLE

        void create() {
            if (m_handle == 0) {
                glGenFramebuffers(1, &m_handle);
            }
        }

        Framebuffer(bool shouldCreate = false) {
            if (shouldCreate) {
                create();
            }
        }

        ~Framebuffer() { glDeleteFramebuffers(1, &m_handle); }
        GLuint handle() { return m_handle; }
        bool exists() { return m_handle != 0; }
        void bind(GLenum target) { glBindFramebuffer(target, m_handle); }
        void bind(FramebufferTypes target) { bind(static_cast<GLenum>(target)); }

        void createWithTexture(Texture& tex, GLenum mode = GL_FRAMEBUFFER, GLenum textureType = GL_TEXTURE_2D) {
            m_textureType = textureType;
            create();
            bind(mode);
            glFramebufferTexture2D(mode, GL_COLOR_ATTACHMENT0, textureType, tex.handle(), 0);
        }

        void createWithReadTexture(Texture& tex, GLenum textureType = GL_TEXTURE_2D) {
            createWithTexture(tex, GL_READ_FRAMEBUFFER, textureType);
        }
        void createWithDrawTexture(Texture& tex, GLenum textureType = GL_TEXTURE_2D) {
            createWithTexture(tex, GL_DRAW_FRAMEBUFFER, textureType);
        }

        void createWithTextureMSAA(Texture& tex, GLenum mode = GL_FRAMEBUFFER) {
            m_textureType = GL_TEXTURE_2D_MULTISAMPLE;
            create();
            bind(mode);
            glFramebufferTexture2D(mode, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex.handle(), 0);
        }
    };

    enum ShaderType {
        Fragment = GL_FRAGMENT_SHADER,
        Vertex = GL_VERTEX_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
        Compute = GL_COMPUTE_SHADER,
        TessControl = GL_TESS_CONTROL_SHADER,
        TessEvaluation = GL_TESS_EVALUATION_SHADER
    };

    struct Shader {
        GLuint m_handle = 0;

        Shader() {}
        Shader(const std::string_view source, ShaderType type) { create(source, static_cast<GLenum>(type)); }

        // Returns whether compilation failed or not
        bool create(const std::string_view source, GLenum type) {
            m_handle = glCreateShader(type);
            const GLchar* const sources[1] = { source.data() };

            glShaderSource(m_handle, 1, sources, nullptr);
            glCompileShader(m_handle);

            GLint success;
            glGetShaderiv(m_handle, GL_COMPILE_STATUS, &success);
            if (success == GL_FALSE) {
                char buf[4096];
                glGetShaderInfoLog(m_handle, 4096, nullptr, buf);
                fprintf(stderr, "Failed to compile shader\nError: %s\n", buf);
                glDeleteShader(m_handle);

                m_handle = 0;
            }

            return m_handle != 0;
        }

        GLuint handle() { return m_handle; }
        bool exists() { return m_handle != 0; }
    };

    struct Program {
        GLuint m_handle = 0;

        bool create(std::initializer_list<std::reference_wrapper<Shader>> shaders) {
            m_handle = glCreateProgram();
            for (const auto& shader : shaders) {
                glAttachShader(m_handle, shader.get().handle());
            }

            glLinkProgram(m_handle);
            GLint success;
            glGetProgramiv(m_handle, GL_LINK_STATUS, &success);

            if (!success) {
                char buf[4096];
                glGetProgramInfoLog(m_handle, 4096, nullptr, buf);
                fprintf(stderr, "Failed to link program\nError: %s\n", buf);
                glDeleteProgram(m_handle);

                m_handle = 0;
            }

            return m_handle != 0;
        }

        GLuint handle() { return m_handle; }
        bool exists() { return m_handle != 0; }
        void use() { glUseProgram(m_handle); }
    };

    static void dispatchCompute(GLuint groupsX = 1, GLuint groupsY = 1, GLuint groupsZ = 1) {
        glDispatchCompute(groupsX, groupsY, groupsZ);
    }

    struct VertexBuffer {
        GLuint m_handle = 0;

        void create() {
            if (m_handle == 0) {
                glGenBuffers(1, &m_handle);
            }
        }

        void createFixedSize(GLsizei size, GLenum usage = GL_DYNAMIC_DRAW) {
            create();
            bind();
            glBufferData(GL_ARRAY_BUFFER, size, nullptr, usage);
        }

        VertexBuffer(bool shouldCreate = false) {
            if (shouldCreate) {
                create();
            }
        }

        ~VertexBuffer() { glDeleteBuffers(1, &m_handle); }
        GLuint handle() { return m_handle; }
        bool exists() { return m_handle != 0; }
        void bind() { glBindBuffer(GL_ARRAY_BUFFER, m_handle); }

        // Reallocates the buffer on every call. Prefer the sub version if possible.
        template <typename VertType>
        void bufferVerts(VertType* vertices, int vertCount, GLenum usage = GL_DYNAMIC_DRAW) {
            glBufferData(GL_ARRAY_BUFFER, sizeof(VertType) * vertCount, vertices, usage);
        }

        // Only use if you used createFixedSize
        template <typename VertType>
        void bufferVertsSub(VertType* vertices, int vertCount, GLintptr offset = 0) {
            glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(VertType) * vertCount, vertices);
        }
    };

    enum DepthFunc {
        Never = GL_NEVER,       // Depth test never passes
        Always = GL_ALWAYS,     // Depth test always passes
        Equal = GL_EQUAL,       // Depth test passes if frag z == depth buffer z
        NotEqual = GL_NOTEQUAL, // Depth test passes if frag z != depth buffer z
        Less = GL_LESS,         // Depth test passes if frag z < depth buffer z
        Lequal = GL_LEQUAL,     // Depth test passes if frag z <= depth buffer z
        Greater = GL_GREATER,   // Depth test passes if frag z > depth buffer z
        Gequal = GL_GEQUAL,     // Depth test passes if frag z >= depth buffer z 
    };

    static void setClearColor(float val) { glClearColor(val, val, val, val); }
    static void setClearColor(float r, float g, float b, float a) { glClearColor(r, g, b, a); }
    static void setClearDepth(float depth) { glClearDepthf(depth); }
    static void setClearStencil(GLint stencil) { glClearStencil(stencil); }
    static void clearColor() { glClear(GL_COLOR_BUFFER_BIT); }
    static void clearDepth() { glClear(GL_DEPTH_BUFFER_BIT); }
    static void clearStencil() { glClear(GL_STENCIL_BUFFER_BIT); }
    static void clearColorAndDepth() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
    static void clearColorAndStencil() { glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
    static void clearDepthAndStencil() { glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
    static void clearAll() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }

    static void setViewport(GLsizei width, GLsizei height) { glViewport(0, 0, width, height); }
    static void setViewport(GLsizei x, GLsizei y, GLsizei width, GLsizei height) { glViewport(x, y, width, height); }
    static void setScissor(GLsizei width, GLsizei height) { glScissor(0, 0, width, height); }
    static void setScissor(GLsizei x, GLsizei y, GLsizei width, GLsizei height) { glScissor(x, y, width, height); }
    static void setStencilMask(GLuint mask) { glStencilMask(mask); }

    static void bindScreenFramebuffer() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
    static void enableScissor() { glEnable(GL_SCISSOR_TEST); }
    static void disableScissor() { glDisable(GL_SCISSOR_TEST); }
    static void enableBlend() { glEnable(GL_BLEND); }
    static void disableBlend() { glDisable(GL_BLEND); }
    static void enableDepth() { glEnable(GL_DEPTH_TEST); }
    static void disableDepth() { glDisable(GL_DEPTH_TEST); }
    static void enableStencil() { glEnable(GL_STENCIL_TEST); }
    static void disableStencil() { glDisable(GL_STENCIL_TEST); }

    static void setDepthFunc(DepthFunc func) { glDepthFunc(static_cast<GLenum>(func)); }

    enum Primitives {
        Triangle = GL_TRIANGLES,
        Triangles = Triangle,
        Tri = Triangle,
        Tris = Triangle,
        TriangleStrip = GL_TRIANGLE_STRIP,
        TriangleFan = GL_TRIANGLE_FAN,
        Line = GL_LINES,
        Lines = Line,
        LineStrip = GL_LINE_STRIP,
        Point = GL_POINTS,
        Points = Point
    };

    static void draw(Primitives prim, GLsizei vertexCount) { glDrawArrays(static_cast<GLenum>(prim), 0, vertexCount); }
    static void draw(Primitives prim, GLint first, GLsizei vertexCount) {
        glDrawArrays(static_cast<GLenum>(prim), first, vertexCount);
    }

    enum FillMode { DrawPoints = GL_POINT, DrawWire = GL_LINE, FillPoly = GL_FILL };

    static void setFillMode(GLenum mode) { glPolygonMode(GL_FRONT_AND_BACK, mode); }
    static void setFillMode(FillMode mode) { glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(mode)); }
    static void drawWireframe() { setFillMode(DrawWire); }

    template <typename T>
    T get(GLenum query) {
        T ret{};
        if constexpr (std::is_same<T, GLint>()) {
            glGetIntegerv(query, &ret);
        }
        else if constexpr (std::is_same<T, GLfloat>()) {
            glGetFloatv(query, &ret);
        }
        else if constexpr (std::is_same<T, GLdouble>()) {
            glGetDoublev(query, &ret);
        }
        else if constexpr (std::is_same<T, GLboolean>()) {
            glGetBooleanv(query, &ret);
        }
        else {
            static_assert(AlwaysFalse<T>, "Invalid type for OpenGL::get");
        }

        return ret;
    }

    static bool isEnabled(GLenum query) { return glIsEnabled(query) != GL_FALSE; }

    static GLint getDrawFramebuffer() { return get<GLint>(GL_DRAW_FRAMEBUFFER_BINDING); }
    static GLint maxSamples() { return get<GLint>(GL_MAX_INTEGER_SAMPLES); }
    static GLint getTex2D() { return get<GLint>(GL_TEXTURE_BINDING_2D); }
    static GLint getProgram() { return get<GLint>(GL_CURRENT_PROGRAM); }
    static bool scissorEnabled() { return isEnabled(GL_SCISSOR_TEST); }

    static bool versionSupported(int major, int minor) { return gl3wIsSupported(major, minor); }

    [[nodiscard]] static GLint uniformLocation(GLuint program, const char* name) {
        return glGetUniformLocation(program, name);
    }
    [[nodiscard]] static GLint uniformLocation(Program& program, const char* name) {
        return glGetUniformLocation(program.handle(), name);
    }

    enum BlendEquation {
        Add = GL_FUNC_ADD,
        Sub = GL_FUNC_SUBTRACT,
        ReverseSub = GL_FUNC_REVERSE_SUBTRACT,
        Min = GL_MIN,
        Max = GL_MAX
    };

    static void setBlendColor(float r, float g, float b, float a = 1.0) { glBlendColor(r, g, b, a); }
    static void setBlendEquation(BlendEquation eq) { glBlendEquation(eq); }
    static void setBlendEquation(BlendEquation eq1, BlendEquation eq2) { glBlendEquationSeparate(eq1, eq2); }

    static void setBlendFactor(GLenum fac1, GLenum fac2) { glBlendFunc(fac1, fac2); }
    static void setBlendFactor(GLenum fac1, GLenum fac2, GLenum fac3, GLenum fac4) {
        glBlendFuncSeparate(fac1, fac2, fac3, fac4);
    }

    // Abstraction for GLSL vectors
    template <typename T, int size>
    class Vector {
        // A GLSL vector can only have 2, 3 or 4 elements
        static_assert(size == 2 || size == 3 || size == 4);
        T m_storage[size];

    public:
        T& r() { return m_storage[0]; }
        T& g() { return m_storage[1]; }
        T& b() {
            static_assert(size >= 3, "Out of bounds OpenGL::Vector access");
            return m_storage[2];
        }
        T& a() {
            static_assert(size >= 4, "Out of bounds OpenGL::Vector access");
            return m_storage[3];
        }

        T& x() { return r(); }
        T& y() { return g(); }
        T& z() { return b(); }
        T& w() { return a(); }
        T& operator[](int index) { return m_storage[index]; }

        T& u() { return r(); }
        T& v() { return g(); }

        T& s() { return r(); }
        T& t() { return g(); }
        T& p() { return b(); }
        T& q() { return a(); }

        Vector(std::array<T, size> list) { std::copy(list.begin(), list.end(), &m_storage[0]); }

        Vector() {}
    };

    using vec2 = Vector<GLfloat, 2>;
    using vec3 = Vector<GLfloat, 3>;
    using vec4 = Vector<GLfloat, 4>;

    using dvec2 = Vector<GLdouble, 2>;
    using dvec3 = Vector<GLdouble, 3>;
    using dvec4 = Vector<GLdouble, 4>;

    using ivec2 = Vector<GLint, 2>;
    using ivec3 = Vector<GLint, 3>;
    using ivec4 = Vector<GLint, 4>;

    using uvec2 = Vector<GLuint, 2>;
    using uvec3 = Vector<GLuint, 3>;
    using uvec4 = Vector<GLuint, 4>;

    // A 2D rectangle, meant to be used for stuff like scissor rects or viewport rects
    // We're never supporting 3D rectangles, because rectangles were never meant to be 3D in the first place
    // x, y: Coords of the top left vertex
    // width, height: Dimensions of the rectangle. Initialized to 0 if not specified.
    template <typename T>
    struct Rectangle {
        T x, y, width, height;

        std::pair<T, T> topLeft() { return std::make_pair(x, y); }
        std::pair<T, T> topRight() { return std::make_pair(x + width, y); }
        std::pair<T, T> bottomLeft() { return std::make_pair(x, y + height); }
        std::pair<T, T> bottomRight() { return std::make_pair(x + width, y + height); }

        Rectangle() : x(0), y(0), width(0), height(0) {}
        Rectangle(T x, T y, T width, T height) : x(x), y(y), width(width), height(height) {}

        bool isEmpty() { return width == 0 && height == 0; }
        bool isLine() { return (width == 0 && height != 0) || (width != 0 && height == 0); }

        void setEmpty() { x = y = width = height = 0; }
    };

    using Rect = Rectangle<GLuint>;

}  // end namespace OpenGL
