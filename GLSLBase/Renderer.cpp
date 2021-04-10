#include "stdafx.h"
#include "Renderer.h"
#include "LoadPng.h"
#include <Windows.h>
#include <cstdlib>
#include <cassert>

Renderer::Renderer(int windowSizeX, int windowSizeY)
{
	//default settings
	glClearDepth(1.f);

	Initialize(windowSizeX, windowSizeY);
}


Renderer::~Renderer()
{
}

void Renderer::Initialize(int windowSizeX, int windowSizeY)
{
	//Set window size
	m_WindowSizeX = windowSizeX;
	m_WindowSizeY = windowSizeY;

	//Load shaders
	m_SolidRectShader = CompileShaders("./Shaders/SolidRect.vs", "./Shaders/SolidRect.fs");
	
	//Create VBOs
	CreateVertexBufferObjects();

	//Initialize camera settings
	m_v3Camera_Position = glm::vec3(0.f, 0.f, 1000.f);
	m_v3Camera_Lookat = glm::vec3(0.f, 0.f, 0.f);
	m_v3Camera_Up = glm::vec3(0.f, 1.f, 0.f);
	m_m4View = glm::lookAt(
		m_v3Camera_Position,
		m_v3Camera_Lookat,
		m_v3Camera_Up
	);

	//Initialize projection matrix
	m_m4OrthoProj = glm::ortho(
		-(float)windowSizeX / 2.f, (float)windowSizeX / 2.f,
		-(float)windowSizeY / 2.f, (float)windowSizeY / 2.f,
		0.0001f, 10000.f);
	m_m4PersProj = glm::perspectiveRH(45.f, 1.f, 1.f, 1000.f);

	//Initialize projection-view matrix
	m_m4ProjView = m_m4OrthoProj * m_m4View; //use ortho at this time
	//m_m4ProjView = m_m4PersProj * m_m4View;

	//Initialize model transform matrix :; used for rotating quad normal to parallel to camera direction
	m_m4Model = glm::rotate(glm::mat4(1.0f), glm::radians(0.f), glm::vec3(1.f, 0.f, 0.f));

	//create test data
	float tempVertices[] = { 0.f,0.f,0.f,1.f,0.f,0.f,1.f,1.f,0.f }; // <-- 데이터쪼가리에 불과하다
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(tempVertices), tempVertices, GL_STATIC_DRAW);

	float tempVertices1[] = { 0.f,0.f,0.f,-1.f,0.f,0.f,-1.f,1.f,0.f }; // <-- 데이터쪼가리에 불과하다
	glGenBuffers(1, &m_VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tempVertices1), tempVertices1, GL_STATIC_DRAW);

	CreateParticle(5000);
}

void Renderer::CreateVertexBufferObjects()
{
	float rect[]
		=
	{
		-0.5, -0.5, 0.f, -0.5, 0.5, 0.f, 0.5, 0.5, 0.f, //Triangle1
		-0.5, -0.5, 0.f,  0.5, 0.5, 0.f, 0.5, -0.5, 0.f, //Triangle2
	};

	glGenBuffers(1, &m_VBORect);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);
}

void Renderer::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	//쉐이더 오브젝트 생성
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = (GLint)strlen(pShaderText);
	//쉐이더 코드를 쉐이더 오브젝트에 할당
	glShaderSource(ShaderObj, 1, p, Lengths);

	//할당된 쉐이더 코드를 컴파일
	glCompileShader(ShaderObj);

	GLint success;
	// ShaderObj 가 성공적으로 컴파일 되었는지 확인
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];

		//OpenGL 의 shader log 데이터를 가져옴
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		printf("%s \n", pShaderText);
	}

	// ShaderProgram 에 attach!!
	glAttachShader(ShaderProgram, ShaderObj);
}

bool Renderer::ReadFile(char* filename, std::string *target)
{
	std::ifstream file(filename);
	if (file.fail())
	{
		std::cout << filename << " file loading failed.. \n";
		file.close();
		return false;
	}
	std::string line;
	while (getline(file, line)) {
		target->append(line.c_str());
		target->append("\n");
	}
	return true;
}

GLuint Renderer::CompileShaders(char* filenameVS, char* filenameFS)
{
	GLuint ShaderProgram = glCreateProgram(); //빈 쉐이더 프로그램 생성

	if (ShaderProgram == 0) { //쉐이더 프로그램이 만들어졌는지 확인
		fprintf(stderr, "Error creating shader program\n");
	}

	std::string vs, fs;

	//shader.vs 가 vs 안으로 로딩됨
	if (!ReadFile(filenameVS, &vs)) {
		printf("Error compiling vertex shader\n");
		return -1;
	};

	//shader.fs 가 fs 안으로 로딩됨
	if (!ReadFile(filenameFS, &fs)) {
		printf("Error compiling fragment shader\n");
		return -1;
	};

	// ShaderProgram 에 vs.c_str() 버텍스 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	// ShaderProgram 에 fs.c_str() 프레그먼트 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	//Attach 완료된 shaderProgram 을 링킹함
	glLinkProgram(ShaderProgram);

	//링크가 성공했는지 확인
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (Success == 0) {
		// shader program 로그를 받아옴
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error linking shader program\n" << ErrorLog;
		return -1;
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error validating shader program\n" << ErrorLog;
		return -1;
	}

	glUseProgram(ShaderProgram);
	std::cout << filenameVS << ", " << filenameFS << " Shader compiling is done.\n";

	return ShaderProgram;
}
unsigned char * Renderer::loadBMPRaw(const char * imagepath, unsigned int& outWidth, unsigned int& outHeight)
{
	std::cout << "Loading bmp file " << imagepath << " ... " << std::endl;
	outWidth = -1;
	outHeight = -1;
	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = NULL;
	fopen_s(&file, imagepath, "rb");
	if (!file)
	{
		std::cout << "Image could not be opened, " << imagepath << " is missing. " << std::endl;
		return NULL;
	}

	if (fread(header, 1, 54, file) != 54)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	if (header[0] != 'B' || header[1] != 'M')
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	if (*(int*)&(header[0x1E]) != 0)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	if (*(int*)&(header[0x1C]) != 24)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	outWidth = *(int*)&(header[0x12]);
	outHeight = *(int*)&(header[0x16]);

	if (imageSize == 0)
		imageSize = outWidth * outHeight * 3;

	if (dataPos == 0)
		dataPos = 54;

	data = new unsigned char[imageSize];

	fread(data, 1, imageSize, file);

	fclose(file);

	std::cout << imagepath << " is succesfully loaded. " << std::endl;

	return data;
}

void Renderer::CreateParticle(int count)
{
	float* particleVertices = new float[count * (3 + 3 + 1 + 1+ 1 + 1 +1 + 4 ) * 3 * 2];
	// 생성할 파티클 개수 * (버텍스당 float point+속도를 위한 벡터 + emittime + 랜덤 라이프타임 + 랜덤폭 +진폭 + 벨류 + 컬러(4개)) * 삼각형을 위한 버텍스 개수  * 사각형을 위한 삼각형의 개수

	int floatCount = count * (3 + 3 + 1 + 1 + 1 + 1+1 + 4) * 3 * 2;
	int vertexCount = count * 3 * 3 * 2; //for drawarrays 인자값을 위한 ㅓ택스 개수
	 
	int index = 0;   
	float particleSize = 0.01f;

	for (int i = 0; i < count; i++)
	{
		float randomValueX = 0.f;
		float randomValueY = 0.f;
		float randomValueZ = 0.f;

		float randomValueVX = 0.f;
		float randomValueVY = 0.f;
		float randomValueVZ = 0.f;

		float randomEmitTime = 0.f;
		float randomLifeTime = 2.f;

		float randomPeriod = 1.f;
		float randomAmp = 1.f;
		float randomvalue = 1.f;
		
		//색상 랜덤
		float randR = 0.0f;
		float randG = 0.0f;
		float randB = 0.0f;
		float randA = 0.0f;
		//randomValueX = (rand()/(float)RAND_MAX - 0.5)*2.f;
		//randomValueY = (rand()/(float)RAND_MAX - 0.5)*2.f;
		randomValueVX = (rand() / (float)RAND_MAX - 0.5) * 0.1f;
		randomValueVY = (rand() / (float)RAND_MAX - 0.5) * 0.1f;

		randomEmitTime = (rand() / (float)RAND_MAX ) * 10.f;
		randomPeriod = (rand() / (float)RAND_MAX ) * 10.f + 1.0f;
		randomAmp	= (rand() / (float)RAND_MAX ) * 0.02f - 0.01f;
		randomLifeTime = (rand() / (float)RAND_MAX) * 0.5f;
		randomvalue = (rand() / (float)RAND_MAX);

		randR = (rand() / (float)RAND_MAX);
		randG = (rand() / (float)RAND_MAX);
		randB = (rand() / (float)RAND_MAX);
		randA = (rand() / (float)RAND_MAX);
		// v0 position
		particleVertices[index] = -particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = -particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;


		// velocity

		particleVertices[index] = randomValueVX;
		index++;				  
		particleVertices[index] = randomValueVY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		particleVertices[index] = randomEmitTime;
		index++;
		particleVertices[index] = randomLifeTime;
		index++;

		particleVertices[index] = randomPeriod;
		index++;
		particleVertices[index] = randomAmp;
		index++;
		particleVertices[index] = randomvalue;
		index++;
		particleVertices[index] = randR;
		index++;

		particleVertices[index] = randG;
		index++;
		particleVertices[index] = randB;
		index++;
		particleVertices[index] = randA;
		index++;
		//v1
		particleVertices[index] = particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = -particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;

		particleVertices[index] = randomValueVX;
		index++;
		particleVertices[index] = randomValueVY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		particleVertices[index] = randomEmitTime;
		index++;
		particleVertices[index] = randomLifeTime;
		index++;
		particleVertices[index] = randomPeriod;
		index++;				  
		particleVertices[index] = randomAmp;
		index++;
		particleVertices[index] = randomvalue;
		index++;
		particleVertices[index] = randR;
		index++;

		particleVertices[index] = randG;
		index++;
		particleVertices[index] = randB;
		index++;
		particleVertices[index] = randA;
		index++;
		//v2
		particleVertices[index] = particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;

		particleVertices[index] = randomValueVX;
		index++;
		particleVertices[index] = randomValueVY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		particleVertices[index] = randomEmitTime;
		index++;
		particleVertices[index] = randomLifeTime;
		index++;

		particleVertices[index] = randomPeriod;
		index++;
		particleVertices[index] = randomAmp;
		index++;
		particleVertices[index] = randomvalue;
		index++;

		particleVertices[index] = randR;
		index++;

		particleVertices[index] = randG;
		index++;
		particleVertices[index] = randB;
		index++;
		particleVertices[index] = randA;
		index++;
		//v3
		particleVertices[index] = -particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = -particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;


		particleVertices[index] = randomValueVX;
		index++;
		particleVertices[index] = randomValueVY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		particleVertices[index] = randomEmitTime;
		index++;
		particleVertices[index] = randomLifeTime;
		index++;

		particleVertices[index] = randomPeriod;
		index++;
		particleVertices[index] = randomAmp;
		index++;

		particleVertices[index] = randomvalue;
		index++;
		particleVertices[index] = randR;
		index++;

		particleVertices[index] = randG;
		index++;
		particleVertices[index] = randB;
		index++;
		particleVertices[index] = randA;
		index++;
		//v4
		particleVertices[index] = particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;


		particleVertices[index] = randomValueVX;
		index++;
		particleVertices[index] = randomValueVY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		particleVertices[index] = randomEmitTime;
		index++;
		particleVertices[index] = randomLifeTime;
		index++;

		particleVertices[index] = randomPeriod;
		index++;
		particleVertices[index] = randomAmp;
		index++;

		particleVertices[index] = randomvalue;
		index++;

		particleVertices[index] = randR;
		index++;

		particleVertices[index] = randG;
		index++;
		particleVertices[index] = randB;
		index++;
		particleVertices[index] = randA;
		index++;
		//v5
		particleVertices[index] = -particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;


		particleVertices[index] = randomValueVX;
		index++;
		particleVertices[index] = randomValueVY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		particleVertices[index] = randomEmitTime;
		index++;
		particleVertices[index] = randomLifeTime;
		index++;

		particleVertices[index] = randomPeriod;
		index++;
		particleVertices[index] = randomAmp;
		index++;

		particleVertices[index] = randomvalue;
		index++;

		particleVertices[index] = randR;
		index++;

		particleVertices[index] = randG;
		index++;
		particleVertices[index] = randB;
		index++;
		particleVertices[index] = randA;
		index++;
		glGenBuffers(1, &m_VBOManyPrticle);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyPrticle);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatCount, particleVertices, GL_STATIC_DRAW);
		m_VBOManyPrticleCount = vertexCount;
	}
}

GLuint Renderer::CreatePngTexture(char * filePath)
{
	//Load Pngs: Load file and decode image.
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filePath);
	if (error != 0)
	{
		lodepng_error_text(error);
		assert(error == 0);
		return -1;
	}

	GLuint temp;
	glGenTextures(1, &temp);

	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

	return temp;
}

GLuint Renderer::CreateBmpTexture(char * filePath)
{
	//Load Bmp: Load file and decode image.
	unsigned int width, height;
	unsigned char * bmp
		= loadBMPRaw(filePath, width, height);

	if (bmp == NULL)
	{
		std::cout << "Error while loading bmp file : " << filePath << std::endl;
		assert(bmp != NULL);
		return -1;
	}

	GLuint temp;
	glGenTextures(1, &temp);

	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp);

	return temp; 
}

void Renderer::Test()
{
	glUseProgram(m_SolidRectShader);

	int attribPosition = glGetAttribLocation(m_SolidRectShader, "a_Position");
	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE,0, 0);
	
	int attribPosition1 = glGetAttribLocation(m_SolidRectShader, "a_Position1");
	glEnableVertexAttribArray(attribPosition1);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO1);
	glVertexAttribPointer(attribPosition1, 3, GL_FLOAT, GL_FALSE,0, 0);

	static float gscale = 0.0f;
	GLint scaleUniform = glGetUniformLocation(m_SolidRectShader, "u_Sclale");
	glUniform1f(scaleUniform, gscale);	


	GLint ColorUniform = glGetUniformLocation(m_SolidRectShader, "u_Color");
	glUniform4f(ColorUniform, 0, gscale, 1, 1);

	GLint PositionUniform = glGetUniformLocation(m_SolidRectShader, "u_Position");
	glUniform3f(PositionUniform, -1, -1, 0);


	glDrawArrays(GL_TRIANGLES, 0, 3);
	gscale += 0.01f; 
	if(gscale > 1.0f)
	{
		gscale = 0.0f;
	}
	/////////////primiitive
	glDisableVertexAttribArray(attribPosition); 
	//glDisableVertexAttribArray(attribPosition1);
}
float g_Time = 0;
void Renderer::Particle()
{
	GLuint shader = m_SolidRectShader;

	glUseProgram(shader);

	int VBOLocation = glGetAttribLocation(shader, "a_Position");
	glEnableVertexAttribArray(VBOLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyPrticle);
	glVertexAttribPointer(VBOLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float)* 15, 0);
	  
	int VLocation = glGetAttribLocation(shader, "a_Velocity");
	glEnableVertexAttribArray(VLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyPrticle);
	glVertexAttribPointer(VLocation, 3, GL_FLOAT, GL_FALSE, 
		sizeof(float) * 15, (GLvoid*)(sizeof(float)*3));

	int EmitLocation = glGetAttribLocation(shader, "a_EmitTime");
	glEnableVertexAttribArray(EmitLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyPrticle);
	glVertexAttribPointer(EmitLocation, 1, GL_FLOAT, GL_FALSE,
		sizeof(float) * 15, (GLvoid*)(sizeof(float) * 6));

	int LifeLocation = glGetAttribLocation(shader, "a_LifeTime");
	glEnableVertexAttribArray(LifeLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyPrticle);
	glVertexAttribPointer(LifeLocation, 1, GL_FLOAT, GL_FALSE,
		sizeof(float) * 15, (GLvoid*)(sizeof(float) * 7));

	int PLocation = glGetAttribLocation(shader, "a_P");
	glEnableVertexAttribArray(PLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyPrticle);
	glVertexAttribPointer(PLocation, 1, GL_FLOAT, GL_FALSE,
		sizeof(float) * 15, (GLvoid*)(sizeof(float) * 8));

	int ALocation = glGetAttribLocation(shader, "a_A");
	glEnableVertexAttribArray(ALocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyPrticle);
	glVertexAttribPointer(ALocation, 1, GL_FLOAT, GL_FALSE,
		sizeof(float) * 15, (GLvoid*)(sizeof(float) * 9));

	int ARLocation = glGetAttribLocation(shader, "a_RandValue");
	glEnableVertexAttribArray(ARLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyPrticle);
	glVertexAttribPointer(ARLocation, 1, GL_FLOAT, GL_FALSE,
		sizeof(float) * 15, (GLvoid*)(sizeof(float) * 10));	
	
	int VBOColor = glGetAttribLocation(shader, "a_Color");
	glEnableVertexAttribArray(VBOColor);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyPrticle);
	glVertexAttribPointer(VBOColor, 4, GL_FLOAT, GL_FALSE,
		sizeof(float) * 15, (GLvoid*)(sizeof(float) * 11));


	GLint UniformTime = glGetUniformLocation(shader, "u_Time");
	glUniform1f(UniformTime, g_Time);

	GLint UniformExForce = glGetUniformLocation(shader, "u_ExForce");
	glUniform3f(UniformExForce, 1,0,0);
	glDrawArrays(GL_TRIANGLES, 0, m_VBOManyPrticleCount);

	
	g_Time += 0.016;
}
