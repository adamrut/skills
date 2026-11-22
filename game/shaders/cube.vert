#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view_projection;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 color;
} push;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec4 out_color;

void main() {
    gl_Position = ubo.view_projection * push.model * vec4(in_position, 1.0);
    out_normal = mat3(push.model) * in_normal;
    out_color = push.color;
}
