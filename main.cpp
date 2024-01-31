#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <GL/glew.h>   // The GL Header File
#include <GL/gl.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint gProgram[5];
int gWidth = 640, gHeight = 480;
bool hit = false;
bool rightPressed = false;
bool leftPressed = false;
int counter = 20;
int highCounter = counter;
int lowCounter = 0;
double eps = 0.025;
int score = 0;
double horizontal = 0.0;
double zbox = -30.0;
bool zbox_tour = false;
double z_small = 0.2;
double small = 0.05;
bool restart = false;
int n;
char h = 'n';
double zground = 0.0;
bool happy = false;
float angle = 0.0;
float angle_small = 5;
double base = -1.6;
bool counterControl = false;

struct Vertex
{
    Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Texture
{
    Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
    GLfloat u, v;
};

struct Normal
{
    Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
    GLuint vIndex[3], tIndex[3], nIndex[3];
};

vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

vector<Vertex> gVertices2;
vector<Texture> gTextures2;
vector<Normal> gNormals2;
vector<Face> gFaces2;

vector<Vertex> gVertices3;
vector<Texture> gTextures3;
vector<Normal> gNormals3;
vector<Face> gFaces3;

GLuint gVertexAttribBuffer, gTextVBO, gIndexBuffer;
GLuint gVertexAttribBuffer2, gIndexBuffer2;
GLuint gVertexAttribBuffer3, gIndexBuffer3;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;
int gVertexDataSizeInBytes2, gNormalDataSizeInBytes2;
int gVertexDataSizeInBytes3, gNormalDataSizeInBytes3;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;


bool ParseObj(const string& fileName)
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == '#') // comment
                {
                    continue;
                }
                else if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        gTextures.push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        gNormals.push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        gVertices.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
					char c;
					int vIndex[3],  nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0]; 
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1]; 
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2]; 

					assert(vIndex[0] == nIndex[0] &&
						   vIndex[1] == nIndex[1] &&
						   vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

                    gFaces.push_back(Face(vIndex, tIndex, nIndex));
                }
                else
                {
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }

            //data += curLine;
            if (!myfile.eof())
            {
                //data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

	/*
	for (int i = 0; i < gVertices.size(); ++i)
	{
		Vector3 n;

		for (int j = 0; j < gFaces.size(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (gFaces[j].vIndex[k] == i)
				{
					// face j contains vertex i
					Vector3 a(gVertices[gFaces[j].vIndex[0]].x, 
							  gVertices[gFaces[j].vIndex[0]].y,
							  gVertices[gFaces[j].vIndex[0]].z);

					Vector3 b(gVertices[gFaces[j].vIndex[1]].x, 
							  gVertices[gFaces[j].vIndex[1]].y,
							  gVertices[gFaces[j].vIndex[1]].z);

					Vector3 c(gVertices[gFaces[j].vIndex[2]].x, 
							  gVertices[gFaces[j].vIndex[2]].y,
							  gVertices[gFaces[j].vIndex[2]].z);

					Vector3 ab = b - a;
					Vector3 ac = c - a;
					Vector3 normalFromThisFace = (ab.cross(ac)).getNormalized();
					n += normalFromThisFace;
				}

			}
		}

		n.normalize();

		gNormals.push_back(Normal(n.x, n.y, n.z));
	}
	*/

	assert(gVertices.size() == gNormals.size());

    return true;
}

bool ParseObj2(const string& fileName)
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == '#') // comment
                {
                    continue;
                }
                else if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        gTextures2.push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        gNormals2.push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        gVertices2.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
					char c;
					int vIndex[3],  nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0]; 
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1]; 
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2]; 

					assert(vIndex[0] == nIndex[0] &&
						   vIndex[1] == nIndex[1] &&
						   vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

                    gFaces2.push_back(Face(vIndex, tIndex, nIndex));
                }
                else
                {
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }

            //data += curLine;
            if (!myfile.eof())
            {
                //data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

	assert(gVertices2.size() == gNormals2.size());

    return true;
}

bool ParseObj3(const string& fileName)
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == '#') // comment
                {
                    continue;
                }
                else if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        gTextures3.push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        gNormals3.push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        gVertices3.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
					char c;
					int vIndex[3],  nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0]; 
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1]; 
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2]; 

					assert(vIndex[0] == nIndex[0] &&
						   vIndex[1] == nIndex[1] &&
						   vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

                    gFaces3.push_back(Face(vIndex, tIndex, nIndex));
                }
                else
                {
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }

            //data += curLine;
            if (!myfile.eof())
            {
                //data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

	assert(gVertices3.size() == gNormals3.size());

    return true;
}

bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

void createVS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

    glAttachShader(program, vs);
}

void createFS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

    glAttachShader(program, fs);
}

void initShaders()
{
    gProgram[0] = glCreateProgram();
    gProgram[1] = glCreateProgram();
    gProgram[2] = glCreateProgram();
    gProgram[3] = glCreateProgram();
    gProgram[4] = glCreateProgram();

    createVS(gProgram[0], "vert3.glsl");
    createFS(gProgram[0], "frag3.glsl");

    createVS(gProgram[1], "vert4.glsl");
    createFS(gProgram[1], "frag4.glsl");

    createVS(gProgram[2], "vert.glsl");
    createFS(gProgram[2], "frag.glsl");   

    createVS(gProgram[3], "vert2.glsl");
    createFS(gProgram[3], "frag2.glsl");    

    createVS(gProgram[4], "vert_text.glsl");
    createFS(gProgram[4], "frag_text.glsl");  

    glBindAttribLocation(gProgram[0], 0, "inVertex");
    glBindAttribLocation(gProgram[0], 1, "inNormal");
    glBindAttribLocation(gProgram[1], 0, "inVertex");
    glBindAttribLocation(gProgram[1], 1, "inNormal");
    glBindAttribLocation(gProgram[2], 0, "inVertex");
    glBindAttribLocation(gProgram[2], 1, "inNormal");
    glBindAttribLocation(gProgram[3], 0, "inVertex");
    glBindAttribLocation(gProgram[3], 1, "inNormal");
    glBindAttribLocation(gProgram[4], 2, "vertex");

    glLinkProgram(gProgram[0]);
    glLinkProgram(gProgram[1]);
    glLinkProgram(gProgram[2]);
    glLinkProgram(gProgram[3]);
    glLinkProgram(gProgram[4]);
    glUseProgram(gProgram[2]);

}


void initVBO()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBuffer);
    glGenBuffers(1, &gIndexBuffer);

    assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    gVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
    GLfloat* vertexData = new GLfloat [gVertices.size() * 3];
    GLfloat* normalData = new GLfloat [gNormals.size() * 3];
    GLuint* indexData = new GLuint [gFaces.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < gVertices.size(); ++i)
    {
        vertexData[3*i] = gVertices[i].x;
        vertexData[3*i+1] = gVertices[i].y;
        vertexData[3*i+2] = gVertices[i].z;

        minX = std::min(minX, gVertices[i].x);
        maxX = std::max(maxX, gVertices[i].x);
        minY = std::min(minY, gVertices[i].y);
        maxY = std::max(maxY, gVertices[i].y);
        minZ = std::min(minZ, gVertices[i].z);
        maxZ = std::max(maxZ, gVertices[i].z);
    }

    for (int i = 0; i < gNormals.size(); ++i)
    {
        normalData[3*i] = gNormals[i].x;
        normalData[3*i+1] = gNormals[i].y;
        normalData[3*i+2] = gNormals[i].z;
    }

    for (int i = 0; i < gFaces.size(); ++i)
    {
        indexData[3*i] = gFaces[i].vIndex[0];
        indexData[3*i+1] = gFaces[i].vIndex[1];
        indexData[3*i+2] = gFaces[i].vIndex[2];
    }


    glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));


    // glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initVBO2()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBuffer2);
    glGenBuffers(1, &gIndexBuffer2);

    assert(gVertexAttribBuffer2 > 0 && gIndexBuffer2 > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer2);

    gVertexDataSizeInBytes2 = gVertices2.size() * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytes2 = gNormals2.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces2.size() * 3 * sizeof(GLuint);
    GLfloat* vertexData = new GLfloat [gVertices2.size() * 3];
    GLfloat* normalData = new GLfloat [gNormals2.size() * 3];
    GLuint* indexData = new GLuint [gFaces2.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < gVertices2.size(); ++i)
    {
        vertexData[3*i] = gVertices2[i].x;
        vertexData[3*i+1] = gVertices2[i].y;
        vertexData[3*i+2] = gVertices2[i].z;

        minX = std::min(minX, gVertices2[i].x);
        maxX = std::max(maxX, gVertices2[i].x);
        minY = std::min(minY, gVertices2[i].y);
        maxY = std::max(maxY, gVertices2[i].y);
        minZ = std::min(minZ, gVertices2[i].z);
        maxZ = std::max(maxZ, gVertices2[i].z);
    }

    for (int i = 0; i < gNormals2.size(); ++i)
    {
        normalData[3*i] = gNormals2[i].x;
        normalData[3*i+1] = gNormals2[i].y;
        normalData[3*i+2] = gNormals2[i].z;
    }

    for (int i = 0; i < gFaces2.size(); ++i)
    {
        indexData[3*i] = gFaces2[i].vIndex[0];
        indexData[3*i+1] = gFaces2[i].vIndex[1];
        indexData[3*i+2] = gFaces2[i].vIndex[2];
    }


    glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes2 + gNormalDataSizeInBytes2, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes2, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes2, gNormalDataSizeInBytes2, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes2));

}


void initVBO3()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBuffer3);
    glGenBuffers(1, &gIndexBuffer3);

    assert(gVertexAttribBuffer3 > 0 && gIndexBuffer3 > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer3);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer3);

    gVertexDataSizeInBytes3 = gVertices3.size() * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytes3 = gNormals3.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces3.size() * 3 * sizeof(GLuint);
    GLfloat* vertexData = new GLfloat [gVertices3.size() * 3];
    GLfloat* normalData = new GLfloat [gNormals3.size() * 3];
    GLuint* indexData = new GLuint [gFaces3.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < gVertices3.size(); ++i)
    {
        vertexData[3*i] = gVertices3[i].x;
        vertexData[3*i+1] = gVertices3[i].y;
        vertexData[3*i+2] = gVertices3[i].z;

        minX = std::min(minX, gVertices3[i].x);
        maxX = std::max(maxX, gVertices3[i].x);
        minY = std::min(minY, gVertices3[i].y);
        maxY = std::max(maxY, gVertices3[i].y);
        minZ = std::min(minZ, gVertices3[i].z);
        maxZ = std::max(maxZ, gVertices3[i].z);
    }

    for (int i = 0; i < gNormals3.size(); ++i)
    {
        normalData[3*i] = gNormals3[i].x;
        normalData[3*i+1] = gNormals3[i].y;
        normalData[3*i+2] = gNormals3[i].z;
    }

    for (int i = 0; i < gFaces3.size(); ++i)
    {
        indexData[3*i] = gFaces3[i].vIndex[0];
        indexData[3*i+1] = gFaces3[i].vIndex[1];
        indexData[3*i+2] = gFaces3[i].vIndex[2];
    }


    glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes3 + gNormalDataSizeInBytes3, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes3, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes3, gNormalDataSizeInBytes3, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes3));

}

void initFonts(int windowWidth, int windowHeight)
{
    // Set OpenGL options
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
    glUseProgram(gProgram[4]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[4], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/liberation/LiberationSerif-Italic.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //
    // Configure VBO for texture quads
    //
    glGenBuffers(1, &gTextVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void init() 
{
	ParseObj("bunny.obj");
    ParseObj2("cube.obj");
    ParseObj3("quad.obj");
    glEnable(GL_DEPTH_TEST);
    initShaders();
    initFonts(gWidth, gHeight);
    initVBO();
    initVBO2();
    initVBO3();
}

void drawModel()
{
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, gFaces.size() * 3, GL_UNSIGNED_INT, 0);
}

void drawModel2() {
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes2));

	glDrawElements(GL_TRIANGLES, gFaces2.size() * 3, GL_UNSIGNED_INT, 0);
}

void drawModel3() {
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer3);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes3));

	glDrawElements(GL_TRIANGLES, gFaces3.size() * 3, GL_UNSIGNED_INT, 0);   
}

void renderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
    glUseProgram(gProgram[4]);
    glUniform3f(glGetUniformLocation(gProgram[4], "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)

        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

char collision_detection(double &horizontal, double &zbox, int i) {
    char c = 'n';
    bool collision_z = zbox >= -3 && zbox <= -1.5;
    if(!i) {
        if((horizontal >= -0.6 && horizontal <= 0.6) && collision_z) c = '1';
        else if((horizontal >= 1.2 && horizontal <= 2.8) && collision_z) c = '2';
    }
    else if(i == 1) {
        if((horizontal >= -2.8 && horizontal <= -1.2) && collision_z) c = '0';
        else if((horizontal >= 1.2 && horizontal <= 2.8) && collision_z) c = '2';
    }
    else if(i == 2) {
        if((horizontal >= -2.8 && horizontal <= -1.2) && collision_z) c = '0';
        else if((horizontal >= -0.5 && horizontal <= 0.5) && collision_z) c = '1';
    }
    return c;
}

bool checkpoint_detection(double &horizontal, double &zbox, int i) {
    bool collision_x;
    bool collision_z = zbox >= -3 && zbox <= -1.5;
    if(!i) {
        collision_x = (horizontal >= -2.8 && horizontal <= -1.2);
    }
    else if(i == 1) {
        collision_x = (horizontal >= -0.6 && horizontal <= 0.6);
    }
    else if(i == 2) {
        collision_x = (horizontal >= 1.2 && horizontal <= 2.8);
    }
    return collision_x && collision_z;
}


void display()
{
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if(h == 'n') score++;

    if(restart) {
        score = 0;
        counter = 20;
        highCounter = counter;
        lowCounter = 0;
        horizontal = 0.0;       
        zbox = -30.0;
        zbox_tour = false; 
        hit = false;
        h = 'n';
        z_small = 0.2;
        small = 0.05;
        zground = 0.0;
        eps = 0.025;
        angle = 0.0;
        angle_small = 5;
        base = -1.6;
        counterControl = false;
        happy = false;
    }

    if(!zbox_tour) {
        n = rand() % 3;
        zbox_tour = true;
    }

    h = collision_detection(horizontal, zbox, n);

    if(h == 'n') {
        glUseProgram(gProgram[2]);

        if(checkpoint_detection(horizontal, zbox, n) && !hit) {
            score += 1000;
            hit = true;
            happy = true;
        } 

        glm::mat4 T;


        if (highCounter) {
            base += eps;
            if(leftPressed && horizontal - small >= -2) horizontal -= small;
            else if(rightPressed && horizontal + small <= 2) horizontal += small;
            T = glm::translate(glm::mat4(1.f), glm::vec3(horizontal, base, -1.8f));
            highCounter--;
            if(!highCounter) {
                lowCounter = counter; 
            }     
        }
        else {
            base -= eps;
            if(leftPressed && horizontal - small >= -2) horizontal -= small;
            else if(rightPressed && horizontal + small <= 2) horizontal += small;
            T = glm::translate(glm::mat4(1.f), glm::vec3(horizontal, base, -1.8f));
            lowCounter--;
            if(!lowCounter) {
                highCounter = counter;
                counterControl = true;
            }
        }

        glm::mat4 R = glm::rotate(glm::mat4(1.f), glm::radians(-85.f), glm::vec3(0, 1, 0));
        glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3(0.2f, 0.3f, 0.2f));
        glm::mat4 modelMat;
        glm::mat4 R3;
        if(!happy) {
            modelMat = T * R * S;
        }
        else {
            angle += angle_small;
            R3 = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0, 1, 0));
            if(angle >= 360.0) {
                angle = 0.0;
                happy = false;
            }
            modelMat = T * R3 * R * S;
        }
        glm::mat4 modelMatInv = glm::transpose(glm::inverse(modelMat));
        float aspect = gWidth/gHeight;
        glm::mat4 perspMat = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 200.0f);

        glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

        drawModel();

        if(!n) {
            if(!hit) {
                glUseProgram(gProgram[1]);

                T = glm::translate(glm::mat4(1.f), glm::vec3(-2.f, -1.5f, zbox));
                S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
                modelMat = T * S;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

                drawModel2();                
            }

            glUseProgram(gProgram[0]);

            T = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -1.5f, zbox));
            S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
            modelMat = T * S;
            modelMatInv = glm::transpose(glm::inverse(modelMat));

            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

            drawModel2();

            T = glm::translate(glm::mat4(1.f), glm::vec3(2.f, -1.5f, zbox));
            S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
            modelMat = T * S;
            modelMatInv = glm::transpose(glm::inverse(modelMat));

            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

            drawModel2();

        }
        else if(n == 1) {
            glUseProgram(gProgram[0]);

            T = glm::translate(glm::mat4(1.f), glm::vec3(-2.f, -1.5f, zbox));
            S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
            modelMat = T * S;
            modelMatInv = glm::transpose(glm::inverse(modelMat));

            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

            drawModel2();

            if(!hit) {
                glUseProgram(gProgram[1]);

                T = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -1.5f, zbox));
                S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
                modelMat = T * S;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

                drawModel2();               
            }

            glUseProgram(gProgram[0]);

            T = glm::translate(glm::mat4(1.f), glm::vec3(2.f, -1.5f, zbox));
            S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
            modelMat = T * S;
            modelMatInv = glm::transpose(glm::inverse(modelMat));

            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

            drawModel2();

        }
        else {
            glUseProgram(gProgram[0]);

            T = glm::translate(glm::mat4(1.f), glm::vec3(-2.f, -1.5f, zbox));
            S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
            modelMat = T * S;
            modelMatInv = glm::transpose(glm::inverse(modelMat));

            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

            drawModel2();

            T = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -1.5f, zbox));
            S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
            modelMat = T * S;
            modelMatInv = glm::transpose(glm::inverse(modelMat));

            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

            drawModel2();

            if(!hit) {
                glUseProgram(gProgram[1]);

                T = glm::translate(glm::mat4(1.f), glm::vec3(2.f, -1.5f, zbox));
                S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
                modelMat = T * S;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

                drawModel2();                
            }
        }

        glUseProgram(gProgram[3]);

        T = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -2.f, zground));
        S = glm::scale(glm::mat4(1.f), glm::vec3(3.5f, 1.0f, 1000.0f));
        R = glm::rotate(glm::mat4(1.0), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
        glm::mat4 View = glm::lookAt(glm::vec3(0,0,zground), glm::vec3(0,0,zground-1.f), glm::vec3(0,1.f,0));
        modelMat = T * S * R;
        modelMatInv = glm::transpose(glm::inverse(modelMat));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "viewMat"), 1, GL_FALSE, glm::value_ptr(View));

        drawModel3();    

        zbox += z_small;
        zground -= z_small;
        if(zbox >= 0.0) {
            zbox = -30.0;
            zbox_tour = false;
            hit = false;
            z_small += 0.02;
            small += 0.01;
            if(small >= 0.2) small = 0.2;
            angle_small += 1.5;
            if(angle_small >= 180) angle_small = 180;
            if(counterControl) {
                counterControl = false;
                if(counter != 5) {
                    counter--;
                    eps = 0.5 / counter;
                }
            }
        }
    }

    else {
        glUseProgram(gProgram[2]);
        double base = -1.3;
        glm::mat4 T = glm::translate(glm::mat4(1.f), glm::vec3(horizontal, base, -1.8f));
        glm::mat4 R2 = glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(0, 0, 1));
        glm::mat4 R = glm::rotate(glm::mat4(1.f), glm::radians(-85.f), glm::vec3(0, 1, 0));
        glm:: mat4 S = glm::scale(glm::mat4(1.f), glm::vec3(0.2f, 0.3f, 0.2f));
        glm::mat4 modelMat = T * R2 * R * S;
        glm::mat4 modelMatInv = glm::transpose(glm::inverse(modelMat));
        float aspect = gWidth/gHeight;
        glm::mat4 perspMat = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 200.0f);

        glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

        drawModel();

        if(!n) {
            glUseProgram(gProgram[1]);

            T = glm::translate(glm::mat4(1.f), glm::vec3(-2.f, -1.5f, zbox));
            S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
            modelMat = T * S;
            modelMatInv = glm::transpose(glm::inverse(modelMat));

            glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

            drawModel2();

            glUseProgram(gProgram[0]);

            if(h == '1') {
                T = glm::translate(glm::mat4(1.f), glm::vec3(2.f, -1.5f, zbox));
                S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
                modelMat = T * S;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

                drawModel2();
            }
            else if(h == '2') {
                T = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -1.5f, zbox));
                S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
                modelMat = T * S;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

                drawModel2();
            }
        }
        else if(n == 1) {
            glUseProgram(gProgram[0]);
            if(h == '0') {
                T = glm::translate(glm::mat4(1.f), glm::vec3(2.f, -1.5f, zbox));
                S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
                modelMat = T * S;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

                drawModel2();
            }
            else if(h == '2') {
                T = glm::translate(glm::mat4(1.f), glm::vec3(-2.f, -1.5f, zbox));
                S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
                modelMat = T * S;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

                drawModel2();
            }

            glUseProgram(gProgram[1]);

            T = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -1.5f, zbox));
            S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
            modelMat = T * S;
            modelMatInv = glm::transpose(glm::inverse(modelMat));

            glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

            drawModel2();
        }
        else {
            glUseProgram(gProgram[0]);
            if(h == '0') {
                T = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -1.5f, zbox));
                S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
                modelMat = T * S;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

                drawModel2();
            }
            else if(h == '1') {
                T = glm::translate(glm::mat4(1.f), glm::vec3(-2.f, -1.5f, zbox));
                S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
                modelMat = T * S;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

                drawModel2();
            }

            glUseProgram(gProgram[1]);

            T = glm::translate(glm::mat4(1.f), glm::vec3(2.f, -1.5f, zbox));
            S = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 1.f, 0.4f));
            modelMat = T * S;
            modelMatInv = glm::transpose(glm::inverse(modelMat));

            glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
            glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

            drawModel2();
        }

        glUseProgram(gProgram[3]);

        T = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -2.f, zground));
        S = glm::scale(glm::mat4(1.f), glm::vec3(3.5f, 1.0f, 100.0f));
        R = glm::rotate(glm::mat4(1.0), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
        glm::mat4 View = glm::lookAt(glm::vec3(0,0,zground), glm::vec3(0,0,zground-1.f), glm::vec3(0,1.f,0));
        modelMat = T * S * R;
        modelMatInv = glm::transpose(glm::inverse(modelMat));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));
        glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "viewMat"), 1, GL_FALSE, glm::value_ptr(View));

        drawModel3(); 

    }

    assert(glGetError() == GL_NO_ERROR);

    if(h == 'n') renderText("Score: " + to_string(score), 0, 445, 0.8, glm::vec3(1, 1, 0));
    else renderText("Score: "  + to_string(score), 0, 445, 0.8, glm::vec3(1, 0, 0));

    assert(glGetError() == GL_NO_ERROR);

}

void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if(key == GLFW_KEY_A && action == GLFW_PRESS) {
        leftPressed = true;
    }
    else if(key == GLFW_KEY_A && action == GLFW_RELEASE) {
        leftPressed = false;
    }
    else if(key == GLFW_KEY_D && action == GLFW_PRESS) {
        rightPressed = true;
    }
    else if(key == GLFW_KEY_D && action == GLFW_RELEASE) {
        rightPressed = false;
    }
    else if(key == GLFW_KEY_R && action == GLFW_PRESS) {
        restart = true;
    }
    else if(key == GLFW_KEY_R && action == GLFW_RELEASE) {
        restart = false;
    }
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(gWidth, gHeight, "HW3", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();


    glfwSetKeyCallback(window, keyboard);


    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, gWidth, gHeight); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

