#version 450
/*
layout(location = 0) in vec3 fragColor; //Interpolated colour from vertex (location must match)
//different because out

*/
layout ( location = 0) in vec3 fragColor;
layout ( location = 1) in vec2 fragTex;
layout ( location = 2) in vec3 fragNormal;
//layout ( location = 3) in vec3 fragViewVec;
layout ( location = 4) in vec3 fragLigthVec; //light direction

//descriptors
layout (set = 1, binding = 0) uniform sampler2D texSampler;

layout ( location = 0) out vec4 outColor;		//Final output color (must also have location) 

void main () {
	//outColor = vec4 (fragColor, 1.0); //will be hardcoded for now
	vec3 N = normalize(fragNormal);
	vec3 L = normalize(fragLigthVec);
	//vec3 V = normalize(fragViewVec);
	//vec3 R = normalize(dot(V, reflect(L,N)));
	//vec3 R = reflect(L,N);
	//TODO add diffuse color and stuff
	vec3 ambient = vec3(0.5, 0.5, 0.5) * 0.2; // hardcoded for now abmientColor * ambientIntensity
	vec3 diffuse = max(0.0, dot(L,N)) * vec3(1.0, 1.0, 1.0)* 0.7; //(0.2, 0.4, 0.1) -> frag color
	//vec3 specular =pow(max( 0.0, dot(R, V),16.0) * vec3(1.35);


	outColor = texture(texSampler, fragTex) * vec4(fragColor, 1.0f) * vec4(ambient + diffuse, 1.0) ; 
	//outColor +=vec4(ambient + diffuse, 1.0);// + specular
}