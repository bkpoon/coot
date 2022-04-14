
#shader vertex

#version 330 core

// layout(location = 0) in vec3 position; // per vertex for the quad/hex
// layout(location = 1) in vec3 normal;   // per vertex for the quad/hex
// layout(location = 2) in vec4 colour;   // per vertex for the quad/hex
// layout(location = 3) in vec2 texCoord;
// layout(location = 4) in vec3 instance_translation;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec4 colour;
layout(location = 5) in vec2 texCoord;
layout(location = 6) in vec3 instance_translation;

uniform mat4 mvp;
uniform mat4 view_rotation;
uniform float canvas_scale; // 0.8 for happy faces, 0.2 for anchored/fixed atoms
out vec2 texCoord_transfer;

void main() {

   // currently this code correctly places the vertices in
   // perspective mode, but not in orthographic. Baah!
   //
   mat4 t = transpose(view_rotation);
   vec4 p2 = vec4(canvas_scale * position, 1.0); // 1.0 is important here
   vec4 p3 = t * p2;
   vec4 p4 = p3 + vec4(instance_translation, 0.0); // 0.0 is important here

   gl_Position = mvp * p4;
   // gl_Position = vec4(0.000000000000000000005 * position, 0.0);
   texCoord_transfer = texCoord;

}


#shader fragment

#version 330 core

uniform sampler2D face;
in vec2 texCoord_transfer;
uniform float opacity;

out vec4 outputColor;

void main() {

   vec4 sampled = texture(face, texCoord_transfer);

   if (sampled.a < 0.1)
      discard;

   // sampled.a *= opacity; // opacity not correct for anchored atoms marker?
   outputColor = sampled;

   // outputColor = vec4(1,0,1,1);

}

// user defined colouring for Rangana

