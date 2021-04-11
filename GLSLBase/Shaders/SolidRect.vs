#version 450

in vec3 a_Position; 
in vec3 a_Velocity; // float 3개
in float a_EmitTime;
in float a_LifeTime;
in float a_P;
in float a_A;
in float a_RandValue;
uniform float u_Time;
uniform vec3 u_ExForce;
in vec4 a_Color;
const vec3 c_Gravity = vec3(0,-0.1,0);

const mat3 c_NV = mat3(0,-1,0,1,0,0,0,0,0);

out vec4 v_Color;
void main()
{
	float newTime = u_Time - a_EmitTime;

	//vec3 newPos = a_Position;
	vec3 newPos;
	newPos.x = a_Position.x +  cos(a_RandValue * 2 *   3.14);
	newPos.y = a_Position.y + sin(a_RandValue * 2 *   3.14);
	
	newPos.x = a_Position.x + 0.05 * (16*pow(sin(a_RandValue * 2 * 3.14) , 3));
	newPos.y = a_Position.y + 0.05 *(  13 * cos(a_RandValue* 2 *   3.14) - 5 * cos(2*a_RandValue* 2 *   3.14) 
	- 2 * cos(3 * a_RandValue* 2 *   3.14) - cos(4 * a_RandValue* 2 *   3.14));


	vec4 color = vec4(0);
	if(newTime < 0.0)
	{
		newPos = vec3  (10000,10000,10000);
		 
	}
	else
	{
		newTime = mod(newTime, a_LifeTime);
		//newPos = newPos + vec3(newTime,0,0);
		//newPos.y = newPos.y +  a_A * newTime * sin(newTime *3.14f * 2.0f * a_P);
		float tt = newTime* newTime;
		vec3 newACC =  (c_Gravity+u_ExForce);
		vec3 currVel = a_Velocity + newTime * newACC;
		vec3 normalV = normalize(currVel* c_NV);
		newPos = newPos +  newTime * a_Velocity + 0.5 * newACC * tt;
		newPos = newPos +  normalV * a_A * sin(newTime* 2*3.14*a_P);


		float intensity = 1.0f - newTime / a_LifeTime;

		if(	mod (newTime,3) == 0)
		{
			color = vec4(1.0f,1.0f,1.0f,1.0f);
		}
		else
		{

			color = a_Color * intensity;
		}
		
	}
	gl_Position = vec4(newPos , 1); // opengl고유의 출력값
	v_Color = color;
}
 