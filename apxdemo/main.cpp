// Copyright (c) 2012 Oren Fromberg. All rights reserved.
// This work is licensed under the Creative Commons Attribution 3.0 Unported 
// License. To view a copy of this license, visit 
// http://creativecommons.org/licenses/by/3.0/ or send a letter to Creative 
// Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.

// All code is written by me, Oren Fromberg, unless otherwise noted.

// The author of the original sky texture titled "miramar" is Jockum Skoglund 
// aka hipshot and it is published under the following licence: "Modify however
// you like, just cred me for my work, maybe link to my page".
// The texture is available in jpg here: http://www.zfight.com/misc/images/textures/envmaps/miramar_large.jpg
// And the original as six tga here:
// http://www.zfight.com/misc/files/textures/envmap_miramar.rar
// hipshot published his textures here:
// - http://forums.epicgames.com/threads/506748-My-skies-and-and-cliff-textures-(large-images!).
// - http://www.quake3world.com/forum/viewtopic.php?t=9242

// includes
#include <SDL.h>
#include <SDL_image.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <GL/glew.h>

// local headers
#include "glm-0.9.2.6/glm/glm.hpp"
#include "glm-0.9.2.6/glm/gtc/matrix_transform.hpp"
#include "glm-0.9.2.6/glm/gtc/type_ptr.hpp"

// defines
#define WIDTH 800
#define HEIGHT 600
#define KEYDOWN_ARRAY_SIZE 256

// global array used to keep track of what keys are pressed on the keyboard
bool g_keysdown_arr[KEYDOWN_ARRAY_SIZE];

// global vector for text typed on keyboard
std::vector<char> g_text;

// cubemap texture
GLuint cubemap_texture;

// the following function was adapted from Keith Lantz
// http://www.keithlantz.net/2011/10/rendering-a-skybox-using-a-cube-map-with-opengl-and-glsl/
char* loadFile(const char *filename) {
	char* data;
	int len;
	std::ifstream ifs(filename, std::ifstream::in);
	ifs.seekg(0, std::ios::end);
	len = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	data = new char[len + 1];
	memset(data, 0, len);
	ifs.read(data, len);
	data[len] = 0;
	ifs.close();
	return data;
}

// the following function was adapted from Keith Lantz
// http://www.keithlantz.net/2011/10/rendering-a-skybox-using-a-cube-map-with-opengl-and-glsl/
void setupCubeMap(GLuint& texture) {
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

// the following function was adapted from Keith Lantz
// http://www.keithlantz.net/2011/10/rendering-a-skybox-using-a-cube-map-with-opengl-and-glsl/
void setupCubeMap(GLuint& texture, SDL_Surface *xpos, SDL_Surface *xneg, SDL_Surface *ypos, SDL_Surface *yneg, SDL_Surface *zpos, SDL_Surface *zneg) {
	setupCubeMap(texture);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, xpos->w, xpos->h, 0, xpos->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, xpos->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, xneg->w, xneg->h, 0, xneg->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, xneg->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, ypos->w, ypos->h, 0, ypos->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, ypos->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, yneg->w, yneg->h, 0, yneg->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, yneg->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, zpos->w, zpos->h, 0, zpos->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, zpos->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, zneg->w, zneg->h, 0, zneg->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, zneg->pixels);
}

// this function will draw a letter at the coordinate (x, y)
void renderLetter (float x, float y, char letter)
{
	int row = letter/16;
	int col = letter - row*16;
	glBegin (GL_QUADS);
	glTexCoord2f ((col * 32.f)/512.0, (row * 32.f)/512.0);
	glVertex2f(x, y);
	glTexCoord2f ((col * 32.f)/512.0, ((row+1) * 32.f)/512.0);
	glVertex2f(x, y+32);
	glTexCoord2f (((col+1) * 32.f)/512.0, ((row+1) * 32.f)/512.0);
	glVertex2f(x+32, y+32);
	glTexCoord2f (((col+1) * 32.f)/512.0, (row * 32.f)/512.0);
	glVertex2f(x+32, y);
	glEnd ();
}

// this function will render all the text that has been typed on the keyboard to the top corner of the screen
void renderText()
{
	for (int i = 0; i < g_text.size(); ++i)
	{
		float origin_x = i*16.0;
		float origin_y = 32.0;
		renderLetter(origin_x, origin_y, g_text[i]);
	}
}

// x, y, z are translation from origin of key, and s and t are indices for character in texture. (origin top left of image)
void drawKey(float x, float y, float z, int s, int t, char c)
{
	// this is for when we render to the selection buffer
	glLoadName((GLuint)c);

	// push modelview matrix
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();

	// translate to key location 
	glTranslatef (x, y, z);

	if (g_keysdown_arr[(unsigned char )c])
	{
		// depress button
		glTranslatef (0.0, 0.0, -2.0);
	}

	glBegin(GL_QUADS);

	// top
	glTexCoord2f (0.0, 0.0);
	glVertex3f ( 1.0, 1.0,-1.0);
	glVertex3f (-1.0, 1.0,-1.0);
	glVertex3f (-1.0, 1.0, 1.0);
	glVertex3f ( 1.0, 1.0, 1.0);

	// bottom
	glTexCoord2f (0.0, 0.0);
	glVertex3f ( 1.0,-1.0, 1.0);
	glVertex3f (-1.0,-1.0, 1.0);
	glVertex3f (-1.0,-1.0,-1.0);
	glVertex3f ( 1.0,-1.0,-1.0);

	// front
	glTexCoord2f (0.05*(s+1), 0.1*t);      glVertex3f( 1.0f, 1.0f, 1.0f);
	glTexCoord2f (0.05*s,  0.1*t);      glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f (0.05*s, .10*(t+1));      glVertex3f(-1.0f,-1.0f, 1.0f);
	glTexCoord2f (0.05*(s+1), .10*(t+1));      glVertex3f( 1.0f,-1.0f, 1.0f);

	// back
	glTexCoord2f (0.0, 0.0);
	glVertex3f ( 1.0,-1.0,-1.0);
	glVertex3f (-1.0,-1.0,-1.0);
	glVertex3f (-1.0, 1.0,-1.0);
	glVertex3f ( 1.0, 1.0,-1.0);

	// left
	glTexCoord2f (0.0, 0.0);
	glVertex3f (-1.0, 1.0, 1.0);
	glVertex3f (-1.0, 1.0,-1.0);
	glVertex3f (-1.0,-1.0,-1.0);
	glVertex3f (-1.0,-1.0, 1.0);

	// right
	glTexCoord2f (0.0, 0.0);
	glVertex3f ( 1.0, 1.0,-1.0);
	glVertex3f ( 1.0, 1.0, 1.0);
	glVertex3f ( 1.0,-1.0, 1.0);
	glVertex3f ( 1.0,-1.0,-1.0);

	glEnd ();

	// pop modelview 
	glPopMatrix ();
}

void drawKeyboard()
{
	// draw the keyboard here
	drawKey(-18, 6.0, 0.0, 0, 0, '`'); //`
	drawKey(-15, 6.0, 0.0, 1, 0, '1'); //1
	drawKey(-12, 6.0, 0.0, 2, 0, '2'); //2
	drawKey(-9, 6.0, 0.0, 3, 0, '3'); //3
	drawKey(-6, 6.0, 0.0, 4, 0, '4'); //4
	drawKey(-3, 6.0, 0.0, 5, 0, '5'); //5
	drawKey(0, 6.0, 0.0, 6, 0, '6'); //6
	drawKey(3, 6.0, 0.0, 7, 0, '7'); //7
	drawKey(6, 6.0, 0.0, 8, 0, '8'); //8
	drawKey(9, 6.0, 0.0, 9, 0, '9'); //9
	drawKey(12, 6.0, 0.0, 10, 0, '0'); //0
	drawKey(15, 6.0, 0.0, 11, 0, '-'); //-
	drawKey(18, 6.0, 0.0, 12, 0, '='); //=

	drawKey(-15.5, 3.0, 0.0, 0, 1, 'q'); //q
	drawKey(-12.5, 3.0, 0.0, 1, 1, 'w'); //w
	drawKey(-9.5, 3.0, 0.0, 2, 1, 'e'); //e
	drawKey(-6.5, 3.0, 0.0, 3, 1, 'r'); //r
	drawKey(-3.5, 3.0, 0.0, 4, 1, 't'); //t
	drawKey(-.5, 3.0, 0.0, 5, 1, 'y'); //y
	drawKey(2.5, 3.0, 0.0, 6, 1, 'u'); //u
	drawKey(5.5, 3.0, 0.0, 7, 1, 'i'); //i
	drawKey(8.5, 3.0, 0.0, 8, 1, 'o'); //o
	drawKey(11.5, 3.0, 0.0, 9, 1, 'p'); //p
	drawKey(14.5, 3.0, 0.0, 10, 1, '['); //[
	drawKey(17.5, 3.0, 0.0, 11, 1, ']'); //]

	drawKey(-15.0, 0.0, 0.0, 0, 2, 'a'); //a
	drawKey(-12.0, 0.0, 0.0, 1, 2, 's'); //s
	drawKey(-9.0, 0.0, 0.0, 2, 2, 'd'); //d
	drawKey(-6.0, 0.0, 0.0, 3, 2, 'f'); //f
	drawKey(-3.0, 0.0, 0.0, 4, 2, 'g'); //g
	drawKey(0.0, 0.0, 0.0, 5, 2, 'h'); //h
	drawKey(3.0, 0.0, 0.0, 6, 2, 'j'); //j
	drawKey(6.0, 0.0, 0.0, 7, 2, 'k'); //k
	drawKey(9.0, 0.0, 0.0, 8, 2, 'l'); //l
	drawKey(12.0, 0.0, 0.0, 9, 2, ';'); //;
	drawKey(15.0, 0.0, 0.0, 10, 2, '\''); //'

	drawKey(-13.0, -3.0, 0.0, 0, 3, 'z'); //z
	drawKey(-10.0, -3.0, 0.0, 1, 3, 'x'); //x
	drawKey(-7.0, -3.0, 0.0, 2, 3, 'c'); //c
	drawKey(-4.0, -3.0, 0.0, 3, 3, 'v'); //v
	drawKey(-1.0, -3.0, 0.0, 4, 3, 'b'); //b
	drawKey(2.0, -3.0, 0.0, 5, 3, 'n'); //n
	drawKey(5.0, -3.0, 0.0, 6, 3, 'm'); //m
	drawKey(8.0, -3.0, 0.0, 7, 3, ','); //,
	drawKey(11.0, -3.0, 0.0, 8, 3, '.'); //.
	drawKey(14.0, -3.0, 0.0, 9, 3, '/'); ///
}

// this code will render to the selection buffer and then process hits to figure out which key was clicked
// some of this code is from http://content.gpwiki.org/index.php/OpenGL:Tutorials:Picking
// and http://www.lighthouse3d.com/opengl/picking/index.php3?openglway3
void doSelection (int x, int y)
{
	GLuint buf[64] = {0};
	GLint hits, view[4];
	int id;
	glSelectBuffer(64, buf);
	glGetIntegerv(GL_VIEWPORT, view);
	glRenderMode (GL_SELECT);
	glInitNames();
	glPushName(0);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix (x, y, 1.0, 1.0, view);
	gluPerspective (45, (float)view[2]/(float)view[3], 1.0, 1000.0);

	drawKeyboard();

	hits = glRenderMode (GL_RENDER);

	// if no hits then we are done here
	if (hits == 0)
		return;

	// find hit with max MinZ
	int max = 0, closest = 0;
	GLuint * ptr = NULL;
	for (int i = 0; i < hits; ++i)
	{
		if ((GLubyte)buf[i * 4 + 1] > max)
		{
			max = (GLubyte)buf[i * 4 + 1];
			closest = i;
		}
	}
	unsigned char c = buf[closest*4 + 3];

	// flip bit
	g_keysdown_arr[c] = !g_keysdown_arr[c];

	// add letter to text vector
	g_text.push_back((char)c);
}

void Init()
{
	// init global array
	memset(g_keysdown_arr, 0, sizeof(char)*KEYDOWN_ARRAY_SIZE);

	// setup an opengl context
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_Surface *screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_HWSURFACE | SDL_OPENGL);
	SDL_WM_SetCaption("Virtual Keyboard Demo for APX-Labs by Oren Fromberg", NULL);

	// initialize the extension wrangler
	glewInit();

	// set our viewport, clear color and depth, and enable depth testing
	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

void SetupCubeMap()
{
	// set up the cube map texture
	SDL_Surface *xpos = IMG_Load("media/xpos.png");
	SDL_Surface *xneg = IMG_Load("media/xneg.png");
	SDL_Surface *ypos = IMG_Load("media/ypos.png");
	SDL_Surface *yneg = IMG_Load("media/yneg.png");
	SDL_Surface *zpos = IMG_Load("media/zpos.png");
	SDL_Surface *zneg = IMG_Load("media/zneg.png");
	setupCubeMap(cubemap_texture, xpos, xneg, ypos, yneg, zpos, zneg);
	SDL_FreeSurface(xneg);	SDL_FreeSurface(xpos);
	SDL_FreeSurface(yneg);	SDL_FreeSurface(ypos);
	SDL_FreeSurface(zneg);	SDL_FreeSurface(zpos);
}

void CheckShaderCompilation(GLuint shader)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

		fprintf(stderr, "Compilation error in shader %s: %s\n", "glShaderV", strInfoLog);
		delete[] strInfoLog;
	}
}

// the main function
int main (int argc, char **argv) 
{
	Init();

	SetupCubeMap();

	// load our shaders and compile them.. create a program and link it
	GLuint glShaderV = glCreateShader(GL_VERTEX_SHADER);
	GLuint glShaderF = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* vShaderSource = loadFile("skybox.vert.hlsl");
	const GLchar* fShaderSource = loadFile("skybox.frag.hlsl");
	glShaderSource(glShaderV, 1, &vShaderSource, NULL);
	glShaderSource(glShaderF, 1, &fShaderSource, NULL);
	glCompileShader(glShaderV);
	CheckShaderCompilation(glShaderV);
	glCompileShader(glShaderF);
	CheckShaderCompilation(glShaderF);
	delete[] vShaderSource;
	delete[] fShaderSource;

	GLuint cubeMapProg = glCreateProgram();
	glAttachShader(cubeMapProg, glShaderV);
	glAttachShader(cubeMapProg, glShaderF);
	glLinkProgram(cubeMapProg);
	glUseProgram(cubeMapProg);
	int  vlength,    flength;
	char vlog[2048], flog[2048];
	glGetShaderInfoLog(glShaderV, 2048, &vlength, vlog);
	glGetShaderInfoLog(glShaderF, 2048, &flength, flog);
	std::cout << vlog << std::endl << std::endl << flog << std::endl << std::endl;
	
	// load shaders for the keyboard geometry
	glShaderV = glCreateShader (GL_VERTEX_SHADER);
	glShaderF = glCreateShader (GL_FRAGMENT_SHADER);
	const GLchar * mVSource = loadFile ("model.vert.hlsl");
	const GLchar * mFSource = loadFile ("model.frag.hlsl");
	glShaderSource(glShaderV, 1, &mVSource, NULL);
	glShaderSource(glShaderF, 1, &mFSource, NULL);
	delete [] mVSource;
	delete [] mFSource;
	glCompileShader(glShaderV);
	CheckShaderCompilation(glShaderV);
	glCompileShader(glShaderF);
	CheckShaderCompilation(glShaderF);
	GLuint modelProg = glCreateProgram();
	glAttachShader(modelProg, glShaderV);
	glAttachShader(modelProg, glShaderF);
	glLinkProgram(modelProg);
	glUseProgram(modelProg);
	glGetShaderInfoLog(glShaderV, 2048, &vlength, vlog);
	glGetShaderInfoLog(glShaderF, 2048, &flength, flog);
	std::cout << vlog << std::endl << std::endl << flog << std::endl << std::endl;

	// load texture with letters on keyboard
	SDL_Surface *keySurface = IMG_Load ("keys.png"); // I made this image with GIMP
	GLuint keys_texture;
	glActiveTexture (GL_TEXTURE0);
	glEnable (GL_TEXTURE_2D);
	glGenTextures (1, &keys_texture);
	glBindTexture (GL_TEXTURE_2D, keys_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, keySurface->w, keySurface->h, 0, keySurface->format->BytesPerPixel == 4? GL_RGBA: GL_RGB, GL_UNSIGNED_BYTE, keySurface->pixels);
	glBindTexture (GL_TEXTURE_2D, 0);
	SDL_FreeSurface (keySurface);
	GLint uniform_keys = glGetUniformLocation (modelProg, "keys");

	// load premade bitmap font
	// http://www.amanithvg.com/testsuite/amanithvg_sre/data/font_bitmap.png
	SDL_Surface *bmf = IMG_Load ("font_bitmap.png");
	GLuint bmf_texture;
	glActiveTexture(GL_TEXTURE0);
	glEnable (GL_TEXTURE_2D);
	glGenTextures (1, &bmf_texture);
	glBindTexture (GL_TEXTURE_2D, bmf_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, bmf->w, bmf->h, 0, bmf->format->BytesPerPixel == 4? GL_RGBA: GL_RGB, GL_UNSIGNED_BYTE, bmf->pixels);
	glBindTexture (GL_TEXTURE_2D, 0);
	SDL_FreeSurface (bmf);

	// grab the pvm matrix and vertex location from our shader program
	GLint PVM    = glGetUniformLocation(cubeMapProg, "PVM");
	GLint vertex = glGetAttribLocation(cubeMapProg, "vertex");

	// these won't change for now
	glm::mat4 Projection = glm::perspective(45.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f); 
	glm::mat4 View       = glm::mat4(1.0f);
	glm::mat4 Model      = glm::scale(glm::mat4(1.0f),glm::vec3(50,50,50));

	// cube vertices for vertex buffer object
	GLfloat cube_vertices[] = {
		-1.0,  1.0,  1.0,
		-1.0, -1.0,  1.0,
		1.0, -1.0,  1.0,
		1.0,  1.0,  1.0,
		-1.0,  1.0, -1.0,
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0,  1.0, -1.0,
	};

	// vertex array buffer
	GLuint vbo_cube_vertices;
	glGenBuffers(1, &vbo_cube_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertex);
	glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// cube indices for index buffer object
	GLushort cube_indices[] = {
		0, 1, 2, 3,
		3, 2, 6, 7,
		7, 6, 5, 4,
		4, 5, 1, 0,
		0, 3, 7, 4,
		1, 2, 6, 5,
	};

	// index array buffer
	GLuint ibo_cube_indices;
	glGenBuffers(1, &ibo_cube_indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

	// vars
	float alpha = 0.f, beta = 0.f, gamma = 50.f;
	int locX = 0, locY = 0, pressedX = 0, pressedY = 0;
	float origX = 0.f, origY = 0.f;
	bool done = false;
	bool mouse_down = false;
	char * ch = NULL;
	unsigned int num_ticks = 0;

	// for handling events
	SDL_Event event;

	// the game loop
	while(!done)
	{		
		// get the current time... might be useful.
		num_ticks = SDL_GetTicks();

		// process inputs
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				done = true;
				break;
			case SDL_MOUSEMOTION:
				locX = event.motion.x;
				locY = event.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouse_down = true;
				pressedX = event.button.x;
				pressedY = event.button.y;
				switch (event.button.button)
				{
				case SDL_BUTTON_LEFT:
					doSelection(event.button.x, HEIGHT - event.button.y);
					break;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (mouse_down)
				{
					mouse_down = false;
					origY = alpha;
					origX = beta;
				}
				switch (event.button.button)
				{
				case SDL_BUTTON_WHEELUP:
					gamma += 10.0;
					break;
				case SDL_BUTTON_WHEELDOWN:
					gamma -= 10.0;
					if (gamma < 10.0) gamma = 10.0;
					break;
				case SDL_BUTTON_LEFT:
					//just set all keys back to up
					for (int i = 0; i < KEYDOWN_ARRAY_SIZE; ++i) g_keysdown_arr[i] = false;
					break;
				}
				break;
			case SDL_KEYDOWN:
				ch = SDL_GetKeyName(event.key.keysym.sym);
				g_keysdown_arr[(unsigned char)*ch] = true;
				g_text.push_back(*ch);
				break;
			case SDL_KEYUP:
				ch = SDL_GetKeyName(event.key.keysym.sym);
				g_keysdown_arr[(unsigned char)*ch] = false;
				break;
			default:
				break;
			}
		}

		// do updates
		if (mouse_down)
		{
			alpha = origY + locY - pressedY;
			beta = origX + locX - pressedX;
		}

		// this should add a subtle sensation of movement to the scene
		//alpha += .005*sin(0.0005*(double)num_ticks);
		//beta += .005*cos(0.0005*(double)num_ticks);

		// this marks the start of the code that draws
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// set the program for the cubemap
		glUseProgram(cubeMapProg);

		// uniforms for sky box include the sampler and the MVP matrix
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);

		// set up the ModelViewProj matrix for the cube map
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), alpha, glm::vec3(-1.0f, 0.0f, 0.0f));
		glm::mat4 RotateY = glm::rotate(RotateX, beta, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 M = Projection * View * Model * RotateY;
		glUniformMatrix4fv(PVM, 1, GL_FALSE, glm::value_ptr(M));

		// bind vertex array and index array
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
		glEnableVertexAttribArray(vertex);
		glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_indices);

		// draw cubemap
		glDrawElements(GL_QUADS, sizeof(cube_indices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

		// clean up OpenGL state for cubemap
		glDisable (GL_TEXTURE_CUBE_MAP);
		glDisableVertexAttribArray (vertex);

		// clear depth buffer because sky box is infinitely far away
		glClear (GL_DEPTH_BUFFER_BIT);

		// use program for rendering keyboard
		glUseProgram(modelProg);

		// enable blending for translucency
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// enable back face culling to make translucent keys look better
		glEnable (GL_CULL_FACE);
		glCullFace (GL_BACK);

		// set up projection matrix for keyboard
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0, (float)WIDTH/(float)HEIGHT, 0.1, 1000.0);

		// set up modelview matrix for keyboard
		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt (0.f, 0.f, gamma, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
		glRotatef (-alpha, 1, 0, 0);
		glRotatef (beta, 0, 1, 0);

		// set the texture for the keys
		glActiveTexture (GL_TEXTURE0);
		glEnable (GL_TEXTURE_2D);
		glBindTexture (GL_TEXTURE_2D, keys_texture);
		glUniform1i (uniform_keys, 0);

		// draw the keyboard
		drawKeyboard();

		// clean up OpenGL state for keyboard
		glDisable (GL_BLEND);
		glDisable (GL_CULL_FACE);

		// now we print the text onto the screen
		// clear depth again
		glClear (GL_DEPTH_BUFFER_BIT);

		// render text to screen using orthographic projection
		glMatrixMode (GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D (0, WIDTH, 0, HEIGHT);
		// scale and translate to get origin in upper left hand corner of screen
		glScalef(1, -1, 1);
		glTranslatef(0, -HEIGHT, 0);
		glMatrixMode (GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glUseProgram(0); // using fixed function for the text, shaders not necessary

		// draw a translucent text box at the top of the screen
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBegin (GL_QUADS);
		glColor4f (1.0, 1.0, 1.0, 0.5);
		glVertex2f (0.0, 0.0);
		glVertex2f (0.0, 100.0);
		glVertex2f (WIDTH, 100.0);
		glVertex2f (WIDTH, 0.0);
		glEnd ();
		glDisable (GL_BLEND);

		// draw the text
		glActiveTexture (GL_TEXTURE0);
		glEnable (GL_TEXTURE_2D);
		glBindTexture (GL_TEXTURE_2D, bmf_texture);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		renderText();
		glMatrixMode (GL_PROJECTION);
		glPopMatrix ();
		glMatrixMode (GL_MODELVIEW);
		glPopMatrix();
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_BLEND);

		// finally, swap buffers
		SDL_GL_SwapBuffers();	    
	}

	return 0;
}

