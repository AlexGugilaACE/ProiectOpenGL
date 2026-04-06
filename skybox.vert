#version 330 core
layout (location = 0) in vec3 aPos;
out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos;
    // Elimin?m transla?ia din matricea view pentru ca cerul s? nu se mi?te cu camera
    mat4 staticView = mat4(mat3(view)); 
    vec4 pos = projection * staticView * vec4(aPos, 1.0);
    // For??m cerul s? fie desenat la "marginea" maxim? a ecranului
    gl_Position = pos.xyww;
}