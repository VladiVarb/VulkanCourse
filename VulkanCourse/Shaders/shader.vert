#version 450 		//use GLSL 4.5
/*
layout (location = 0) out vec3 fragColor; //Output colour for vertex(location is required)
*/

layout ( location = 0) in vec3 pos; //now get this from utility vertex struct 
layout ( location = 1) in vec3 col;
layout ( location = 2) in vec2 tex;
layout ( location = 3) in vec3 normal;
layout ( set = 0, binding = 0) uniform PV{
	mat4 projection;
	mat4 view;
	vec3 lightDirection;
} pv;
//will use push const model instead
/*
layout (set = 0, binding = 1) uniform UboModel{
	mat4 model;
}uboModel;
*/

layout(push_constant) uniform PushModel{
	mat4 model;	
}pushModel;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTex;
layout (location = 2) out vec3 fragNormal;
//layout (location = 3) out vec3 fragViewVec;
layout (location = 4) out vec3 fragLightVec; //light direction

//can be any function name and specify it in pipeline what function to run first
void main(){
	gl_Position = pv.projection * pv.view * pushModel.model * vec4 (pos, 1.0); //1.0 is tha last val of vec4   positions[gl_VertexIndex]
	//fragColor = colors[gl_VertexIndex];
	//vec4 worldPosition = pushModel.model * vec4(pos,1.0); //???
	fragColor = col;
	fragTex = tex;
	fragNormal = mat3 (pushModel.model) * normal;//mat3(transponse(inverse(pushModel.model)) * normal;
	//fragViewVec = vec3(pv.view * worldPosition);
	fragLightVec = normalize(pv.lightDirection) ;//- vec3(worldPosition);
}