#include "shader.h"

#include "console.h"
#include "opengl.h"

#include <stdio.h>

#define SHADER_TYPE(shader_type)                        \
    ((shader_type) == GL_VERTEX_SHADER     ? "vertex"   \
     : (shader_type) == GL_FRAGMENT_SHADER ? "fragment" \
                                           : "????")

static uint init_shader(uint type, char const *source, char info_log[INFO_LOG_LENGTH], Err *err) {
    uint const id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);
    if (!shader_compile_success(id, info_log, err)) {
        GLOW_WARNING("%s shader compilation failed with ```\n%s```", SHADER_TYPE(type), info_log);
    }
    return id;
}

#undef SHADER_TYPE

Shader new_shader_from_source(char const *vertex_source, char const *fragment_source, Err *err) {
    char info_log[INFO_LOG_LENGTH] = { 0 };

    uint const vertex_id = init_shader(GL_VERTEX_SHADER, vertex_source, info_log, err);
    uint const fragment_id = init_shader(GL_FRAGMENT_SHADER, fragment_source, info_log, err);

    uint const program_id = glCreateProgram();
    glAttachShader(program_id, vertex_id);
    glAttachShader(program_id, fragment_id);
    glLinkProgram(program_id);
    if (!program_link_success(program_id, info_log, err)) {
        GLOW_WARNING("shader program linking failed with ```\n%s```", info_log);
    }

    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);

    return (Shader) { program_id, vertex_id, fragment_id };
}

static char *alloc_data_from_filepath(char const *path, Err *err) {
    FILE *fp = fopen(path, "rb");
    if (!fp) { return (*err = Err_Fopen, NULL); }

    fseek(fp, 0, SEEK_END);
    long const fsize = ftell(fp);
    rewind(fp); // fseek(fp, 0, SEEK_SET);
    char *data = calloc(fsize + 1, sizeof(char));

    if (data) {
        fread(data, 1, fsize, fp);
    } else {
        *err = Err_Calloc;
    }

    fclose(fp);
    return data;
}

Shader new_shader_from_filepath(char const *vertex_path, char const *fragment_path, Err *err) {
    char *vertex_source = alloc_data_from_filepath(vertex_path, err);
    if (*err) { return (Shader) { 0 }; }

    char *fragment_source = alloc_data_from_filepath(fragment_path, err);
    if (*err) { return (Shader) { 0 }; }

    Shader const shader = new_shader_from_source(vertex_source, fragment_source, err);

    free(fragment_source);
    free(vertex_source);

    return shader;
}

static bool reload_shader(
    Shader *shader,
    Shader (*new_shader_fn)(char const *, char const *, Err *),
    char const *vertex,
    char const *fragment) {
    Err err = Err_None;
    Shader const new_shader = new_shader_fn(vertex, fragment, &err);
    if (err) { return false; }

    glDeleteProgram(shader->program_id);
    shader->program_id = new_shader.program_id;
    // @Todo: accept NULL args to reuse old ids.
    shader->vertex_id = new_shader.vertex_id;
    shader->fragment_id = new_shader.fragment_id;

    return true;
}
bool reload_shader_from_source(
    Shader *shader, char const *vertex_source, char const *fragment_source) {
    return reload_shader(shader, new_shader_from_source, vertex_source, fragment_source);
}
bool reload_shader_from_filepath(
    Shader *shader, char const *vertex_path, char const *fragment_path) {
    return reload_shader(shader, new_shader_from_filepath, vertex_path, fragment_path);
}

void use_shader(Shader const shader) {
    glUseProgram(shader.program_id);
}

// clang-format off
void set_shader_int(Shader const shader, char const *name, int value) { glUniform1i(get_uniform_location(shader.program_id, name), value); }
void set_shader_bool(Shader const shader, char const *name, bool value) { glUniform1i(get_uniform_location(shader.program_id, name), (int) value); }
void set_shader_float(Shader const shader, char const *name, f32 value) { glUniform1f(get_uniform_location(shader.program_id, name), value); }

void set_shader_vec2(Shader const shader, char const *name, vec2 const vec) { glUniform2fv(get_uniform_location(shader.program_id, name), 1, (f32 *) &vec); }
void set_shader_vec3(Shader const shader, char const *name, vec3 const vec) { glUniform3fv(get_uniform_location(shader.program_id, name), 1, (f32 *) &vec); }
void set_shader_vec4(Shader const shader, char const *name, vec4 const vec) { glUniform4fv(get_uniform_location(shader.program_id, name), 1, (f32 *) &vec); }

void set_shader_mat3(Shader const shader, char const *name, mat3 const mat) { glUniformMatrix3fv(get_uniform_location(shader.program_id, name), 1, GL_FALSE, &mat.m[0][0]); }
void set_shader_mat4(Shader const shader, char const *name, mat4 const mat) { glUniformMatrix4fv(get_uniform_location(shader.program_id, name), 1, GL_FALSE, &mat.m[0][0]); }
// clang-format on
