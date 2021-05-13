#version 450
/*
layout(location = 0) in vec3 fragColor; //Interpolated colour from vertex (location must match)
//different because out

*/
layout(input_attachment_index = 0, binding = 0) uniform subpassInput colorInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput depthInput;

layout(location = 0) out vec4 outColor;
void main () {
	//outColor = vec4 (fragColor, 1.0); //will be hardcoded for now
	//outColor = subpassLoad(colorInput).rgba; //
	//outColor.g = 0.0f;
	int xHalf = 800 / 2;
	float lowerBound = 0.98f;
	float upperBound = 1.0f;
	if(gl_FragCoord.x > xHalf)
	{
		float depth = subpassLoad(depthInput).r;
		float depthScaled = 1.0f - ((depth-lowerBound)/ (upperBound - lowerBound));
		
		outColor= vec4( depthScaled , 0.0f,0.0f,1.0);	//depthScaled
	}
	else
	{
		outColor = subpassLoad(colorInput).rgba; //
	}
}