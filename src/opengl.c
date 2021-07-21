#include "opengl.h"

#include "console.h"
#include "maths.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

static void GLAPIENTRY debug_message_callback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const void *userParam) {
    if (/* id == 131169 || */ id == 131185 || id == 131204 || id == 131218) {
        // 131185: Buffer detailed info
        // 131204: Texture state usage warning
        // 131218: Program/shader state performance warning
        return; // ignore non-significant error / warning codes
    }
    char const *source_string =
        ((source == GL_DEBUG_SOURCE_API)               ? "API"
         : (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM)   ? "Window System"
         : (source == GL_DEBUG_SOURCE_SHADER_COMPILER) ? "Shader Compiler"
         : (source == GL_DEBUG_SOURCE_THIRD_PARTY)     ? "Third Party"
         : (source == GL_DEBUG_SOURCE_APPLICATION)     ? "Application"
         : (source == GL_DEBUG_SOURCE_OTHER)           ? "Other"
                                                       : "");
    char const *type_string =
        ((type == GL_DEBUG_TYPE_ERROR)                 ? "Error"
         : (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR) ? "Deprecated Behaviour"
         : (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)  ? "Undefined Behaviour"
         : (type == GL_DEBUG_TYPE_PORTABILITY)         ? "Portability"
         : (type == GL_DEBUG_TYPE_PERFORMANCE)         ? "Performance"
         : (type == GL_DEBUG_TYPE_MARKER)              ? "Marker"
         : (type == GL_DEBUG_TYPE_PUSH_GROUP)          ? "Push Group"
         : (type == GL_DEBUG_TYPE_POP_GROUP)           ? "Pop Group"
         : (type == GL_DEBUG_TYPE_OTHER)               ? "Other"
                                                       : "");
    char const *severity_string =
        ((severity == GL_DEBUG_SEVERITY_HIGH)           ? "High"
         : (severity == GL_DEBUG_SEVERITY_MEDIUM)       ? "Medium"
         : (severity == GL_DEBUG_SEVERITY_LOW)          ? "Low"
         : (severity == GL_DEBUG_SEVERITY_NOTIFICATION) ? "Notification"
                                                        : "");
    GLOW_WARNING(
        "OpenGL debug message %u: raised from '%s' with type '%s' and '%s' severity: `%s`",
        id,
        source_string,
        type_string,
        severity_string,
        message);
}

static void error_callback(int error, char const *description) {
    GLOW_WARNING("glfw error %d: `%s`", error, description);
}

GLFWwindow *init_opengl(WindowSettings settings, Err *err) {
    if (*err) { return NULL; }

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) { return (*err = Err_Glfw_Init, NULL); }

    // https://www.glfw.org/docs/latest/window.html#window_hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, settings.msaa);
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    GLFWwindow *window = NULL;
    if (settings.fullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        GLFWvidmode const *mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        window = glfwCreateWindow(mode->width, mode->height, "glow", monitor, NULL);
    } else {
        window = glfwCreateWindow(settings.width, settings.height, "glow", NULL, NULL);
    }

    if (!window) { return (*err = Err_Glfw_Window, NULL); }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(settings.vsync ? 1 : 0);
    if (settings.set_callbacks_fn) { settings.set_callbacks_fn(window); }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // @Cleanup

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        return (*err = Err_Glad_Init, NULL);
    }

    glEnable(GL_DEPTH_TEST); // @Cleanup
    if (settings.msaa > 0) { glEnable(GL_MULTISAMPLE); }

    int flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debug_message_callback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
    }

    GLOW_LOG("GL_VERSION = %s", (char *) glGetString(GL_VERSION));
    GLOW_LOG("GL_RENDERER = %s", (char *) glGetString(GL_RENDERER));

    return window;
}

bool shader_compile_success(uint shader, char info_log[INFO_LOG_LENGTH], Err *err) {
    if (*err) { return false; }

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        glGetShaderInfoLog(shader, INFO_LOG_LENGTH, NULL, info_log);
        info_log[CLAMP(length - 1, 0, INFO_LOG_LENGTH - 1)] = '\0';
        return (*err = Err_Shader_Compile, false);
    }

    return true;
}

bool program_link_success(uint program, char info_log[INFO_LOG_LENGTH], Err *err) {
    if (*err) { return false; }

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        glGetProgramInfoLog(program, INFO_LOG_LENGTH, NULL, info_log);
        info_log[CLAMP(length - 1, 0, INFO_LOG_LENGTH - 1)] = '\0';
        return (*err = Err_Shader_Link, false);
    }

    return true;
}

int get_uniform_location(uint program, char const *name) {
    int const loc = glGetUniformLocation(program, name);
    if (loc == -1) { GLOW_WARNING("failed to find uniform location for: `%s`", name); }
    return loc;
}

int get_attribute_location(uint program, char const *name) {
    int const loc = glGetAttribLocation(program, name);
    if (loc == -1) { GLOW_WARNING("failed to find attribute location for: `%s`", name); }
    return loc;
}
