#version 330 core
out vec4 FragColor;

struct Light {
    vec3 direction;
    vec3 color;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords; 
  
uniform sampler2D sphereTexture;
uniform vec3 viewPos;
uniform Light light;

void main()
{
    // ambient
    vec3 ambient = light.color * 0.1 * texture(sphereTexture, TexCoords).rgb;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.color * diff * texture(sphereTexture, TexCoords).rgb;
    
    //specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = light.color * spec * texture(sphereTexture, TexCoords).rgb;  
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
} 