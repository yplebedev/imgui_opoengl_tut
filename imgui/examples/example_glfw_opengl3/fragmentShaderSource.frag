#version 330 core

out vec4 FragColor;
in vec2 texcoord;

uniform sampler2D ourTexture;
uniform sampler2D secondTexture;

vec4 g2lin(vec4 x) {
   return vec4(pow(x.rgb, vec3(2.2)), x.a);
}

vec4 lin2g(vec4 x) {
   return vec4(pow(x.rgb, vec3(0.4545)), x.a);
}

void main() {
   vec4 top = texture(secondTexture, texcoord);
   top = g2lin(top);
   vec4 bottom = texture(ourTexture, texcoord);
   bottom = g2lin(bottom);
   FragColor = vec4(mix(bottom.rgb, top.rgb * bottom.rgb, top.a * 0.8), 1.0);
   FragColor = lin2g(FragColor);
}