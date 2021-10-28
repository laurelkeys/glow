#include "shader.h"

#include "console.h"
#include "file.h"
#include "opengl.h"

#include <stdio.h>

#include <glad/glad.h>

static uint make_shader(uint type, char const *source, char info_log[INFO_LOG_LENGTH], Err *err) {
    if (*err) { return 0; }

    uint const id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);
    if (!is_shader_compile_success(id, info_log, err)) {
        char const *type_string =
            (type == GL_VERTEX_SHADER     ? "vertex"
             : type == GL_FRAGMENT_SHADER ? "fragment"
             : type == GL_GEOMETRY_SHADER ? "geometry"
                                          : "");
        GLOW_WARNING("%s shader compilation failed with: `\n%s`", type_string, info_log);
    }

    return id;
}

Shader new_shader_from_source(ShaderSources const source, Err *err) {
    if (*err) { return (Shader) { 0 }; }

    char info_log[INFO_LOG_LENGTH] = { 0 };

    bool const has_geometry_shader = source.geometry != NULL;

    uint const vertex_id = make_shader(GL_VERTEX_SHADER, source.vertex, info_log, err);
    uint const fragment_id = make_shader(GL_FRAGMENT_SHADER, source.fragment, info_log, err);
    uint const geometry_id =
        has_geometry_shader ? make_shader(GL_GEOMETRY_SHADER, source.geometry, info_log, err) : 0;

    uint const program_id = glCreateProgram();
    glAttachShader(program_id, vertex_id);
    glAttachShader(program_id, fragment_id);
    if (has_geometry_shader) { glAttachShader(program_id, geometry_id); }

    glLinkProgram(program_id);
    if (!is_program_link_success(program_id, info_log, err)) {
        GLOW_WARNING("shader program linking failed with: `\n%s`", info_log);
    }

    glDeleteShader(geometry_id);
    glDeleteShader(fragment_id);
    glDeleteShader(vertex_id);

    return (Shader) { program_id };
}

Shader new_shader_from_filepath(ShaderFilepaths const path, Err *err) {
    if (*err) { return (Shader) { 0 }; }

    bool const has_geometry_shader = path.geometry != NULL;

    char *vertex_source = alloc_data_from_filepath(path.vertex, err);
    char *fragment_source = alloc_data_from_filepath(path.fragment, err);
    char *geometry_source =
        has_geometry_shader ? alloc_data_from_filepath(path.geometry, err) : NULL;

    Shader const shader = new_shader_from_source(
        (ShaderSources) { vertex_source, fragment_source, geometry_source }, err);

    free(geometry_source);
    free(fragment_source);
    free(vertex_source);

    return shader;
}

static Err try_reload_shader(
    Shader *shader,
    Shader (*new_shader_fn)(ShaderStrings const, Err *),
    ShaderStrings const new_shader_arg) {
    Err err = Err_None;

    Shader const new_shader = new_shader_fn(new_shader_arg, &err);
    if (err == Err_None) {
        glDeleteProgram(shader->program_id);
        shader->program_id = new_shader.program_id;
    }

    return err;
}
bool try_reload_shader_from_source(Shader *shader, ShaderSources const source) {
    Err const err = try_reload_shader(shader, new_shader_from_source, source);
    return err == Err_None;
}
bool try_reload_shader_from_filepath(Shader *shader, ShaderFilepaths const path) {
    Err const err = try_reload_shader(shader, new_shader_from_filepath, path);
    return err == Err_None;
}

void use_shader(Shader const shader) {
    glUseProgram(shader.program_id);
}

/* clang-format off */
void set_shader_int(Shader const shader, char const *name, int value) { glUniform1i(get_uniform_location(shader.program_id, name), value); }
void set_shader_bool(Shader const shader, char const *name, bool value) { glUniform1i(get_uniform_location(shader.program_id, name), (int) value); }
void set_shader_float(Shader const shader, char const *name, f32 value) { glUniform1f(get_uniform_location(shader.program_id, name), value); }

void set_shader_sampler2D(Shader const shader, char const *name, uint texture_unit) {
    assert(texture_unit >= GL_TEXTURE0);
    glUniform1i(get_uniform_location(shader.program_id, name), (int) (texture_unit - GL_TEXTURE0));
}

void set_shader_vec2(Shader const shader, char const *name, vec2 const vec) { glUniform2fv(get_uniform_location(shader.program_id, name), 1, (f32 *) &vec); }
void set_shader_vec3(Shader const shader, char const *name, vec3 const vec) { glUniform3fv(get_uniform_location(shader.program_id, name), 1, (f32 *) &vec); }
void set_shader_vec4(Shader const shader, char const *name, vec4 const vec) { glUniform4fv(get_uniform_location(shader.program_id, name), 1, (f32 *) &vec); }

void set_shader_mat3(Shader const shader, char const *name, mat3 const mat) { glUniformMatrix3fv(get_uniform_location(shader.program_id, name), 1, GL_FALSE, &mat.m[0][0]); }
void set_shader_mat4(Shader const shader, char const *name, mat4 const mat) { glUniformMatrix4fv(get_uniform_location(shader.program_id, name), 1, GL_FALSE, &mat.m[0][0]); }
/* clang-format on */
