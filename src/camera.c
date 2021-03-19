#include "camera.h"

#include "maths.h"

Camera new_camera_at(vec3 const position) {
    vec3 const forward = { 0, 0, -1 }; // pitch = 0, yaw = -90
    vec3 const right = vec3_normalize(vec3_cross(forward, CAMERA_WORLD_UP));
    vec3 const up = vec3_cross(right, forward);
    return (Camera) {
        CAMERA_WORLD_UP,
        position,
        forward,
        right,
        up,

        .pitch = 0,
        .yaw = -90,

        .movement_speed = 2.5f,
        .mouse_sensitivity = 0.1f,
        .fovy = 45.0f,
    };
}

mat4 get_camera_view_matrix(Camera const *camera) {
    return mat4_lookat(camera->position, vec3_add(camera->position, camera->forward), camera->up);
}

static void update_camera_coordinate_system(Camera *camera) {
    f32 const yaw = RADIANS(camera->yaw);
    f32 const pitch = RADIANS(camera->pitch);
    camera->forward = vec3_normalize((vec3) {
        .x = cosf(yaw) * cosf(pitch),
        .y = sinf(pitch),
        .z = sinf(yaw) * cosf(pitch),
    });
    camera->right = vec3_normalize(vec3_cross(camera->forward, camera->world_up));
    camera->up = vec3_cross(camera->right, camera->forward);
}

void update_camera_fovy(Camera *camera, f32 yoffset) {
    camera->fovy -= yoffset;
    camera->fovy = CLAMP(camera->fovy, CAMERA_FOVY_MIN, CAMERA_FOVY_MAX);
}
void update_camera_angles(Camera *camera, CameraMouseEvent const mouse_event) {
    camera->pitch += mouse_event.yoffset * camera->mouse_sensitivity;
    camera->pitch = CLAMP(camera->pitch, -89.0f, 89.0f);
    camera->yaw += mouse_event.xoffset * camera->mouse_sensitivity;
    update_camera_coordinate_system(camera);
}
void update_camera_position(Camera *camera, CameraMovement const movement, f32 delta_time) {
    f32 const speed = camera->movement_speed * delta_time; // frame-rate independent
    vec3 const forward_backward = vec3_scl(camera->forward, speed);
    vec3 const right_left = vec3_scl(camera->right, speed);
    vec3 const up_down = vec3_scl(camera->up, speed);
    // clang-format off
    switch (movement) {
        case CameraMovement_Forward:  camera->position = vec3_add(camera->position, forward_backward); break;
        case CameraMovement_Backward: camera->position = vec3_sub(camera->position, forward_backward); break;
        case CameraMovement_Left:     camera->position = vec3_sub(camera->position, right_left); break;
        case CameraMovement_Right:    camera->position = vec3_add(camera->position, right_left); break;
        case CameraMovement_Up:       camera->position = vec3_add(camera->position, up_down); break;
        case CameraMovement_Down:     camera->position = vec3_sub(camera->position, up_down); break;
    }
    // clang-format on
}
