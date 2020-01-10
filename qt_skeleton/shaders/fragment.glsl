#version 410
// Fragment shader

layout (location = 0) in vec3 vertcoords_camera_fs;
layout (location = 1) in vec3 vertnormal_camera_fs;

uniform bool reflectionLinesEnabled;
uniform float reflectionLinesSize;
uniform bool gaussianEnabled;

out vec4 fColor;

void main() {
    vec3 normal;
    normal = normalize(vertnormal_camera_fs);

  if (reflectionLinesEnabled) {
      // Choose color based on the normal of the surface compared to the z-axis
      if (mod(dot(normal, vec3(0, 0, 1)), reflectionLinesSize) > (reflectionLinesSize/2)) {
        fColor = vec4(1, 1, 1, 1.0);
      } else {
        fColor = vec4(0,0,0, 1.0);
      }
  } else if (gaussianEnabled) {
      // Choose color based on the normal of the surface compared to the different axis
      fColor = vec4(dot(normal, vec3(0, 0, 1)), dot(normal, vec3(0, 1, 0)), dot(normal, vec3(1, 0, 0)), 1);
  } else {
      vec3 lightpos = vec3(3.0, 0.0, 2.0)*10.0;
      vec3 lightcolour = vec3(1.0);

      vec3 matcolour = vec3(0.53, 0.80, 0.87);
      vec3 matspeccolour = vec3(1.0);

      float matambientcoeff = 0.2;
      float matdiffusecoeff = 0.3;
      float matspecularcoeff = 0.8;

      vec3 surftolight = normalize(lightpos - vertcoords_camera_fs);
      float diffusecoeff = max(0.0, dot(surftolight, normal));

      vec3 camerapos = vec3(0.0);
      vec3 surftocamera = normalize(camerapos - vertcoords_camera_fs);

      vec3 reflected = 2.0 * diffusecoeff * normal - surftolight;
      float specularcoeff = max(0.0, dot(reflected, surftocamera));

      vec3 compcolour = min(1.0, matambientcoeff + matdiffusecoeff * diffusecoeff) * lightcolour * matcolour;
           compcolour += matspecularcoeff * specularcoeff * lightcolour * matspeccolour;

      fColor = vec4(compcolour, 1.0);
  }
}
