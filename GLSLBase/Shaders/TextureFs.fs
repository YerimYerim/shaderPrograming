#version 450

layout(location=0) out vec4 FragColor;

uniform sampler2D u_TexSampler;
uniform float u_Step;

in vec2 v_TexPos;

const float PI = 3.141592;

vec4 P1()
{
	vec2 newPos = v_TexPos;
	
	newPos.y = abs(newPos.y -0.5) * 2;

	vec4 returnColor = texture(u_TexSampler, newPos);
	return returnColor;
}

vec4 BGR()
{

	vec2 newPos = v_TexPos;

	newPos.x = fract(newPos.x*3);

	newPos.y = floor(v_TexPos.x *3) / 3 + newPos.y /3;

	vec4 returnColor = texture(u_TexSampler, newPos);
	return  returnColor;

}
vec4 RGB()
{

	vec2 newPos = v_TexPos;
	
	newPos.x = fract(newPos.x*3);
	newPos.y = floor(-v_TexPos.x*3)/3 + newPos.y /3;


	vec4 returnColor = texture(u_TexSampler, newPos);
	return  returnColor;

}

vec4 BuckDol()
{

	vec2 newPos = v_TexPos;
	newPos.x = newPos.x*2;
	newPos.y = newPos.y*2;
	
	float x = floor(newPos.x);
	newPos.y = x *0.5 + newPos.y;

	vec4 returnColor = texture(u_TexSampler, newPos);
	


	return returnColor;
}



vec4 P2()
{
	vec2 newTex = v_TexPos;

	newTex.y = newTex.y * 3 ;
	float y = ((ceil(- newTex.y  + 2.0f ) - 1) * 2 )/ 3.0f;
	newTex.y = (v_TexPos.y + y);

	vec4 returnColor = texture(u_TexSampler, newTex);

	return returnColor;
}

vec4 p3()
{
	vec2 newTex = v_TexPos;

	newTex.y = newTex.y * 2;
	newTex.x = newTex.x * 2 ;

	float y = ceil(newTex.y + 1) * 0.5f;
	float x = newTex.x - y;


	vec4 returnColor = texture(u_TexSampler, vec2(x,newTex.y));
	return returnColor;
}

vec4 p4()
{
	vec2 newTex = v_TexPos;

	newTex.y = newTex.y * 2;
	newTex.x = newTex.x * 2 ;

	float y = ceil(newTex.x + 1) * 0.5f;
	float x = newTex.y + y;


	vec4 returnColor = texture(u_TexSampler, vec2(newTex.x,x));
	return returnColor;
}

vec4 SingleTexture()
{
	vec4 newFragColor = texture(u_TexSampler, v_TexPos);
	return newFragColor;
}

vec4 SpriteTexture()
{
	vec2 newTex = vec2(v_TexPos.x, u_Step/6.0 +v_TexPos.y/6.0);
	vec4 newFragColor = texture(u_TexSampler, newTex);
	return newFragColor;
}
void main()
{	

	FragColor = BuckDol();

}
