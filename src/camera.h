#pragma once

#include "prelude.h"

#include "maths.h"
#include "opengl.h"

static vec3 const CAMERA_WORLD_UP = { 0, 1, 0 };
static f32 const CAMERA_FOVY_MIN = 1.0f;
static f32 const CAMERA_FOVY_MAX = 90.0f;

typedef struct Camera {
    vec3 world_up;
    vec3 position;
    // Camera coordinate system.
    vec3 forward;
    vec3 right;
    vec3 up;
    // Euler angles (in degrees).
    f32 pitch;
    f32 yaw;
    // Interactive camera settings.
    f32 movement_speed;
    f32 mouse_sensitivity;
    f32 fovy;
} Camera;

typedef enum CameraMovement {
    CameraMovement_Forward,
    CameraMovement_Backward,
    CameraMovement_Left,
    CameraMovement_Right,
    CameraMovement_Up,
    CameraMovement_Down,
} CameraMovement;

typedef struct CameraMouseEvent {
    f32 xoffset;
    f32 yoffset;
} CameraMouseEvent;

Camera new_camera_at(vec3 const position);

mat4 get_camera_view_matrix(Camera const *camera);

void update_camera_fovy(Camera *camera, f32 yoffset);
void update_camera_angles(Camera *camera, CameraMouseEvent const mouse_event);
void update_camera_position(Camera *camera, CameraMovement const movement, f32 delta_time);