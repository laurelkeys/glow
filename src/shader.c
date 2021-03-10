#include "shader.h"

#include "console.h"
#include "opengl.h"

#include <stdio.h>

Shader new_shader_from_source(char const *vertex_source, char const *fragment_source, Err *err) {
    char info_log[INFO_LOG_LENGTH];

    uint const vertex_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_id, 1, &vertex_source, NULL);
    glCompileShader(vertex_id);
    if (!shader_compile_success(vertex_id, info_log, err)) {
        GLOW_WARNING("vertex shader compilation failed with ```\n%s```", info_log);
    }

    uint const fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_id, 1, &fragment_source, NULL);
    glCompileShader(fragment_id);
    if (!shader_compile_success(fragment_id, info_log, err)) {
        GLOW_WARNING("fragment shader compilation failed with ```\n%s```", info_log);
    }

    uint const program_id = glCreateProgram();
    glAttachShader(program_id, vertex_id);
    glAttachShader(program_id, fragment_id);
    glLinkProgram(program_id);
    if (!program_link_success(program_id, info_log, err)) {
        GLOW_WARNING("shader program linking failed with ```\n%s```", info_log);
    }

    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);

    return (Shader) { program_id };
}

Shader new_shader_from_filepath(char const *vertex_path, char const *fragment_path, Err *err) {
#define READ_SHADER(fp, shader_path, shader_source) \
    char *shader_source = NULL;                     \
    FILE *fp = fopen(shader_path, "rb");            \
    if (fp) {                                       \
        fseek(fp, 0, SEEK_END);                     \
        long fsize = ftell(fp);                     \
        fseek(fp, 0, SEEK_SET);                     \
        shader_source = malloc(fsize + 1);          \
        if (shader_source) {                        \
            fread(shader_source, 1, fsize, fp);     \
            shader_source[fsize] = '\0';            \
        }                                           \
        fclose(fp);                                 \
    } else {                                        \
        return (*err = Err_Fopen, (Shader) { 0 });  \
    }

    READ_SHADER(vs, vertex_path, vertex_source);
    READ_SHADER(fs, fragment_path, fragment_source);

#undef READ_SHADER

    if (!vertex_source || !fragment_source) { return (*err = Err_Malloc, (Shader) { 0 }); }

    return new_shader_from_source(vertex_source, fragment_source, err);
}

void use_shader(Shader const shader) {
    glUseProgram(shader.program_id);
}

static int get_uniform_location(Shader const shader, char const *name) {
    int const loc = glGetUniformLocation(shader.program_id, name);
    if (loc == -1) { GLOW_WARNING("failed to find uniform location for: '%s'", name); }
    return loc;
}

void set_shader_int(Shader const shader, char const *name, int value) {
    glUniform1i(get_uniform_location(shader, name), value);
}
void set_shader_bool(Shader const shader, char const *name, bool value) {
    glUniform1i(get_uniform_location(shader, name), (int) value);
}
void set_shader_float(Shader const shader, char const *name, f32 value) {
    glUniform1f(get_uniform_location(shader, name), value);
}
