#version 330

// Uniforms for RayLib
uniform mat4 mvp; // Model-View-Projection matrix

// Vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
out vec2 fragTexCoord;
out vec2 scale;

void main() {
    gl_Position = mvp * vec4(vertexPosition, 1.0);
    fragTexCoord = vertexTexCoord * 1.0001;
    scale = vec2(1.0, 1.0); // Adjust as needed for scaling
}
