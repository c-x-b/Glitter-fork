//#include<Header.glsl>

out vec4 FragColor;

struct Light {
    vec3 direction;
    vec3 color;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords; 
in vec3 scatteringResult;
in vec3 groundAttenuate;
in float cameraDistance;
  
uniform sampler2D sphereTexture;
uniform sampler2D sphereSpecularTexture;
uniform Light light;

void main()
{
    // ambient
    //vec3 ambient = light.color * 0.1 * texture(sphereTexture, TexCoords).rgb;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * texture(sphereTexture, TexCoords).rgb;
    
    //specular
    // vec3 viewDir = normalize(viewPos - FragPos);
    // vec3 reflectDir = reflect(-lightDir, norm);  
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    // vec3 specular = spec * texture(sphereTexture, TexCoords).rgb;  
    vec3 specular = vec3(1.0, 0.941, 0.898) * pow(diff, 32) * 0.6 * texture(sphereSpecularTexture, TexCoords).g;

    vec3 result = diffuse + specular;

    vec3 scatteringColor = mix(scatteringResult * 0.01, scatteringResult * 0.3, (diff + 0.1) * 5.0);
    vec3 groundColor = mix(groundAttenuate * 0.01, groundAttenuate * 0.3, (diff + 0.1) * 5.0) * result;
        
    FragColor = vec4(scatteringColor + groundColor, 1.0);
} 