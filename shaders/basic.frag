#version 130

uniform vec4 color;
uniform int bLighting;

in vec3 normalFrag;
out vec4 outColor;

void main()
{
  float lighting = 1.0;
  
  if(bLighting != 0)
  {
    vec3 lightDirection = normalize(vec3(1.0, 2.0, 3.0));

    // Compute simple diffuse directional lighting with some ambient light
    float ambient = 0.2;
    float diffuse = max(0.0, dot(normalize(normalFrag), lightDirection));
    lighting = 0.15f * ambient + 0.85f * diffuse;
  }

  // Modulate color with lighting and apply gamma correction
	outColor = pow(lighting * color, vec4(1.0 / 2.1));
}

