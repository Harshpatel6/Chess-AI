#version 130 //Shader Preprocessor - Released August 2008 - version 3.0
// This fragment shader operates on every pixel in a polygon

//Received from colorShading.vert
in vec2 fragmentPosition;
in vec4 fragmentColor;
in vec2 fragmentUV;

// The following effects whats drawn onscreen
out vec4 color;

//uniform float time; //Uniform is used to make the variable constant over all files
uniform sampler2D sampler;

void main(){
	//UV cordinates: xy coordinates for textures ranging from 0 to 1(U = x, V = y)
	vec4 textureColor= texture(sampler,fragmentUV);

	color = textureColor * fragmentColor;
	
	//vec4(fragmentColor.r * (cos(fragmentPosition.x * 4.0 + time) + 1.0) * 0.5,
			//	 fragmentColor.g * (cos(fragmentPosition.y * 8.0 + time) + 1.0) * 0.5,
			//	 fragmentColor.b * (cos(fragmentPosition.x * 2.0 + time) + 1.0) * 0.5, fragmentColor.a);
	
}