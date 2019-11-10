#if defined WIN32 || _WIN32
#include <Kinect.h>
#endif

#include "glm/gtc/type_ptr.hpp"
#include <GL/glew.h>
#include "GL/freeglut.h"
#include "glm/glm.hpp"
#include <glm/gtx/transform.hpp>

#include "InitShader.h"
#include "LoadTexture.h"
#include "ScreenGrabber.h"
#include "file_io.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>

#include "imgui_impl_glut.h"
#include <cstdlib>

#include <conio.h>
#include <Windows.h>

#include "../RtMidi/RtMidi.h"
#include <deque>
#include "KinectJointFilter.h"
using namespace Sample;

RtMidiIn *midiin;

bool hide_ui = false;

ScreenGrabber* pGrabber = 0;
bool recording = false;
bool pauseUpdate = false;

static const int kinectDepthHeight = 424;
static const int kinectDepthWidth = 512;

static const int kinectColorHeight = 1080;
static const int kinectColorWidth = 1920;

static IMultiSourceFrameReader * reader = NULL;
static ICoordinateMapper * mapper = NULL;
static IKinectSensor * sensor = NULL;

CameraSpacePoint * depth2xyz = NULL;
ColorSpacePoint * depth2rgb = NULL;
GLubyte * rgbImage = NULL;

static bool enabledPointCloud = true;
static float * positionarray = NULL;
static float * colorarray = NULL;

static BOOLEAN bodyTracked = false;
static Joint joints[JointType_Count];
std::deque <double> _weightedX;
std::deque <double> _weightedY;
std::vector<std::deque <double>> JointsQueueX;
std::vector<std::deque <double>> JointsQueueY;
bool w_exp = false;
bool HD_exp = false;
bool button1 = false;


template <typename T>
inline void allocateData(T ** data, int size) { *data = new T[size]; }

template <typename T>
inline void destroyData(T ** data) { if (*data != NULL) delete[] data; }

int frame_num = 0;
int nFrames = 80; //how many frames of skeleton data to save
bool wireFrame = false;

std::vector<glm::vec3> skelBuffer;
std::vector<glm::vec3> skelNormal;

std::vector<unsigned int> skelIndices;
std::vector<unsigned int> curveIndices;
std::vector<unsigned int> patchIndices;

GLuint SkelVertsVBO = -1;
GLuint SkelNormalsVBO = -1;
GLuint SkelIndex = -1;
GLuint CurveIndex = -1;
GLuint PatchIndex = -1;

GLuint BackgroundTex = -1;
GLuint SurfaceTex = -1;

struct DanceVisSettings
{
	float camera_dist;

	int transformation;

	bool blend_enabled;
	int blend_equation;
	int blend_func;

	bool polygon_mode;
	float line_width;

	int shader_mode;
	float shader_params[10];

	
	int background;
	int background_mode;
	float background_params[10];

	glm::vec4 color[4];

	//settings below here aren't saved in the file
	int displacement_mode;
	float displacement_amount;
	

	DanceVisSettings::DanceVisSettings()
	{
		for(int i=0; i<4; i++)
		{
			color[i] = glm::vec4(1.0f);
		}
	};
};

int current_preset = 0;
DanceVisSettings presets[10];

DanceVisSettings& settings()
{
	return presets[current_preset];
}

std::string settings_filename("settings.ini");

struct QuadData
{
	GLuint quad_vao;
	GLuint quad_vbo;
};
QuadData quad;

glm::mat4 P;
glm::mat4 V;
float cam_height = 0.0f;

bool newSkeletonAvailable = false;
const int nBones = 20;
const int skel[2 * nBones] = { JointType_Head, JointType_Neck, JointType_Neck, JointType_SpineShoulder, JointType_SpineShoulder, JointType_SpineMid, JointType_SpineMid, JointType_SpineBase,
JointType_SpineShoulder, JointType_ShoulderLeft, JointType_ShoulderLeft, JointType_ElbowLeft, JointType_ElbowLeft, JointType_WristLeft, JointType_WristLeft, JointType_HandLeft,
JointType_SpineBase, JointType_HipLeft, JointType_HipLeft, JointType_KneeLeft, JointType_KneeLeft, JointType_AnkleLeft, JointType_AnkleLeft, JointType_FootLeft,
JointType_SpineShoulder, JointType_ShoulderRight, JointType_ShoulderRight, JointType_ElbowRight, JointType_ElbowRight, JointType_WristRight, JointType_WristRight, JointType_HandRight,
JointType_SpineBase, JointType_HipRight, JointType_HipRight, JointType_KneeRight, JointType_KneeRight, JointType_AnkleRight, JointType_AnkleRight, JointType_FootRight };



float map(float x, float min_range1, float max_range1, float min_range2, float max_range2) {
	// range 1 is the input range that needs to me mapped to range 2

	float range1 = abs(max_range1 - min_range1);
	float range2 = abs(max_range2 - min_range2);

	return (x / range1) * range2;

}

void mycallback(double deltatime, std::vector< unsigned char > *message, void *userData) // this function is used for MIDI, reads incoming data from MIDI
{
	unsigned int nBytes = message->size();

	for (unsigned int i = 0; i < nBytes; i++) {
		int midiInput = (int)message->at(i);
		if (midiInput == 0) {
			float midiValue = map((float)message->at(i + 1), 0.0, 127.0, 0.0, 20.0);
			settings().line_width = midiValue;
			break;
		}
		else if (midiInput == 1) {
			float midiValue = map((float)message->at(i + 1), 0.0, 127.0, 0.0, 10.0);
			settings().shader_mode = midiValue;
			break;
		}
		else if (midiInput == 2) {
			float midiValue = map((float)message->at(i + 1), 0.0, 127.0, 0.0, 1.0);
			settings().displacement_amount = midiValue;
			break;
		}
		else if (midiInput == 16) {
			float midiValue = map((float)message->at(i + 1), 0.0, 127.0, -2.0, 2.0);
			settings().camera_dist = midiValue;
			break;
		}
		else if (midiInput == 17) {
			float midiValue = map((float)message->at(i + 1), 0.0, 127.0, -2.0, 2.0);
			cam_height = midiValue;
			break;
		}
		else if (midiInput == 18) {
			float midiValue = map((float)message->at(i + 1), 0.0, 127.0, 0.0, 1.0);
			settings().color[0].r = midiValue;
			break;
		}
		else if (midiInput == 19) {
			float midiValue = map((float)message->at(i + 1), 0.0, 127.0, 0.0, 1.0);
			settings().color[0].g = midiValue;
			break;
		}
		else if (midiInput == 20) {
			float midiValue = map((float)message->at(i + 1), 0.0, 127.0, 0.0, 1.0);
			settings().color[0].b = midiValue;
			break;
		}
		else if (midiInput == 21) {
			float midiValue = map((float)message->at(i + 1), 0.0, 127.0, 0.0, 1.0);
			settings().color[0].a = midiValue;
			break;
		}
		else if (midiInput == 32) {
			settings().polygon_mode = !settings().polygon_mode;
			break;
		}

		//std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
	}
	//if (nBytes > 0)
		//std::cout << "stamp = " << deltatime << std::endl;
}

std::istream& operator>> (std::istream &in, DanceVisSettings &s)
{
   std::string header; //should be "settings"
   std::string spc(" ");
   in >> header;

	in >>  s.camera_dist;

	in >>  s.transformation;

	in >>  s.blend_enabled;
	in >>  s.blend_equation;
	in >>  s.blend_func;

	in >>  s.polygon_mode;
	in >>  s.line_width;

	in >>  s.shader_mode;
	for(int i=0; i<10; i++)
	{
	in >>  s.shader_params[i];
	}

	in >>  s.background;
	in >>  s.background_mode;

	for(int i=0; i<10; i++)
	{
	in >>  s.background_params[i];
	}

	for(int i=0; i<4; i++)
	{
		in >>  s.color[i].r >> s.color[i].g >> s.color[i].b >> s.color[i].a;
	}

   return in;
}

std::ostream &operator<< (std::ostream &out, const DanceVisSettings &s)
{
   std::string spc(" ");
   out << "settings ";

   	out <<  s.camera_dist << spc;

	out <<  s.transformation << spc;

	out <<  s.blend_enabled << spc;
	out <<  s.blend_equation << spc;
	out <<  s.blend_func << spc;

	out <<  s.polygon_mode << spc;
	out <<  s.line_width << spc;

	out <<  s.shader_mode<<spc;
	for(int i=0; i<10; i++)
	{
	out <<  s.shader_params[i] << spc;
	}

	out <<  s.background << spc;
	out <<  s.background_mode << spc;

	for(int i=0; i<10; i++)
	{
	out <<  s.background_params[i] << spc;
	}

	for(int i=0; i<4; i++)
	{
		out <<  s.color[i].r << spc << s.color[i].g << spc << s.color[i].b << spc << s.color[i].a << spc;
	}
	return out;
}


void settings_load(const std::string& path)
{
	std::fstream fs;
	std::string spc(" ");
	fs.open(path, std::fstream::in);

	if (fs.is_open())
	{
		for(int i=0; i<10; i++)
		{
			fs >> presets[i];
		}
	}
	fs.close();
}

void settings_save(const std::string& path)
{
   std::fstream fs;
   std::string spc(" ");
   fs.open(path, std::fstream::out);

	for(int i=0; i<10; i++)
	{
		fs << presets[i];
	}

   fs.close();
}


std::vector<std::vector<std::string>> background_filenames;
std::vector<std::vector<int>> background_textures;

static const std::string skel_vertex_shader("skel_vs.glsl");
static const std::string skel_fragment_shader("skel_fs.glsl");
GLuint skel_shader_program = -1;

static const std::string patch_vertex_shader("patch_vs.glsl");
static const std::string patch_tc_shader("patch_tc.glsl");
static const std::string patch_te_shader("patch_te.glsl");
static const std::string patch_gs_shader("patch_gs.glsl");
static const std::string patch_fragment_shader("patch_fs.glsl");
GLuint patch_shader_program = -1;

static const std::string bkg_vertex_shader("bkg_vs.glsl");
static const std::string bkg_fragment_shader("bkg_fs.glsl");
GLuint bkg_shader_program = -1;



void CreateQuad()
{
   glGenVertexArrays(1, &quad.quad_vao);
   glBindVertexArray(quad.quad_vao);

   float vertices[] = { 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f };

   //create vertex buffers for vertex coords
   glGenBuffers(1, &quad.quad_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, quad.quad_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   int pos_loc = 0;
   glEnableVertexAttribArray(pos_loc);
   glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, 0);

   glBindVertexArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void reload_shader(GLuint& program, const std::string& vs, const std::string& tc, const std::string& te, const std::string& gs, const std::string& fs)
{
   GLuint new_shader = -1;
   if (tc.length() == 0 && te.length() == 0 && gs.length() == 0)
   {
      new_shader = InitShader(vs.c_str(), fs.c_str());
   }
   else if (tc.length() == 0 && te.length() == 0)
   {
      new_shader = InitShader(vs.c_str(), gs.c_str(), fs.c_str());
   }
   else if (gs.length() == 0)
   {
	   new_shader = InitShader(vs.c_str(), tc.c_str(), te.c_str(), fs.c_str());
   }
   else
   {
	   new_shader = InitShader(vs.c_str(), tc.c_str(), te.c_str(), gs.c_str(), fs.c_str());
   }

   if(new_shader == -1) // loading failed
   {
      glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
   }
   else
   {
      glClearColor(0.35f, 0.35f, 0.35f, 0.0f);

      if(program != -1)
      {
         glDeleteProgram(program);
      }
      program = new_shader;

   }
}

void reload_shaders()
{
   reload_shader(bkg_shader_program, bkg_vertex_shader, "", "", "", bkg_fragment_shader);
   reload_shader(skel_shader_program, skel_vertex_shader, "", "", "", skel_fragment_shader);
   reload_shader(patch_shader_program, patch_vertex_shader, patch_tc_shader, patch_te_shader, patch_gs_shader, patch_fragment_shader);
}

void glError()
{
   GLenum errCode;
   const GLubyte *errString;
   if((errCode = glGetError()) != GL_NO_ERROR)
   {
      errString = gluErrorString(errCode);
      std::cout << "OpenGL Error: " << errString << std::endl;
   }
}

void initializeKinect()
{
	GetDefaultKinectSensor(&sensor);
	if (sensor)
	{
		sensor->get_CoordinateMapper(&mapper);

		sensor->Open();
		sensor->OpenMultiSourceFrameReader(
			FrameSourceTypes::FrameSourceTypes_Depth | 
			FrameSourceTypes::FrameSourceTypes_Color |
			FrameSourceTypes::FrameSourceTypes_Body, 
			&reader);
	}
}

void getDepthData(IMultiSourceFrame * frame, GLubyte * data)
{
	IDepthFrameReference * depthFrameReference = NULL;
	IDepthFrame * depthFrame = NULL;

	frame->get_DepthFrameReference(&depthFrameReference);
	depthFrameReference->AcquireFrame(&depthFrame);
	if (depthFrameReference) depthFrameReference->Release();

	if (!depthFrame) return;

	unsigned short * buffer = NULL;
	unsigned int sizebuffer = 0;
	depthFrame->AccessUnderlyingBuffer(&sizebuffer, &buffer);

	mapper->MapDepthFrameToCameraSpace((kinectDepthWidth * kinectDepthHeight), buffer, (kinectDepthWidth * kinectDepthHeight), depth2xyz);
	float * floatingDestination = (float *)data;
	for (unsigned int i = 0; i < sizebuffer; ++i)
	{
		(*floatingDestination++) = depth2xyz[i].X;
		(*floatingDestination++) = depth2xyz[i].Y;
		(*floatingDestination++) = depth2xyz[i].Z;
	}

	mapper->MapDepthFrameToColorSpace((kinectDepthWidth * kinectDepthHeight), buffer, (kinectDepthWidth * kinectDepthHeight), depth2rgb);
	if (depthFrame) depthFrame->Release();
}

void getColorData(IMultiSourceFrame * frame, GLubyte * data)
{
	IColorFrameReference * colorFrameReference = NULL;
	IColorFrame * colorFrame = NULL;

	frame->get_ColorFrameReference(&colorFrameReference);
	colorFrameReference->AcquireFrame(&colorFrame);
	if(colorFrameReference) colorFrameReference->Release();

	if (!colorFrame) return;

	colorFrame->CopyConvertedFrameDataToArray(kinectColorWidth * kinectColorHeight * 4, rgbImage, ColorImageFormat_Rgba);

	float * floatingDestination = (float *)data;
	for (int i = 0; i < (kinectDepthWidth * kinectDepthHeight); ++i)
	{
		ColorSpacePoint p = depth2rgb[i];

		if (p.X < 0 || p.Y < 0 || p.X > kinectColorWidth || p.Y > kinectColorHeight)
		{
			(*floatingDestination++) = 0.0f;
			(*floatingDestination++) = 0.0f;
			(*floatingDestination++) = 0.0f;
		}
		else
		{
			int linearIndex = ((int)p.X + ((int)p.Y * kinectColorWidth));
			(*floatingDestination++) = rgbImage[(4 * linearIndex) + 0] / 255.0f;
			(*floatingDestination++) = rgbImage[(4 * linearIndex) + 1] / 255.0f;
			(*floatingDestination++) = rgbImage[(4 * linearIndex) + 2] / 255.0f;
		}
	}

	if (colorFrame) colorFrame->Release();
}

void getBodyData(IMultiSourceFrame * frame)
{
	IBodyFrameReference * bodyFrameReference = NULL;
	IBodyFrame * bodyFrame = NULL;

	frame->get_BodyFrameReference(&bodyFrameReference);
	bodyFrameReference->AcquireFrame(&bodyFrame);
	if (bodyFrameReference) bodyFrameReference->Release();

	if (!bodyFrame) return;

	IBody * body[BODY_COUNT] = { 0 };
	bodyFrame->GetAndRefreshBodyData(BODY_COUNT, body);
	FilterDoubleExponential FDE;
	for (int i = 0; i < BODY_COUNT; ++i)
	{
		body[i]->get_IsTracked(&bodyTracked);
		if (bodyTracked)
		{
			body[i]->GetJoints(JointType_Count, joints);
			if (HD_exp) 
			{
				FDE.Update(body[i], joints);
			}
				
			break;
		}
	}


	if (bodyFrame) bodyFrame->Release();
}

void getKinectData()
{
	IMultiSourceFrame * frame = NULL;
	if (SUCCEEDED(reader->AcquireLatestFrame(&frame))) 
	{
		getDepthData(frame, (GLubyte *)positionarray);
		getColorData(frame, (GLubyte *)colorarray);
		getBodyData(frame);

		newSkeletonAvailable = true;
	}
	else
	{
		newSkeletonAvailable = false;
	}
	if (frame) frame->Release();
}

void resize(int w, int h)
{
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
		45.0,
		w / (GLdouble)h,
		0.1,
		1000.0
		);

	P = glm::perspective(45.0, w / (GLdouble)h, 0.01, 1000.0);
}



void idle()
{
	static glm::vec3 at(0.0f);
	static float frame = 0.0f;
	frame += 1.0f/30.0f;


	//glm::vec3 offset(0.1f*sin(0.001f*frame), 0.0f, +0.01f*cos(0.0013f*frame));
	//V = glm::lookAt(glm::vec3(0.0, 0.0, -0.15)+offset, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

	at = glm::mix(at, skelBuffer[JointType_SpineMid], 0.5f);
	V = glm::lookAt(glm::vec3(0.0, cam_height, -0.15-settings().camera_dist), at, glm::vec3(0.0, 1.0, 0.0));

	getKinectData();
	glutPostRedisplay();
}

inline static void glFixedPipelineLine(const glm::vec3 & pointA, const glm::vec3 & pointB)
{
	glVertex3fv(glm::value_ptr(pointA)); glVertex3fv(glm::value_ptr(pointB));
}

inline static glm::vec3 KinectCameraSpacePositionToglm(const CameraSpacePoint & point)
{
	return glm::vec3(point.X, point.Y, point.Z);
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void DrawIndexedSkeletons()
{
	glBindBuffer(GL_ARRAY_BUFFER, SkelVertsVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SkelIndex);

	const int pos_loc = 0;
	glEnableVertexAttribArray(pos_loc);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));
	glDrawElements(GL_LINES, skelIndices.size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));

	glDisableVertexAttribArray(pos_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DrawIndexedCurves()
{
	glBindBuffer(GL_ARRAY_BUFFER, SkelVertsVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CurveIndex);

	const int pos_loc = 0;
	glEnableVertexAttribArray(pos_loc);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));
	glDrawElements(GL_LINES, curveIndices.size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));

	glDisableVertexAttribArray(pos_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void DrawPatches()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PatchIndex);

	const int pos_loc = 0;
	const int normal_loc = 1;

	glBindBuffer(GL_ARRAY_BUFFER, SkelVertsVBO);
	glEnableVertexAttribArray(pos_loc);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, SkelNormalsVBO);
	glEnableVertexAttribArray(normal_loc);
	glVertexAttribPointer(normal_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));

	glPatchParameteri(GL_PATCH_VERTICES, 8);
	glDrawElements(GL_PATCHES, patchIndices.size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));

	glDisableVertexAttribArray(pos_loc);
	glDisableVertexAttribArray(normal_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void updateNormals()
{
	for(auto& n: skelNormal)
	{
		n = glm::vec3(0.0f);
	}

	for(int f=0; f<nFrames; f++)
	{
		for(int b=0; b<nBones; b++)
		{
			int i0 = skel[2*b] + JointType_Count*f;
			int i1 = skel[2*b+1] + JointType_Count*f;

			glm::vec3 p0 = skelBuffer[i0];
			glm::vec3 p1 = skelBuffer[i1];
			glm::vec3 u = p1-p0;

			if(f<nFrames-1)
			{
				//next quad
				int j0 = i0 + JointType_Count;
				int j1 = i1 + JointType_Count;

				glm::vec3 q0 = skelBuffer[j0];
				glm::vec3 q1 = skelBuffer[j1];

				glm::vec3 v = q0-p0;
				glm::vec3 w = q1-p1;

				skelNormal[i0] += glm::cross(u,v);
				skelNormal[i1] += glm::cross(w,-u);
			}
			if(f>0)
			{
				//previous quad
				int j0 = i0 - JointType_Count;
				int j1 = i1 - JointType_Count;

				glm::vec3 q0 = skelBuffer[j0];
				glm::vec3 q1 = skelBuffer[j1];

				glm::vec3 v = q0-p0;
				glm::vec3 w = q1-p1;

				skelNormal[i0] += glm::cross(v,u);
				skelNormal[i1] += glm::cross(w,u);
			}
		}
	}
}

void draw_gui()
{
   ImGui_ImplGlut_NewFrame();

   if(ImGui::Checkbox("pause update", &pauseUpdate))
   {
      
   }
   ImGui::SameLine();
   if(ImGui::Button("Reload Shaders"))
   {
		reload_shaders();  
   }

   if(ImGui::SliderFloat("camera dis", &settings().camera_dist, -2.0f, 2.0f))
   {
      
   }
   
   if(ImGui::SliderFloat("camera height", &cam_height, -2.0f, 2.0f))
   {
      
   }

   if(ImGui::SliderInt("transformation", &settings().transformation, 0, 3))
   {
      
   }
   if(ImGui::Checkbox("blend enabled", &settings().blend_enabled))
   {
      
   }
   ImGui::SameLine();
   if(ImGui::SliderInt("blend equation", &settings().blend_equation, 0, 2))
   {
      
   }
   if(ImGui::SliderInt("Blend Func", &settings().blend_func, 0, 1))
   {
      
   }


   if(ImGui::Checkbox("Wireframe", &settings().polygon_mode))
   {
      
   }

   
   ImGui::SameLine();
   if(ImGui::SliderFloat("Line Width", &settings().line_width, 0.0f, 20.0f))
   {
      
   }
   
   
   if(ImGui::SliderInt("shader mode", &settings().shader_mode, 0, 10))
   {
      
   }

   const int n_sliders = 10;
   for (int i = 0; i<n_sliders; i++)
   {
      std::string name = std::string("Slider ") + std::to_string(i);
      ImGui::SliderFloat(name.c_str(), &settings().shader_params[i], 0.0f, +1.0f);
   }

   
   if(ImGui::SliderInt("background", &settings().background, 0, 1))
   {
      
   }

   if(ImGui::SliderInt("background mode", &settings().background_mode, 0, 10))
   {
      
   }

   const int n_bkg_sliders = 10;
   for (int i = 0; i<n_sliders; i++)
   {
      std::string name = std::string("Bkg Slider ") + std::to_string(i);
      ImGui::SliderFloat(name.c_str(), &settings().background_params[i], 0.0f, +1.0f);
   }

  
   for(int i=0; i<4; i++)
   {
	   std::string name = std::string("Color ") + std::to_string(i);
	   if(ImGui::ColorEdit4(name.c_str(), &settings().color[i].r, true))
	   {

	   }
   }

   ImGui::Text("Preset");
   ImGui::SameLine();
   for(int i=0; i<10; i++)
   {
	   std::string label = std::to_string(i);
	   bool highlight_button = current_preset==i;

	   if(highlight_button)
	   {
			ImGui::PushStyleColor(ImGuiCol_Button, ImColor(0.5f, 0.6f, 0.6f));
	   }

	   if(ImGui::Button(label.c_str()))
	   {
		    current_preset = i;
	   }

	   if(highlight_button)
	   {
			ImGui::PopStyleColor();
	   }
	   ImGui::SameLine();
   }

   if(ImGui::Button("Save Presets"))
	{
		settings_save(settings_filename);
	}

   if(ImGui::SliderInt("displacement mode", &settings().displacement_mode, 0, 10))
   {
      
   }

   
   if(ImGui::SliderFloat("displacement amount", &settings().displacement_amount, 0.0f, 1.0f))
   {
      
   }
 
   ImGui::Checkbox("Weighted Exponential Smoothing", &w_exp);  
   // http://blogs.interknowlogy.com/2011/10/27/kinect-joint-smoothing/  

   ImGui::Checkbox("Holt Double Exponential Smoothing", &HD_exp);

   ImGui::Render();
}

glm::vec4 blendTracking(glm::vec4 a, glm::vec3 b, glm::vec3 dist) {
	// x is my dist AND y is my thres
	// eponential threshold value for blending function = y = 1.0/(100*x+1.0);	

	// clamp(y, 0.001, 0.999);
	//std::cout << "dist x: " << dist.x << "  dist y: " << dist.y << "  dist z: " << dist.z << std::endl;

	glm::vec4 p = a;
	glm::vec3 d = dist;

	glm::mat4 scaleX = glm::scale(glm::vec3((100.0)));
	d = glm::vec3(scaleX * glm::vec4(dist, 1.0));

	// x blend value
	
	if (dist.x >= 0.1) {
		//float xb = 1.0 / (d.x + 1.0);
		//p.x = (xb*a.x) + ((1 - xb)*b.x);
		std::cout << "X DIST IS LARGE" << std::endl;
		p.x = (0.8*a.x) + (0.2*b.x);
	}

	if (dist.y >= 0.1) {
		// y blend value
		//float yb = 1.0 / (d.y + 1.0);
		//p.y = (yb*a.y) + ((1 - yb)*b.y);
		std::cout << "Y DIST IS LARGE" << std::endl;
		p.y = (0.8*a.y) + (0.2*b.y);
	}

	if (dist.z >= 0.05) {
		// z blend value
		//float zb = 1.0 / (d.z + 1.0);
		//p.z = (zb*a.z) + ((1 - zb)*b.z);
		std::cout << "Z DIST IS LARGE" << std::endl;
		p.z = (0.8*a.z) + (0.2*b.z);
	}

	return p;
}

double ExponentialMovingAverage(std::deque<double> data, int dataSize, double baseValue)
{
	double numerator = 0;
	double denominator = 0;

	double average = 0.0;
	for (int i = 0; i < dataSize; i++) {
		average += data[i];
	}
	average /= (double)dataSize;

	for (int i = 0; i < dataSize; ++i)
	{
		numerator += data[i] * pow(baseValue, dataSize - i - 1);
		denominator += pow(baseValue, dataSize - i - 1);
	}

	numerator += average * pow(baseValue, dataSize);
	denominator += pow(baseValue, dataSize);

	return numerator / denominator;
}

glm::vec4 ExponentialWeightedAvg(Joint joint, int index)
{
	JointsQueueX[index].push_back(joint.Position.X);
	JointsQueueY[index].push_back(joint.Position.Y);

	if (JointsQueueX[index].size() > 7)  // Settings.Default.Smoothing = 5/7
	{
		JointsQueueX[index].pop_front();
		JointsQueueY[index].pop_front();
	}

	double x = ExponentialMovingAverage(JointsQueueX[index], JointsQueueX[index].size(), 0.9);
	double y = ExponentialMovingAverage(JointsQueueY[index], JointsQueueY[index].size(), 0.9);

	return glm::vec4(x,y, joint.Position.Z, 1.0f);
}

void display()
{

	static float frame = 0.0f;
	static int frame_id = 0;
	frame += 1.0f/30.0f;
	float scale = 1.0f;
	frame_id++;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	glUseProgram(bkg_shader_program);
	glUniform1f(0, frame);
	
	if(settings().background >= 0)
	{
		int n=background_textures[settings().background].size();
		
		for(int i=0; i<n; i++)
		{
			glActiveTexture(GL_TEXTURE0+i);
			glBindTexture(GL_TEXTURE_2D, background_textures[settings().background][i]);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			glUniform1i(i+1, i);
		}
	}

	glUniform1i(5, settings().background_mode);
	glUniform1fv(6, 10, settings().background_params);
	
	glBindVertexArray(quad.quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);

	if (enabledPointCloud)
	{
		glPointSize(5.0f);
		glBegin(GL_POINTS);
		for (int i = 0; i < (kinectDepthWidth * kinectDepthHeight); ++i)
		{
			glColor3f(colorarray[(i * 3) + 0], colorarray[(i * 3) + 1], colorarray[(i * 3) + 2]);
			glVertex3f(positionarray[(i * 3) + 0], positionarray[(i * 3) + 1], positionarray[(i * 3) + 2]);
		}
		glEnd();
		glPointSize(1.0f);
	}

	//if (bodyTracked)
	{
		if(newSkeletonAvailable == true && pauseUpdate == false)
		{
			//copy next frame into skelBuffer
			std::rotate(skelBuffer.begin(), skelBuffer.begin()+skelBuffer.size()-JointType_Count, skelBuffer.end()); //// OVER HERE //////

			glm::mat4 M = glm::scale(glm::vec3(1.0f));

			if(settings().transformation==1 || settings().transformation==3)
			{
				float s = 1.1f;
				//float s = 2.1f;
				glm::mat4 T0 = glm::translate(-KinectCameraSpacePositionToglm(joints[JointType_SpineMid].Position));
				glm::mat4 T1 = glm::translate(KinectCameraSpacePositionToglm(joints[JointType_SpineMid].Position));
				M = T1*glm::rotate(s*frame, glm::vec3(0.0f, 1.0f, 0.0f))*T0;
			}
			if(settings().transformation==2 || settings().transformation==3)
			{
				float s = 0.1f;
				//float s = 1.1f;
				glm::mat4 R2 = glm::rotate(s*frame, glm::vec3(0.0f, 1.0f, 0.0f));
				M = R2*M;
			}

			//////////////// KINECT JOINT SMOOTHING ////////////////////
			for(int i=0; i<JointType_Count; i++)
			{
				glm::vec4 p = glm::vec4(0);
				if (w_exp)
				{
					http://blogs.interknowlogy.com/2011/10/27/kinect-joint-smoothing/
					p = ExponentialWeightedAvg(joints[i], i);
				}
				else if (HD_exp) 
				{
					p = glm::vec4(KinectCameraSpacePositionToglm(joints[i].Position), 1.0f);
				}
				else {
					p = glm::vec4(KinectCameraSpacePositionToglm(joints[i].Position), 1.0f);				
				}
								
				glm::vec4 p2 = M*p;

				/*
				glm::vec3 prev = skelBuffer[i + JointType_Count + 1];
				glm::vec3 dist = abs(prev - glm::vec3(p2));
				p2 = blendTracking(p2, prev, dist);
				*/

				skelBuffer[i] = glm::vec3(p2);
			}
			//std::cout << "KINECT END" << std::endl;
			//updateNormals(); //not needed - computed in shader

			glBindBuffer(GL_ARRAY_BUFFER, SkelVertsVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, skelBuffer.size()*3*sizeof(float), skelBuffer.data());

			//glBindBuffer(GL_ARRAY_BUFFER, SkelNormalsVBO);
			//glBufferSubData(GL_ARRAY_BUFFER, 0, skelNormal.size()*3*sizeof(float), skelNormal.data());

			frame_num++;
			glError();
		}

		
		const bool drawSkeletons = false;
		const bool drawCurves = false;

		if(drawSkeletons == true || drawCurves == true)
		{
			glLineWidth(5.0f);
			glUseProgram(skel_shader_program);
			glm::mat4 PVM = P*V;
			glUniformMatrix4fv(0, 1, false, glm::value_ptr(PVM));
			glUniform4fv(1, 1, glm::value_ptr(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)));
			if(drawSkeletons == true)
			{
				DrawIndexedSkeletons();
			}

			glUniform4fv(1, 1, glm::value_ptr(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)));
			if(drawCurves == true)
			{
				DrawIndexedCurves();
			}
		}
		
		if(settings().polygon_mode==0)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else if(settings().polygon_mode==1)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
			
		if(settings().blend_enabled==0)
		{
			glDisable(GL_BLEND);
		}
		else
		{
			glEnable(GL_BLEND);
		}

		if(settings().blend_equation==0)
		{
			glBlendEquation(GL_FUNC_ADD);
		}
		else if(settings().blend_equation==1)
		{
			glBlendEquation(GL_FUNC_SUBTRACT);
		}
		else if(settings().blend_equation==2)
		{
			glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		}

		if(settings().blend_func==0)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//semitransparent
		}
		else if(settings().blend_func==1)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE); //additive
		}
		
		glLineWidth(settings().line_width+0.0001f);
		glUseProgram(patch_shader_program);

		glUniform1f(1, frame);
		glUniform1f(2, float(frame_id%nFrames));
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SurfaceTex);
		glUniform1i(3, 1);//texture is unit 1

		glUniform1i(4, settings().shader_mode);
		glUniform1fv(5, 10, settings().shader_params);
		glm::mat4 PVM = P*V;
		glUniformMatrix4fv(0, 1, false, glm::value_ptr(PVM));
		glUniform4fv(15, 4, glm::value_ptr(settings().color[0]));

		glUniform1i(19, settings().displacement_mode);
		glUniform1f(20, settings().displacement_amount);

		DrawPatches();

		glError();

		glUseProgram(0);

		
	}

	glBlendEquation(GL_FUNC_ADD);
	if(hide_ui == false)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		draw_gui();
	}

	glutSwapBuffers();

	if(recording)
	{
		pGrabber->Grab();
	}

}

void special(int key, int x, int y)
{
	ImGui_ImplGlut_SpecialCallback(key);
	if(key == GLUT_KEY_F1)
	{
		hide_ui = !hide_ui;
	}

	switch (key)
	{
	case GLUT_KEY_F11:
		recording = !recording;
		break;

	default:
		break;
	}

}

void keyboard(unsigned char key, int x, int y)
{
	ImGui_ImplGlut_KeyCallback(key);

	switch (key)
	{
	case 27:
	case 'q':
	case 'Q':
		glutLeaveMainLoop();
		break;

	case 13:
	case 'f':
	case 'F':
		glutFullScreenToggle();
		break;

	case 'r':
		reload_shaders();
		break;

	default:
		break;
	}

	if(key >= '0' && key <= '9')
	{
		current_preset = key-'0';
	}

}

void keyboard_up(unsigned char key, int x, int y)
{
   ImGui_ImplGlut_KeyUpCallback(key);
}

void special_up(int key, int x, int y)
{
   ImGui_ImplGlut_SpecialUpCallback(key);
}

void passive(int x, int y)
{
   ImGui_ImplGlut_PassiveMouseMotionCallback(x,y);
}

void motion(int x, int y)
{
   ImGui_ImplGlut_MouseMotionCallback(x, y);
}

void mouse(int button, int state, int x, int y)
{
   ImGui_ImplGlut_MouseButtonCallback(button, state);
}

int main(int argc, char ** argv)
{


	settings_load(settings_filename);

	initializeKinect();

	allocateData(&rgbImage, (kinectColorWidth * kinectColorHeight * 4));
	allocateData(&depth2rgb, (kinectDepthWidth * kinectDepthHeight));
	allocateData(&depth2xyz, (kinectDepthWidth * kinectDepthHeight));

	allocateData(&positionarray, (kinectDepthWidth * kinectDepthHeight * 3));
	allocateData(&colorarray, (kinectDepthWidth * kinectDepthHeight * 3));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(kinectDepthWidth, kinectDepthHeight);
	glutCreateWindow("Bodygraphe");

	pGrabber = new ScreenGrabber();

	glewInit();
	ImGui_ImplGlut_Init();

	file_load("backgrounds.ini");

	JointsQueueX.resize(JointType_Count);
	JointsQueueY.resize(JointType_Count);

	skelBuffer.resize(nFrames*JointType_Count);
	skelNormal.resize(skelBuffer.size());

	skelIndices.resize(nFrames*40);

	int max_units;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_units);

	glGenBuffers(1, &SkelVertsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, SkelVertsVBO);
	glBufferData(GL_ARRAY_BUFFER, skelBuffer.size()*3*sizeof(float), skelBuffer.data(), GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	int p = 0;
	for(int f=0; f<nFrames; f++)
	{
		for(int i=0; i<40; i++)
		{
			skelIndices[p++] = skel[i] + JointType_Count*f;
		}
	}

	glGenBuffers(1, &SkelNormalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, SkelNormalsVBO);
	glBufferData(GL_ARRAY_BUFFER, skelNormal.size()*3*sizeof(float), skelNormal.data(), GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glGenBuffers(1, &SkelIndex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SkelIndex);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, skelIndices.size()*sizeof(unsigned int), skelIndices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	curveIndices.resize(2*(nFrames-1)*JointType_Count);
	p = 0;
	for(int i=0; i<JointType_Count; i++)
	{
		for(int f=0; f<nFrames-1; f++)
		{
			curveIndices[p++] = i + JointType_Count*f;
			curveIndices[p++] = i + JointType_Count*(f+1);
		}
	}

	glGenBuffers(1, &CurveIndex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CurveIndex);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, curveIndices.size()*sizeof(unsigned int), curveIndices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	
	for (int i = 0; i < 2 * nBones; i += 2)
	{
		for (int f = 0; f < nFrames - 3; ++f)
		{
			for (int q = 0; q < 4; q++)
			{
				patchIndices.push_back(skel[i + 0] + (f+q)*JointType_Count);
				patchIndices.push_back(skel[i + 1] + (f+q)*JointType_Count);
			}
		}
	}
	

    glGenBuffers(1, &PatchIndex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PatchIndex);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, patchIndices.size()*sizeof(unsigned int), patchIndices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glError();

	glActiveTexture(GL_TEXTURE0);
	BackgroundTex = LoadTexture("background.png");
	glActiveTexture(GL_TEXTURE1);
	SurfaceTex = LoadTexture("grad.png");
	CreateQuad();


	reload_shaders();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		0.0, 0.0, 0.0,
		0.0, 0.0, 5.0,
		0.0, 1.0, 0.0
		);

	V = glm::lookAt(glm::vec3(0.0, -0.0, -0.2), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));


	/////////////////////// MIDI ///////////////////////////

	midiin = new RtMidiIn();
	std::vector<unsigned char> message;
	int nBytes, i;
	double stamp;
	// Check available ports.
	unsigned int nPorts = midiin->getPortCount();
	if (nPorts == 0) {
		//std::cout << "No ports available!\n";
		//goto cleanup;
	}
	midiin->openPort(0);
	midiin->setCallback(&mycallback);
	// Don't ignore sysex, timing, or active sensing messages.
	midiin->ignoreTypes(false, false, false);
	
	// Periodically check input queue.
	std::cout << "Reading MIDI from port ... quit with Ctrl-C.\n";

	/////////////////////// MIDI ///////////////////////////
	float j = 0.001;
	for (int i = 0; i < 100; i++) {
		float y = 1.0 / (100.0*j + 1);		
		std::cout << "j: " << j << ",  y: " << y << std::endl;
		j += 0.001;
	}

	glutKeyboardFunc(keyboard);

	glutSpecialFunc(special);
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutKeyboardUpFunc(keyboard_up);
	glutSpecialUpFunc(special_up);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);

	glutMainLoop();

	destroyData(&positionarray);
	destroyData(&colorarray);

	destroyData(&depth2xyz);
	destroyData(&depth2rgb);
	destroyData(&rgbImage);
	return 0;
}