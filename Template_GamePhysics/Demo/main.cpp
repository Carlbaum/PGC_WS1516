//--------------------------------------------------------------------------------------
// File: main.cpp
//
// The main file containing the entry point main().
//--------------------------------------------------------------------------------------

#include <sstream>
#include <iomanip>
#include <random>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

//DirectX includes
#include <DirectXMath.h>
//using namespace DirectX;
using std::cout;

// Effect framework includes
#include <d3dx11effect.h>

// DXUT includes
#include <DXUT.h>
#include <DXUTcamera.h>

// DirectXTK includes
#include "Effects.h"
#include "VertexTypes.h"
#include "PrimitiveBatch.h"
#include "GeometricPrimitive.h"
#include "ScreenGrab.h"

// AntTweakBar includes
#include "AntTweakBar.h"

// Internal includes
#include "util/util.h"
#include "util/FFmpeg.h"

#define TEMPLATE_DEMO
#define MASS_SPRING_SYSTEM
#define RIGID_BODY_SIMULATION
#define RIGID_BODY_COLLISION

#include "PositionableUnit.cpp"

// Mass Spring includes
#include "spring.h"
#include "point.h"
#include <vector>
#include <list>
#include <Windows.h>

//Rigid Body includes
#include "rigidBody.h"
#include "collisionDetect.h"
#include "Contact.h"

//Fluid Simulation
#include "Fluid.h"
#include "FluidSimulation.h"
#include "Particle.h"

// DXUT camera
// NOTE: CModelViewerCamera does not only manage the standard view transformation/camera position 
//       (CModelViewerCamera::GetViewMatrix()), but also allows for model rotation
//       (CModelViewerCamera::GetWorldMatrix()). 
//       Look out for CModelViewerCamera::SetButtonMasks(...).
CModelViewerCamera g_camera;

// Effect corresponding to "effect.fx"
ID3DX11Effect* g_pEffect = nullptr;
ID3D11Device* g_pPd3Device = nullptr;
// Main tweak bar
TwBar* g_pTweakBar;

// DirectXTK effects, input layouts and primitive batches for different vertex types
// (for drawing multicolored & unlit primitives)
BasicEffect*                          g_pEffectPositionColor          = nullptr;
ID3D11InputLayout*                    g_pInputLayoutPositionColor     = nullptr;
PrimitiveBatch<VertexPositionColor>*  g_pPrimitiveBatchPositionColor  = nullptr;

// DirectXTK effect, input layout and primitive batch for position/normal vertices
// (for drawing unicolor & oriented & lit primitives)
BasicEffect*                          g_pEffectPositionNormal         = nullptr;
ID3D11InputLayout*                    g_pInputLayoutPositionNormal    = nullptr;
PrimitiveBatch<VertexPositionNormal>* g_pPrimitiveBatchPositionNormal = nullptr;

BasicEffect*                               g_pEffectPositionNormalColor         = nullptr;
ID3D11InputLayout*                         g_pInputLayoutPositionNormalColor    = nullptr;
PrimitiveBatch<VertexPositionNormalColor>* g_pPrimitiveBatchPositionNormalColor = nullptr;

// DirectXTK simple geometric primitives
std::unique_ptr<GeometricPrimitive> g_pSphere;
std::unique_ptr<GeometricPrimitive> g_pTeapot;
std::unique_ptr<GeometricPrimitive> g_pCube;

// Movable object management
XMINT2   g_viMouseDelta = XMINT2(0,0);
XMFLOAT3 g_vfMovableObjectPos = XMFLOAT3(0,0,0);
XMFLOAT3 g_vfRotate = XMFLOAT3(0, 0, 0);


// TweakAntBar GUI variables

//startup demo
int g_iTestCase = 8;
int g_iPreTestCase = -1;
bool  g_bSimulateByStep = false;
bool  g_bIsSpaceReleased = true;
#ifdef TEMPLATE_DEMO
int   g_iNumSpheres    = 100;
float g_fSphereSize    = 0.05f;
bool  g_bDrawTeapot    = true;
bool  g_bDrawTriangle  = true;
bool  g_bDrawSpheres = true;
#endif
#ifdef MASS_SPRING_SYSTEM
bool g_bDrawMassSpringSystem = true;
XMVECTORF32 TUM_BLUE = {0, 0.396, 0.741,1};
XMVECTORF32 TUM_BLUE_LIGHT = {.259, .522, .957,1};
int g_integrationMethod = 0, g_preIntegrationMethod = 0;
int g_demoCase = 0, g_preDemoCase = 0;
float groundFriction, groundBouncyness;
float g_xWall =1.0f, g_zWall=1.0f, g_ceiling=1.0f;
bool g_usingWalls = false;
float g_explosionForce =1;

bool g_firstStep = true;
#endif

//general variables for both simulations
DWORD previousTime, currentTime;
float deltaTime = 0;
bool g_fixedTimestep = false;
float g_manualTimestep = 0.005;
float g_gravity = -9.81;
bool g_useGravity = true;
bool g_useDamping = true;

int g_windowWidth;
int g_windowHeight;

#ifdef RIGID_BODY_SIMULATION

bool g_bDrawRigidBodySimulation = true;
std::vector<MassPoint>* pointList;
rigidBody* rb;
XMFLOAT3 forceStart;
XMFLOAT3 forceEnd;
float g_damping_linear = 0.3f;
float g_damping_angular = 0.3f;

#endif
#ifdef RIGID_BODY_COLLISION

bool g_bDrawRigidBodyCollision = true;
std::vector<MassPoint>* pointList1, * pointList2;
rigidBody* rb1, * rb2;
std::vector<rigidBody>* rigidBodies; // for rb demo 4
rigidBody* floorRB;

XMMATRIX mat1, mat2;
CollisionInfo simpletest;
Contact contact;
#endif

//Fluid Simulation
Fluid* fluid;
FluidSimulation* fluidSim;

void InitRigidBox(std::vector<MassPoint>* listOfPoints, float width, float height, float depth, float mass) {
	width /= 2;
	height /= 2;
	depth /= 2;
	mass /= 8;

	listOfPoints->push_back(MassPoint(XMFLOAT3(width,height,depth), mass ));
	listOfPoints->push_back(MassPoint(XMFLOAT3(width,-height,-depth), mass ));
	listOfPoints->push_back(MassPoint(XMFLOAT3(width,height,-depth), mass ));
	listOfPoints->push_back(MassPoint(XMFLOAT3(-width,-height,depth), mass ));
	listOfPoints->push_back(MassPoint(XMFLOAT3(-width,height,depth), mass ));
	listOfPoints->push_back(MassPoint(XMFLOAT3(-width,-height,-depth), mass ));
	listOfPoints->push_back(MassPoint(XMFLOAT3(-width,height,-depth), mass ));
	listOfPoints->push_back(MassPoint(XMFLOAT3(width,-height,depth), mass ));
}

void InitRigidBodies()
{
	rigidBody* rbTemp;
	std::vector<MassPoint> * pointListTemp;
	float w = 0.0f, h = 0.0f, d = 0.0f;
	switch(g_iTestCase) {
	case 4:
	case 5:
		pointList = new std::vector<MassPoint>;
		w = 1.0f, h = 0.6f, d = 0.5f;
		InitRigidBox(pointList, w,h,d,2.f);
		for(auto mp = pointList->begin(); mp != pointList->end(); mp++) {
			//if (mp->position.x == 0.5f && mp->position.x == -0.3f && mp->position.x ==0.25f) {
				mp->SetForce(XMFLOAT3(1.f,1.f,0.f));
				break;
			//}
		}

		rb = new rigidBody(pointList, XMFLOAT3(.0f , .0f, .0f), XMFLOAT3(.0f , .0f, 1.5708f), XMFLOAT3(w, h, d));
		break;
	case 6:
		std::cout << "init.ing rigid body collision\n";;
		//Init Rigid Body Collision
		w = 1.0f, h = 0.6f, d = 0.5f;
		w /= 2, h /= 2, d /= 2;
		pointList1 = new std::vector<MassPoint>;
		pointList2 = new std::vector<MassPoint>;
		InitRigidBox(pointList1, w,h,d,2.f);
		InitRigidBox(pointList2, w,h,d,2.f);
		rb1 = new rigidBody(pointList1, XMFLOAT3(.0f , -1.f, .0f), XMFLOAT3(.0f , .0f, 0.785398f), XMFLOAT3(d/2, h, d));
		rb2 = new rigidBody(pointList2, XMFLOAT3(.0f , .0f, .0f), XMFLOAT3(.0f , .0f, .0f), XMFLOAT3(d, w, h));

		rb1->setPosition(XMFLOAT3(.0f,1.0f,.0f));
		//rb2->setPosition(XMFLOAT3(.0f,1.0f,.0f));

		mat1 = mat2 = XMMATRIX(.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f);

		std::cout << "init.ed rigid body collision\n";;
		break;
	case 7:
		w = 1.0f, h = 0.6f, d = 0.5f;
		w /= 2, h /= 2, d /= 2;
		rigidBodies = new std::vector<rigidBody>;
		for(int i = 0; i<5; i++) //init 5 rigidbodies
		{
			pointListTemp = new std::vector<MassPoint>;
			InitRigidBox(pointListTemp, w,h,d,2.f);
			rbTemp = new rigidBody(pointListTemp, XMFLOAT3(i*0.5f , -1.f, 0.5 - i*0.2), XMFLOAT3(0.4f*i , 0.1*i, 0.785398f), XMFLOAT3(d/2, h, d));

			rbTemp->setPosition(XMFLOAT3(-2+i*0.5f,1.0f+0.5*i,.0f));
			rigidBodies->push_back(*rbTemp);
		}
		for(int i = 0; i<5; i++) //init 5 rigidbodies
		{
			pointListTemp = new std::vector<MassPoint>;
			InitRigidBox(pointListTemp, w,h,d,2.f);
			rbTemp = new rigidBody(pointListTemp, XMFLOAT3(.0f , 2*i, .0f), XMFLOAT3(.01f*i , .0f, .0f), XMFLOAT3(d, w, h));
			rbTemp->setPosition(XMFLOAT3(-2+0.75f*i,.0f,.0f));
			rigidBodies->push_back(*rbTemp);
		}
		//rb2->setPosition(XMFLOAT3(.0f,1.0f,.0f));
		pointListTemp = new std::vector<MassPoint>;
		InitRigidBox(pointListTemp, 1,1,1,9999999.9f);
		floorRB = rbTemp = new rigidBody(pointListTemp, XMFLOAT3(.0f , .0f, .0f), XMFLOAT3(.0f , .0f, .0f), XMFLOAT3(500, 10, 500));
		floorRB-> setPosition(XMFLOAT3(.0f,-6,0));
		floorRB->setStatic(true);
		mat1 = mat2 = XMMATRIX(.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f,.0f);
		break;
	default:
		break;
	}
}

void DestroyRigidBodies()
{
	int anus = 0;
	switch(g_iPreTestCase) {
	case 4:
	case 5:
		std::cout << "deleting rigid body shite (d1)\n";
		delete(pointList);
		delete(rb);
		break;
	case 6:
		std::cout << "deleting rigid body shite (d2-1,";
		delete(pointList1);
		std::cout << "d2-2,";
		delete(pointList2);
		std::cout << "d2-3,";
		delete(rb1);
		std::cout << "d2-4)\n";
		delete(rb2);
		std::cout << "deleted rigid body shite\n";
		break;
	case 7:
	//	delete(pointList1);
	//	delete(rb1);
		cout<<"for ";
		delete(rigidBodies);
/*
		for(auto rb = rigidBodies.begin(); rb != rigidBodies.end();)
			{
				cout<<anus;
				auto it = rb;
				cout<<"a";
				rb++;
				cout<<"b";
				rigidBody* rbPointer =  &((rigidBody)*it);
				cout<<"c";
				rigidBodies.erase(it);
				cout<<"d";
				//delete(rbPointer);
				cout<<"e";
				anus++;
			}
			*/
		break;
	default:
		break;
	}
}

void ResetRigidBodies()
{
	DestroyRigidBodies();
	InitRigidBodies();
}

// Mass Spring variable
std::list<Spring> springs;
std::list<SpringPoint*> points;
void InitMassSprings()
{
	if(g_demoCase == 0) {
		Spring g_spring1, g_spring2;
		SpringPoint* g_point1,* g_point2,* g_point3;

		g_point1 = new SpringPoint(XMFLOAT3(0.0f,0,0));
		g_point1->setVelocity(XMFLOAT3(-1,0,0));
		g_point1->setDamping(0.2f);

		g_point2 = new SpringPoint(XMFLOAT3(0,0.2f,0));
		g_point2->setVelocity(XMFLOAT3(1,0,0));
		g_point2->setDamping(0.2f);
		//g_point2->gp_isStatic = true;

		g_spring1.setPoint(1, g_point1);
		g_spring1.setPoint(2, g_point2);


		points.push_back(g_point1);
		points.push_back(g_point2);
		springs.push_back(g_spring1);
	} else if(g_demoCase == 1) {
		//TWO RANDOM 5-Springs
		Spring g_springs[15] = {};
		SpringPoint* g_points[18] = {};
		float x1 = 0;
		float vx1 = -1;
		float x2 = -2;
		float vx2 = -1;
		float x3 = 1;
		float vx3 = -4;
		
		for(int i = 0; i < 6; i++) {
			g_points[i] = new SpringPoint(XMFLOAT3(x1,x1,x1));
			g_points[i]->setVelocity(XMFLOAT3(vx1,0,0));
			g_points[i]->setDamping(0.2f);
			x1 += 1;
			vx1 *= -1;
			points.push_back(g_points[i]);

			g_points[i+6] = new SpringPoint(XMFLOAT3(-x2,x2,-x2));
			g_points[i+6]->setVelocity(XMFLOAT3(0,0,vx2));
			g_points[i+6]->setDamping(0.1f);
			x2 += 1;
			vx2 *= -1;
			points.push_back(g_points[i+6]);

			g_points[i+12] = new SpringPoint(XMFLOAT3(x2,-x3,-x1));
			g_points[i+12]->setVelocity(XMFLOAT3(vx2,vx1,vx3));
			g_points[i+12]->setDamping(0.1f);
			x3 += 1;
			vx3 *= -1;
			points.push_back(g_points[i+12]);
		}
		g_points[12]->gp_isStatic = true;
		g_points[12]->setPosition(XMFLOAT3(0,1,0));

		for(int i = 0; i < 5; i++) {
			g_springs[i].setPoint(1, g_points[i]);
			g_springs[i].setPoint(2, g_points[i+1]);
			springs.push_back(g_springs[i]);

			g_springs[i+5].setPoint(1, g_points[i+6]);
			g_springs[i+5].setPoint(2, g_points[i+6+1]);
			springs.push_back(g_springs[i+5]);

			g_springs[i+10].setPoint(1, g_points[i+12]);
			g_springs[i+10].setPoint(2, g_points[i+12+1]);
			springs.push_back(g_springs[i+10]);
		}
		Spring cirle;
		cirle.setPoint(1,g_points[14]);
		cirle.setPoint(2,g_points[17]);
		springs.push_back(cirle);
		cirle.setPoint(1,g_points[1]);
		cirle.setPoint(2,g_points[17]);
		springs.push_back(cirle);


		//GRID STUFF
		/*Spring g_springs[24] = {};
		SpringPoint* g_points[16] = {};
		float x1 = 0;
		float vx1 = -1;
		float x2 = -2;
		float vx2 = -1;
		
		for(int i = 0; i < 6; i++) {
			g_points[i] = new SpringPoint(XMFLOAT3(x1,x1,x1));
			g_points[i]->setVelocity(XMFLOAT3(vx1,0,0));
			g_points[i]->setDamping(0.2f);
			x1 += 1;
			vx1 *= -1;
			points.push_back(g_points[i]);

			g_points[i+6] = new SpringPoint(XMFLOAT3(-x2,x2,-x2));
			g_points[i+6]->setVelocity(XMFLOAT3(0,0,vx2));
			g_points[i+6]->setDamping(0.1f);
			x2 += 1;
			vx2 *= -1;
			points.push_back(g_points[i+6]);
		}

		for(int i = 0; i < 5; i++) {
			g_springs[i].setPoint(1, g_points[i]);
			g_springs[i].setPoint(2, g_points[i+1]);
			springs.push_back(g_springs[i]);

			g_springs[i+5].setPoint(1, g_points[i+6]);
			g_springs[i+5].setPoint(2, g_points[i+6+1]);
			springs.push_back(g_springs[i+5]);
		}*/
	}
}

void DestroyMassSprings()
{
	for(auto point = points.begin(); point != points.end();)
	{
		auto it = point;
		point++;
		SpringPoint* pointPointer =  ((SpringPoint*)*it);
		points.erase(it);
		free(pointPointer);
	}
	for(auto spring = springs.begin(); spring != springs.end();)
	{
		auto it = spring;
		spring++;
		springs.erase(it);
	}
}

void ResetMassSprings(float deltaTime) {
	DestroyMassSprings();
	InitMassSprings();
	if(g_integrationMethod == 2) {
		SpringPoint* a;
		Spring* b;
		for(auto spring = springs.begin(); spring != springs.end();spring++)
		{
			b= &(((Spring)*spring));
			b->computeElasticForces();
		}
		for(auto point = points.begin(); point != points.end();point++)
		{
			a =  (((SpringPoint*)*point));
			if(g_useGravity) { a->addGravity(g_gravity); }
			a->computeAcceleration();
			a->IntegrateVelocity(deltaTime/2.0f);
			a->resetForces();
		}	
	}

	if(g_iTestCase == 0)
	{
		Spring* b;
		for(auto spring = springs.begin(); spring != springs.end(); spring++)
		{
			b= &((Spring)*spring);
			if(g_integrationMethod == 0)
				{std::cout << "\nEuler demo1 initially:\n";}
			else if(g_integrationMethod == 1)
				{std::cout << "\nMidpoint demo1 initially:\n";}
			b->printSpring();
		}
	}
	g_firstStep = true;
}
//simulates an explosion one meter under the ground
void explode()
{
	if(g_iTestCase == 3)
	{
		SpringPoint* a;
		for(auto point = points.begin(); point != points.end();point++)
		{
			a =  (((SpringPoint*)*point)); 
			//a->setVelocity(addVector(a->gp_velocity,invertVector(addVector(a->gp_position,XMFLOAT3(0,3,0)),g_explosionForce)));
			a->setVelocity(addVector(a->gp_velocity,multiplyVector(normalizeVector(addVector(a->gp_position,XMFLOAT3(0,2,0))),g_explosionForce/vectorLength(a->gp_position))));
		
		}
	}
		if(g_iTestCase == 7)
		{
			rigidBody* b;
			for(auto rb = rigidBodies->begin(); rb != rigidBodies->end();rb++)
			{
				b = &*rb;
				//a->setVelocity(addVector(a->gp_velocity,invertVector(addVector(a->gp_position,XMFLOAT3(0,3,0)),g_explosionForce)));
				b->setLinearVelocity(addVector(b->getVelocity(),multiplyVector(normalizeVector(addVector(b->getPosition(),XMFLOAT3(0,2,0))),g_explosionForce/vectorLength(b->getPosition()))));
			}
		}

}



// Video recorder
FFmpeg* g_pFFmpegVideoRecorder = nullptr;

// Create TweakBar and add required buttons and variables
void InitTweakBar(ID3D11Device* pd3dDevice)
{
    g_pTweakBar = TwNewBar("TweakBar");
	TwDefine(" TweakBar color='0 128 128' alpha=128 ");

	TwType TW_TYPE_INTEGRATOR = TwDefineEnumFromString("Integration Method", "Euler,Midpoint,LeapFrog");
	TwType TW_TYPE_DEMOCASE = TwDefineEnumFromString("Demo Setup", "Demo 1/2/3,Demo 4");
	TwType TW_TYPE_TESTCASE = TwDefineEnumFromString("Test Scene", "MSS Demo 1,MSS Demo 2,MSS Demo 3,MSS Demo 4, RB Demo 1, RB Demo 2, RB Demo 3, RB Demo 4, FlSim Demo");
	TwAddVarRW(g_pTweakBar, "Test Scene", TW_TYPE_TESTCASE, &g_iTestCase, "");
	// HINT: For buttons you can directly pass the callback function as a lambda expression.
	TwAddButton(g_pTweakBar, "Reset Scene", [](void *){g_iPreTestCase = -1; }, nullptr, "");
	TwAddButton(g_pTweakBar, "Reset Camera", [](void *){g_camera.Reset(); }, nullptr, "");
	// Run mode, step by step, control by space key
	TwAddVarRW(g_pTweakBar, "RunStep(space)", TW_TYPE_BOOLCPP, &g_bSimulateByStep, "");
	
#ifdef TEMPLATE_DEMO
	switch (g_iTestCase)
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
#ifdef MASS_SPRING_SYSTEM
	case 3:
		TwAddVarRW(g_pTweakBar, "Demo Setup", TW_TYPE_DEMOCASE, &g_demoCase, "");
		TwAddVarRW(g_pTweakBar, "-> Integration Method", TW_TYPE_INTEGRATOR, &g_integrationMethod, "");
		TwAddVarRW(g_pTweakBar, "Use damping", TW_TYPE_BOOLCPP, &g_useDamping, "");
		TwAddVarRW(g_pTweakBar, "Point Size", TW_TYPE_FLOAT, &g_fSphereSize, "min=0.01 step=0.01");
		TwAddVarRW(g_pTweakBar, "Use fixed timestep", TW_TYPE_BOOLCPP, &g_fixedTimestep, "");
		TwAddVarRW(g_pTweakBar, "-> timestep (ms)", TW_TYPE_FLOAT, &g_manualTimestep, "min=0.001 step=0.001");
		TwAddVarRW(g_pTweakBar, "Use gravity", TW_TYPE_BOOLCPP, &g_useGravity, "");
		TwAddVarRW(g_pTweakBar, "-> gravity constant", TW_TYPE_FLOAT, &g_gravity, "min=-20 ma=20 step=0.1");
		TwAddVarRW(g_pTweakBar, "Collide with walls:", TW_TYPE_BOOLCPP, &g_usingWalls, "");
		TwAddVarRW(g_pTweakBar, "-> X-Wall Positions", TW_TYPE_FLOAT, &g_xWall, "min=0.5 ma=10 step=0.1");
		TwAddVarRW(g_pTweakBar, "-> Z-Wall Positions", TW_TYPE_FLOAT, &g_zWall, "min=0.5 ma=10 step=0.1");
		TwAddVarRW(g_pTweakBar, "-> Ceiling height", TW_TYPE_FLOAT, &g_ceiling, "min=0.5 ma=10 step=0.1");
		TwAddButton(g_pTweakBar, "Explode!!", [](void *){explode(); }, nullptr, "");	
		TwAddVarRW(g_pTweakBar, "-> Explosion Force", TW_TYPE_FLOAT, &g_explosionForce, "min=0.1 ma=10 step=0.1");
		break;
#endif
#ifdef RIGID_BODY_SIMULATION
	case 4:
		//RB Demo 1

		//TwAddVarRW(g_pTweakBar, "Demo Setup", TW_TYPE_DEMOCASE, &g_demoCase, "");
		//TwAddVarRW(g_pTweakBar, "-> Integration Method", TW_TYPE_INTEGRATOR, &g_integrationMethod, "");
		//TwAddVarRW(g_pTweakBar, "Use damping", TW_TYPE_BOOLCPP, &g_useDamping, "");
		//TwAddVarRW(g_pTweakBar, "Point Size", TW_TYPE_FLOAT, &g_fSphereSize, "min=0.01 step=0.01");
		TwAddVarRW(g_pTweakBar, "Use fixed timestep", TW_TYPE_BOOLCPP, &g_fixedTimestep, "");
		TwAddVarRW(g_pTweakBar, "-> timestep (ms)", TW_TYPE_FLOAT, &g_manualTimestep, "min=0.001 step=0.001");
		TwAddVarRW(g_pTweakBar, "Use gravity", TW_TYPE_BOOLCPP, &g_useGravity, "");
		TwAddVarRW(g_pTweakBar, "-> gravity constant", TW_TYPE_FLOAT, &g_gravity, "min=-20 ma=20 step=0.1");
		//TwAddVarRW(g_pTweakBar, "Collide with walls:", TW_TYPE_BOOLCPP, &g_usingWalls, "");
		//TwAddVarRW(g_pTweakBar, "-> X-Wall Positions", TW_TYPE_FLOAT, &g_xWall, "min=0.5 ma=10 step=0.1");
		//TwAddVarRW(g_pTweakBar, "-> Z-Wall Positions", TW_TYPE_FLOAT, &g_zWall, "min=0.5 ma=10 step=0.1");
		//TwAddVarRW(g_pTweakBar, "-> Ceiling height", TW_TYPE_FLOAT, &g_ceiling, "min=0.5 ma=10 step=0.1");
		//TwAddButton(g_pTweakBar, "Explode!!", [](void *){explode(); }, nullptr, "");	
		//TwAddVarRW(g_pTweakBar, "-> Explosion Force", TW_TYPE_FLOAT, &g_explosionForce, "min=0.1 ma=10 step=0.1");
		break;
#endif
	case 5:
		//RB Demo 2
		break;
#ifdef RIGID_BODY_COLLISION
	case 6:
		//RB Demo 3

		//TwAddVarRW(g_pTweakBar, "Demo Setup", TW_TYPE_DEMOCASE, &g_demoCase, "");
		//TwAddVarRW(g_pTweakBar, "-> Integration Method", TW_TYPE_INTEGRATOR, &g_integrationMethod, "");
		//TwAddVarRW(g_pTweakBar, "Use damping", TW_TYPE_BOOLCPP, &g_useDamping, "");
		//TwAddVarRW(g_pTweakBar, "Point Size", TW_TYPE_FLOAT, &g_fSphereSize, "min=0.01 step=0.01");
		//TwAddVarRW(g_pTweakBar, "Use fixed timestep", TW_TYPE_BOOLCPP, &g_fixedTimestep, "");
		//TwAddVarRW(g_pTweakBar, "-> timestep (ms)", TW_TYPE_FLOAT, &g_manualTimestep, "min=0.001 step=0.001");
		//TwAddVarRW(g_pTweakBar, "Use gravity", TW_TYPE_BOOLCPP, &g_useGravity, "");
		//TwAddVarRW(g_pTweakBar, "-> gravity constant", TW_TYPE_FLOAT, &g_gravity, "min=-20 ma=20 step=0.1");
		//TwAddVarRW(g_pTweakBar, "Collide with walls:", TW_TYPE_BOOLCPP, &g_usingWalls, "");
		//TwAddVarRW(g_pTweakBar, "-> X-Wall Positions", TW_TYPE_FLOAT, &g_xWall, "min=0.5 ma=10 step=0.1");
		//TwAddVarRW(g_pTweakBar, "-> Z-Wall Positions", TW_TYPE_FLOAT, &g_zWall, "min=0.5 ma=10 step=0.1");
		//TwAddVarRW(g_pTweakBar, "-> Ceiling height", TW_TYPE_FLOAT, &g_ceiling, "min=0.5 ma=10 step=0.1");
		//TwAddButton(g_pTweakBar, "Explode!!", [](void *){explode(); }, nullptr, "");	
		//TwAddVarRW(g_pTweakBar, "-> Explosion Force", TW_TYPE_FLOAT, &g_explosionForce, "min=0.1 ma=10 step=0.1");
		break;
#endif
	case 7:
		//RB Demo 4
		TwAddVarRW(g_pTweakBar, "Use fixed timestep", TW_TYPE_BOOLCPP, &g_fixedTimestep, "");
		TwAddVarRW(g_pTweakBar, "-> timestep (ms)", TW_TYPE_FLOAT, &g_manualTimestep, "min=0.001 step=0.001");
		TwAddVarRW(g_pTweakBar, "Use gravity", TW_TYPE_BOOLCPP, &g_useGravity, "");
		TwAddVarRW(g_pTweakBar, "-> gravity constant", TW_TYPE_FLOAT, &g_gravity, "min=-20 ma=20 step=0.1");
		TwAddButton(g_pTweakBar, "Explode!!", [](void *){explode(); }, nullptr, "");	
		TwAddVarRW(g_pTweakBar, "-> Explosion Force", TW_TYPE_FLOAT, &g_explosionForce, "min=0.1 ma=10 step=0.1");
		TwAddVarRW(g_pTweakBar, "Use damping", TW_TYPE_BOOLCPP, &g_useDamping, "");
		TwAddVarRW(g_pTweakBar, "-> Linear Damping", TW_TYPE_FLOAT, &g_damping_linear, "min=0 ma=10 step=0.1");
		TwAddVarRW(g_pTweakBar, "-> Angular Damping", TW_TYPE_FLOAT, &g_damping_angular, "min=0 ma=10 step=0.1");
		break;
	default:
		break;
	}
#endif
}

// Draw the edges of the bounding box [-0.5;0.5]� rotated with the cameras model tranformation.
// (Drawn as line primitives using a DirectXTK primitive batch)
void DrawBoundingBox(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Setup position/color effect
    g_pEffectPositionColor->SetWorld(g_camera.GetWorldMatrix());
    
    g_pEffectPositionColor->Apply(pd3dImmediateContext);
    pd3dImmediateContext->IASetInputLayout(g_pInputLayoutPositionColor);

    // Draw
    g_pPrimitiveBatchPositionColor->Begin();
    
    // Lines in x direction (red color)
    for (int i=0; i<4; i++)
    {
        g_pPrimitiveBatchPositionColor->DrawLine(
            VertexPositionColor(XMVectorSet(-0.5f, (float)(i%2)-0.5f, (float)(i/2)-0.5f, 1), Colors::Red),
            VertexPositionColor(XMVectorSet( 0.5f, (float)(i%2)-0.5f, (float)(i/2)-0.5f, 1), Colors::Red)
        );
    }

    // Lines in y direction
    for (int i=0; i<4; i++)
    {
        g_pPrimitiveBatchPositionColor->DrawLine(
            VertexPositionColor(XMVectorSet((float)(i%2)-0.5f, -0.5f, (float)(i/2)-0.5f, 1), Colors::Green),
            VertexPositionColor(XMVectorSet((float)(i%2)-0.5f,  0.5f, (float)(i/2)-0.5f, 1), Colors::Green)
        );
    }

    // Lines in z direction
    for (int i=0; i<4; i++)
    {
        g_pPrimitiveBatchPositionColor->DrawLine(
            VertexPositionColor(XMVectorSet((float)(i%2)-0.5f, (float)(i/2)-0.5f, -0.5f, 1), Colors::Blue),
            VertexPositionColor(XMVectorSet((float)(i%2)-0.5f, (float)(i/2)-0.5f,  0.5f, 1), Colors::Blue)
        );
    }

    g_pPrimitiveBatchPositionColor->End();
}

// Draw a large, square plane at y=-1 with a checkerboard pattern
// (Drawn as multiple quads, i.e. triangle strips, using a DirectXTK primitive batch)
void DrawFloor(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Setup position/normal/color effect
    g_pEffectPositionNormalColor->SetWorld(XMMatrixIdentity());
    g_pEffectPositionNormalColor->SetEmissiveColor(Colors::Black);
    g_pEffectPositionNormalColor->SetDiffuseColor(0.8f * Colors::White);
    g_pEffectPositionNormalColor->SetSpecularColor(0.4f * Colors::White);
    g_pEffectPositionNormalColor->SetSpecularPower(1000);
    
    g_pEffectPositionNormalColor->Apply(pd3dImmediateContext);
    pd3dImmediateContext->IASetInputLayout(g_pInputLayoutPositionNormalColor);

    // Draw 4*n*n quads spanning x = [-n;n], y = -1, z = [-n;n]
    const float n = 4;
    XMVECTOR normal      = XMVectorSet(0, 1,0,0);
    XMVECTOR planecenter = XMVectorSet(0,-1,0,0);

    g_pPrimitiveBatchPositionNormalColor->Begin();
    for (float z = -n; z < n; z++)
    {
        for (float x = -n; x < n; x++)
        {
            // Quad vertex positions
            XMVECTOR pos[] = { XMVectorSet(x  , -1, z+1, 0), 
                               XMVectorSet(x+1, -1, z+1, 0),
                               XMVectorSet(x+1, -1, z  , 0),
                               XMVectorSet(x  , -1, z  , 0) };

            // Color checkerboard pattern (white & gray)
            XMVECTOR color = ((int(z + x) % 2) == 0) ? XMVectorSet(1,1,1,1) : XMVectorSet(0.6f,0.6f,0.6f,1);

            // Color attenuation based on distance to plane center
            float attenuation[] = {
                1.0f - XMVectorGetX(XMVector3Length(pos[0] - planecenter)) / n,
                1.0f - XMVectorGetX(XMVector3Length(pos[1] - planecenter)) / n,
                1.0f - XMVectorGetX(XMVector3Length(pos[2] - planecenter)) / n,
                1.0f - XMVectorGetX(XMVector3Length(pos[3] - planecenter)) / n };

            g_pPrimitiveBatchPositionNormalColor->DrawQuad(
                VertexPositionNormalColor(pos[0], normal, attenuation[0] * color),
                VertexPositionNormalColor(pos[1], normal, attenuation[1] * color),
                VertexPositionNormalColor(pos[2], normal, attenuation[2] * color),
                VertexPositionNormalColor(pos[3], normal, attenuation[3] * color)
            );
        }
    }
    g_pPrimitiveBatchPositionNormalColor->End();    
}

#ifdef TEMPLATE_DEMO
// Draw several objects randomly positioned in [-0.5f;0.5]�  using DirectXTK geometric primitives.
void DrawSomeRandomObjects(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Setup position/normal effect (constant variables)
    g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
    g_pEffectPositionNormal->SetSpecularColor(0.4f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(100);
      
    std::mt19937 eng;
    std::uniform_real_distribution<float> randCol( 0.0f, 1.0f);
    std::uniform_real_distribution<float> randPos(-0.5f, 0.5f);

    for (int i=0; i<g_iNumSpheres; i++)
    {
        // Setup position/normal effect (per object variables)
        g_pEffectPositionNormal->SetDiffuseColor(0.6f * XMColorHSVToRGB(XMVectorSet(randCol(eng), 1, 1, 0)));
        XMMATRIX scale    = XMMatrixScaling(g_fSphereSize, g_fSphereSize, g_fSphereSize);
        XMMATRIX trans    = XMMatrixTranslation(randPos(eng),randPos(eng),randPos(eng));
        g_pEffectPositionNormal->SetWorld(scale * trans * g_camera.GetWorldMatrix());

        // Draw
        // NOTE: The following generates one draw call per object, so performance will be bad for n>>1000 or so
        g_pSphere->Draw(g_pEffectPositionNormal, g_pInputLayoutPositionNormal);
    }
}

// Draw a teapot at the position g_vfMovableObjectPos.
void DrawMovableTeapot(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Setup position/normal effect (constant variables)
    g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
    g_pEffectPositionNormal->SetDiffuseColor(0.6f * Colors::Cornsilk);
    g_pEffectPositionNormal->SetSpecularColor(0.4f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(100);

    XMMATRIX scale    = XMMatrixScaling(0.5f, 0.5f, 0.5f);    
    XMMATRIX trans    = XMMatrixTranslation(g_vfMovableObjectPos.x, g_vfMovableObjectPos.y, g_vfMovableObjectPos.z);
	XMMATRIX rotations = XMMatrixRotationRollPitchYaw(g_vfRotate.x, g_vfRotate.y, g_vfRotate.z);
	g_pEffectPositionNormal->SetWorld(rotations * scale * trans);

    // Draw
    g_pTeapot->Draw(g_pEffectPositionNormal, g_pInputLayoutPositionNormal);
}

// Draw a simple triangle using custom shaders (g_pEffect)
void DrawTriangle(ID3D11DeviceContext* pd3dImmediateContext)
{
	XMMATRIX world = g_camera.GetWorldMatrix();
	XMMATRIX view  = g_camera.GetViewMatrix();
	XMMATRIX proj  = g_camera.GetProjMatrix();
	XMFLOAT4X4 mViewProj;
	XMStoreFloat4x4(&mViewProj, world * view * proj);
	g_pEffect->GetVariableByName("g_worldViewProj")->AsMatrix()->SetMatrix((float*)mViewProj.m);
	g_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(0)->Apply(0, pd3dImmediateContext);
    
	pd3dImmediateContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	pd3dImmediateContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
	pd3dImmediateContext->IASetInputLayout(nullptr);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dImmediateContext->Draw(3, 0);
}
#endif

#ifdef MASS_SPRING_SYSTEM

void DrawPoint(ID3D11DeviceContext* pd3dImmediateContext, SpringPoint* point)
{
	//set color
	g_pEffectPositionNormal->SetDiffuseColor(TUM_BLUE_LIGHT);
	g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
	g_pEffectPositionNormal->SetSpecularColor(0.5f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(50);

	//set position
	XMMATRIX scale    = XMMatrixScaling(g_fSphereSize, g_fSphereSize, g_fSphereSize);
	XMMATRIX trans    = XMMatrixTranslation(point->gp_position.x,point->gp_position.y,point->gp_position.z);
    g_pEffectPositionNormal->SetWorld(scale * trans * g_camera.GetWorldMatrix());

	//draw everything
    g_pSphere->Draw(g_pEffectPositionNormal, g_pInputLayoutPositionNormal);
}

void DrawParticle(Particle& particle, float size)
{
	/*std::mt19937 eng;
    std::uniform_real_distribution<float> randCol( 0.0f, 1.0f);

	g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
    g_pEffectPositionNormal->SetSpecularColor(0.4f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(100);
	g_pEffectPositionNormal->SetDiffuseColor(0.6f * XMColorHSVToRGB(XMVectorSet(randCol(eng), 1, 1, 0)));*/

	//set color
	g_pEffectPositionNormal->SetDiffuseColor(TUM_BLUE_LIGHT);
	g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
	g_pEffectPositionNormal->SetSpecularColor(0.5f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(50);

	//set position
	XMMATRIX scale    = XMMatrixScaling(size, size, size);
	XMMATRIX trans    = XMMatrixTranslation(particle.getPosition().x,particle.getPosition().y,particle.getPosition().z);
    g_pEffectPositionNormal->SetWorld(scale * trans * g_camera.GetWorldMatrix());

	//draw everything
    g_pSphere->Draw(g_pEffectPositionNormal, g_pInputLayoutPositionNormal);
}

void DrawSpring(ID3D11DeviceContext* pd3dImmediateContext, Spring* spring)
{
	g_pEffectPositionColor->SetWorld(g_camera.GetWorldMatrix());
    
    g_pEffectPositionColor->Apply(pd3dImmediateContext);
    pd3dImmediateContext->IASetInputLayout(g_pInputLayoutPositionColor);

    // Draw
    g_pPrimitiveBatchPositionColor->Begin();
    
	g_pPrimitiveBatchPositionColor->DrawLine(
		VertexPositionColor(XMVectorSet(spring->gs_point1->gp_position.x,spring->gs_point1->gp_position.y,spring->gs_point1->gp_position.z, 1),TUM_BLUE),
		VertexPositionColor(XMVectorSet(spring->gs_point2->gp_position.x,spring->gs_point2->gp_position.y,spring->gs_point2->gp_position.z, 1), Colors::White)
    );
    
	g_pPrimitiveBatchPositionColor->End();
}

void DrawMassSpringSystem(ID3D11DeviceContext* pd3dImmediateContext)
{
	for(auto point = points.begin(); point != points.end();point++)
	{
		DrawPoint(pd3dImmediateContext, ((SpringPoint*)*point));
	}	
	int i =0;
	for(auto spring = springs.begin(); spring != springs.end();spring++)
	{
		DrawSpring(pd3dImmediateContext, &((Spring)*spring));
		i++;
	}
}


#endif

#ifdef RIGID_BODY_SIMULATION

void DrawCube(rigidBody* rb) {
	//set color
	g_pEffectPositionNormal->SetDiffuseColor(TUM_BLUE_LIGHT);
	g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
	g_pEffectPositionNormal->SetSpecularColor(0.5f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(50);

	/* //FROM TEAPOT:
	XMMATRIX scale    = XMMatrixScaling(0.5f, 0.5f, 0.5f);    
    XMMATRIX trans    = XMMatrixTranslation(g_vfMovableObjectPos.x, g_vfMovableObjectPos.y, g_vfMovableObjectPos.z);
	XMMATRIX rotations = XMMatrixRotationRollPitchYaw(g_vfRotate.x, g_vfRotate.y, g_vfRotate.z);
	g_pEffectPositionNormal->SetWorld(rotations * scale * trans);
	*/

	//set position
	//cout << "scale x,y,z: " << rb->scale.x << ", " << rb->scale.y << ", " << rb->scale.z << std::endl;
	//cout << "pos x,y,z: " << rb->r_position.x << ", " << rb->r_position.y << ", " << rb->r_position.z << std::endl;
	XMMATRIX scale    = XMMatrixScaling(rb->getScale().x, rb->getScale().y, rb->getScale().z);
	XMMATRIX trans    = XMMatrixTranslation(rb->getPosition().x,rb->getPosition().y,rb->getPosition().z);
	XMMATRIX rotation = XMMatrixRotationQuaternion(XMLoadFloat4(&rb->getRotationQuaternion()));
    g_pEffectPositionNormal->SetWorld( scale * rotation * trans/* g_camera.GetWorldMatrix()*/); //scale * trans * rotation * g_camera.GetWorldMatrix());

	//draw everything
    g_pCube->Draw(g_pEffectPositionNormal, g_pInputLayoutPositionNormal);
}

#endif
#ifdef RIGID_BODY_COLLISION
XMMATRIX getObj2WorldMat(rigidBody* rb1){
		XMMATRIX scale1    = XMMatrixScaling(rb1->getScale().x, rb1->getScale().y, rb1->getScale().z);
		XMMATRIX trans1    = XMMatrixTranslation(rb1->getPosition().x,rb1->getPosition().y,rb1->getPosition().z);
		XMMATRIX rotation1 = XMMatrixRotationQuaternion(XMLoadFloat4(&rb1->getRotationQuaternion()));
		return scale1 * rotation1 * trans1;
}
void DrawCollisionCubes(rigidBody* rb1) {
	//TODO FIX ALL CODE IN THIS TO SUIT COLLISIONS
	//set color
	g_pEffectPositionNormal->SetDiffuseColor(TUM_BLUE_LIGHT);
	g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
	g_pEffectPositionNormal->SetSpecularColor(0.5f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(50);

	/* //FROM TEAPOT:
	XMMATRIX scale    = XMMatrixScaling(0.5f, 0.5f, 0.5f);    
    XMMATRIX trans    = XMMatrixTranslation(g_vfMovableObjectPos.x, g_vfMovableObjectPos.y, g_vfMovableObjectPos.z);
	XMMATRIX rotations = XMMatrixRotationRollPitchYaw(g_vfRotate.x, g_vfRotate.y, g_vfRotate.z);
	g_pEffectPositionNormal->SetWorld(rotations * scale * trans);
	*/

	//set position
	//cout << "scale x,y,z: " << rb->scale.x << ", " << rb->scale.y << ", " << rb->scale.z << std::endl;
	//cout << "pos x,y,z: " << rb->r_position.x << ", " << rb->r_position.y << ", " << rb->r_position.z << std::endl;
	XMMATRIX scale1    = XMMatrixScaling(rb1->getScale().x, rb1->getScale().y, rb1->getScale().z);
	XMMATRIX trans1    = XMMatrixTranslation(rb1->getPosition().x,rb1->getPosition().y,rb1->getPosition().z);
	XMMATRIX rotation1 = XMMatrixRotationQuaternion(XMLoadFloat4(&rb1->getRotationQuaternion()));
	XMFLOAT4X4 debug; 
	XMStoreFloat4x4(&debug, rotation1);
    g_pEffectPositionNormal->SetWorld( scale1 * rotation1 * trans1/* g_camera.GetWorldMatrix()*/); //scale * trans * rotation * g_camera.GetWorldMatrix());

	//draw everything
    g_pCube->Draw(g_pEffectPositionNormal, g_pInputLayoutPositionNormal);
}

#endif
// ============================================================
// DXUT Callbacks
// ============================================================


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependent on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();;

    std::wcout << L"Device: " << DXUTGetDeviceStats() << std::endl;
    
    // Load custom effect from "effect.fxo" (compiled "effect.fx")
	std::wstring effectPath = GetExePath() + L"effect.fxo";
	if(FAILED(hr = D3DX11CreateEffectFromFile(effectPath.c_str(), 0, pd3dDevice, &g_pEffect)))
	{
        std::wcout << L"Failed creating effect with error code " << int(hr) << std::endl;
		return hr;
	}

    // Init AntTweakBar GUI
	TwInit(TW_DIRECT3D11, pd3dDevice);
    InitTweakBar(pd3dDevice);

	// Init Mass Spring System
	InitMassSprings();

	//Init Rigid Body Simulation
	InitRigidBodies();

	//Init Fluid Simulation
	fluid = new Fluid(XMFLOAT3(0.f, .0f, 0.f), XMINT3(2, 0, 0), 7, .03f, 1.f, 200.f, .01f);

    // Create DirectXTK geometric primitives for later usage
	g_pCube = GeometricPrimitive::CreateCube(pd3dImmediateContext, 1.0f, false);
    g_pSphere = GeometricPrimitive::CreateGeoSphere(pd3dImmediateContext, 2.0f, 2, false);
    g_pTeapot = GeometricPrimitive::CreateTeapot(pd3dImmediateContext, 1.5f, 8, false);

    // Create effect, input layout and primitive batch for position/color vertices (DirectXTK)
    {
        // Effect
        g_pEffectPositionColor = new BasicEffect(pd3dDevice);
        g_pEffectPositionColor->SetVertexColorEnabled(true); // triggers usage of position/color vertices

        // Input layout
        void const* shaderByteCode;
        size_t byteCodeLength;
        g_pEffectPositionColor->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
        
        pd3dDevice->CreateInputLayout(VertexPositionColor::InputElements,
                                      VertexPositionColor::InputElementCount,
                                      shaderByteCode, byteCodeLength,
                                      &g_pInputLayoutPositionColor);

        // Primitive batch
        g_pPrimitiveBatchPositionColor = new PrimitiveBatch<VertexPositionColor>(pd3dImmediateContext);
    }

    // Create effect, input layout and primitive batch for position/normal vertices (DirectXTK)
    {
        // Effect
        g_pEffectPositionNormal = new BasicEffect(pd3dDevice);
        g_pEffectPositionNormal->EnableDefaultLighting(); // triggers usage of position/normal vertices
        g_pEffectPositionNormal->SetPerPixelLighting(true);

        // Input layout
        void const* shaderByteCode;
        size_t byteCodeLength;
        g_pEffectPositionNormal->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        pd3dDevice->CreateInputLayout(VertexPositionNormal::InputElements,
                                      VertexPositionNormal::InputElementCount,
                                      shaderByteCode, byteCodeLength,
                                      &g_pInputLayoutPositionNormal);

        // Primitive batch
        g_pPrimitiveBatchPositionNormal = new PrimitiveBatch<VertexPositionNormal>(pd3dImmediateContext);
    }

    // Create effect, input layout and primitive batch for position/normal/color vertices (DirectXTK)
    {
        // Effect
        g_pEffectPositionNormalColor = new BasicEffect(pd3dDevice);
        g_pEffectPositionNormalColor->SetPerPixelLighting(true);
        g_pEffectPositionNormalColor->EnableDefaultLighting();     // triggers usage of position/normal/color vertices
        g_pEffectPositionNormalColor->SetVertexColorEnabled(true); // triggers usage of position/normal/color vertices

        // Input layout
        void const* shaderByteCode;
        size_t byteCodeLength;
        g_pEffectPositionNormalColor->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        pd3dDevice->CreateInputLayout(VertexPositionNormalColor::InputElements,
                                      VertexPositionNormalColor::InputElementCount,
                                      shaderByteCode, byteCodeLength,
                                      &g_pInputLayoutPositionNormalColor);

        // Primitive batch
        g_pPrimitiveBatchPositionNormalColor = new PrimitiveBatch<VertexPositionNormalColor>(pd3dImmediateContext);
    }

	g_pPd3Device = pd3dDevice;
	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	SAFE_RELEASE(g_pEffect);
	
    TwDeleteBar(g_pTweakBar);
    g_pTweakBar = nullptr;
	TwTerminate();

	g_pCube.reset();
    g_pSphere.reset();
    g_pTeapot.reset();
    
    SAFE_DELETE (g_pPrimitiveBatchPositionColor);
    SAFE_RELEASE(g_pInputLayoutPositionColor);
    SAFE_DELETE (g_pEffectPositionColor);

    SAFE_DELETE (g_pPrimitiveBatchPositionNormal);
    SAFE_RELEASE(g_pInputLayoutPositionNormal);
    SAFE_DELETE (g_pEffectPositionNormal);

    SAFE_DELETE (g_pPrimitiveBatchPositionNormalColor);
    SAFE_RELEASE(g_pInputLayoutPositionNormalColor);
    SAFE_DELETE (g_pEffectPositionNormalColor);

	//Destroy Fluid Simulation
	delete(fluid);

	//Destroy Rigid Body Simulation
	DestroyRigidBodies();

	DestroyMassSprings();
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    // Update camera parameters
	int width = pBackBufferSurfaceDesc->Width;
	int height = pBackBufferSurfaceDesc->Height;

	g_windowWidth = width;
	g_windowHeight = height;

	g_camera.SetWindow(width, height);
	g_camera.SetProjParams(XM_PI / 4.0f, float(width) / float(height), 0.1f, 100.0f);

    // Inform AntTweakBar about back buffer resolution change
  	TwWindowSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    HRESULT hr;

	if(bKeyDown)
	{
		switch(nChar)
		{
            // RETURN: toggle fullscreen
			case VK_RETURN :
			{
				if(bAltDown) DXUTToggleFullScreen();
				break;
			}
            // F8: Take screenshot
			case VK_F8:
			{
                // Save current render target as png
                static int nr = 0;
				std::wstringstream ss;
				ss << L"Screenshot" << std::setfill(L'0') << std::setw(4) << nr++ << L".png";

                ID3D11Resource* pTex2D = nullptr;
                DXUTGetD3D11RenderTargetView()->GetResource(&pTex2D);
                SaveWICTextureToFile(DXUTGetD3D11DeviceContext(), pTex2D, GUID_ContainerFormatPng, ss.str().c_str());
                SAFE_RELEASE(pTex2D);

                std::wcout << L"Screenshot written to " << ss.str() << std::endl;
				break;
			}
            // F10: Toggle video recording
            case VK_F10:
            {
                if (!g_pFFmpegVideoRecorder) {
                    g_pFFmpegVideoRecorder = new FFmpeg(25, 21, FFmpeg::MODE_INTERPOLATE);
                    V(g_pFFmpegVideoRecorder->StartRecording(DXUTGetD3D11Device(), DXUTGetD3D11RenderTargetView(), "output.avi"));
                } else {
                    g_pFFmpegVideoRecorder->StopRecording();
                    SAFE_DELETE(g_pFFmpegVideoRecorder);
                }
            }			    
		}
	}
}

void applyForceByMouseDrag(int& xPos, int& yPos, int& xPosSave, int& yPosSave, rigidBody* rbs, int rbsSize, float forceScale) {
	// Accumulate deltas in g_viMouseDelta
			g_viMouseDelta.x += xPos - xPosSave;
			g_viMouseDelta.y += yPos - yPosSave;
		

			xPosSave = xPos;
			yPosSave = yPos;

			// Calcuate camera directions in world space
			XMMATRIX viewInv = XMMatrixInverse(nullptr, g_camera.GetViewMatrix());
			XMVECTOR camRightWorld = XMVector3TransformNormal(g_XMIdentityR0, viewInv);
			XMVECTOR camUpWorld = XMVector3TransformNormal(g_XMIdentityR1, viewInv);

			// Add accumulated mouse deltas to force
			XMFLOAT3 vfForce(0.f, 0.f, 0.f);
			XMVECTOR vForce = XMLoadFloat3(&vfForce);

			vForce = XMVectorAdd(vForce, forceScale * (float)g_viMouseDelta.x * camRightWorld);
			vForce = XMVectorAdd(vForce, -forceScale * (float)g_viMouseDelta.y * camUpWorld);

			XMStoreFloat3(&vfForce, vForce);

			// Reset accumulated mouse deltas
			g_viMouseDelta = XMINT2(0, 0);
		
			//calculate the closest point to the mouse
			MassPoint* closest;
			XMFLOAT3 temp(0.f, 0.f, 0.f);
			float xPosProj = ((static_cast<float>(xPos)/g_windowWidth) * 2.f - 1.f);
			float yPosProj = (((static_cast<float>(yPos)/g_windowHeight) * 2.f - 1.f) * -1.f);
			float distanceMousePoint = D3D11_FLOAT32_MAX;
			rigidBody* rbTemp;

			for (int i = 0; i < rbsSize; i++) {
				rbTemp = rbs + i;
				XMMATRIX scale    = XMMatrixScaling(rbTemp->getScale().x, rbTemp->getScale().y, rbTemp->getScale().z);
				XMMATRIX trans    = XMMatrixTranslation(rbTemp->getPosition().x,rbTemp->getPosition().y,rbTemp->getPosition().z);
				XMMATRIX rotation = XMMatrixRotationQuaternion(XMLoadFloat4(&rbTemp->getRotationQuaternion()));
				XMMATRIX modelViewProjection = g_camera.GetProjMatrix() * g_camera.GetViewMatrix() * scale * /*rotation **/ trans;

				//cout << std:: endl << "Points: " << std::endl;
				for (auto point = (*rbTemp->getMassPoints()).begin(); point != (*rbTemp->getMassPoints()).end(); point++) {
					//get the projection coordinates of a point
					//XMStoreFloat3(&temp, XMVector3Rotate(XMLoadFloat3(&point->position), XMLoadFloat4(&rbs[i]->getRotationQuaternion())));
					//temp = addVector(rbs[i]->getPosition(), temp);
					//XMStoreFloat3(&temp, XMVector3Transform(XMLoadFloat3(&point->position), XMMatrixInverse(nullptr, rotation)));
					XMStoreFloat3(&temp, XMVector3Transform(XMLoadFloat3(&temp), 
						modelViewProjection));
					//cout << temp.x << "\t" << temp.y << "\t" << temp.z << std::endl;
					if (distanceMousePoint > sqrt(static_cast<double>(pow(temp.x - xPosProj, 2.f) + pow(temp.y - yPosProj, 2.f)))) {
						distanceMousePoint = sqrt(static_cast<double>(pow(temp.x - xPosProj, 2.f) + pow(temp.y - yPosProj, 2.f)));
						closest = point._Ptr;
					}
				}
			}

			closest->force = vfForce;
			/*XMStoreFloat3(&temp, XMVector3Transform(XMLoadFloat3(&closest->position), 
					g_camera.GetProjMatrix() *
					g_camera.GetViewMatrix()));
			cout << std:: endl << "Closest: " << "\t" << temp.x << "\t" << temp.y << std::endl;
			cout << std:: endl << "Mouse: " << "\t" << xPos << "\t" << yPos << std::endl;
			cout << std:: endl << "Width:" << "\t" << g_windowWidth << "\tHeight:\t" << g_windowHeight << std::endl;
			cout << std:: endl << "Mouse [-1; 1]: " << "\t" << ((static_cast<float>(xPos)/g_windowWidth) * 2.f - 1.f) << "\t" << (((static_cast<double>(yPos)/g_windowHeight) * 2.f - 1.f) * -1.f) << std::endl;*/
}

//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
                       bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
                       int xPos, int yPos, void* pUserContext )
{
	//static bool bLeftWasDown = false;
	// Track mouse movement if left mouse key is pressed
	static int xPosSave = 0, yPosSave = 0;

	switch (g_iTestCase)
	{
	case 4:
	case 5:
		{
		if (bLeftButtonDown)
		{
			//cout << std::endl << "case 5" << std::endl;
			rigidBody* rbs[] = {rb};
			//cout << std::endl << sizeof(rbs) / sizeof(*rbs) << std::endl;
			applyForceByMouseDrag(xPos, yPos, xPosSave, yPosSave, *rbs, 1, 0.1f);
			// Accumulate deltas in g_viMouseDelta
			//g_viMouseDelta.x += xPos - xPosSave;
			//g_viMouseDelta.y += yPos - yPosSave;
		

			//xPosSave = xPos;
			//yPosSave = yPos;

			//// Calcuate camera directions in world space
			//XMMATRIX viewInv = XMMatrixInverse(nullptr, g_camera.GetViewMatrix());
			//XMVECTOR camRightWorld = XMVector3TransformNormal(g_XMIdentityR0, viewInv);
			//XMVECTOR camUpWorld = XMVector3TransformNormal(g_XMIdentityR1, viewInv);

			//// Add accumulated mouse deltas to force
			//XMFLOAT3 vfForce(0.f, 0.f, 0.f);
			//XMVECTOR vForce = XMLoadFloat3(&vfForce);

			//float forceScale = .1f;
			//vForce = XMVectorAdd(vForce, forceScale * (float)g_viMouseDelta.x * camRightWorld);
			//vForce = XMVectorAdd(vForce, -forceScale * (float)g_viMouseDelta.y * camUpWorld);

			//XMStoreFloat3(&vfForce, vForce);

			//// Reset accumulated mouse deltas
			//g_viMouseDelta = XMINT2(0, 0);
		
			////calculate the closest point to the mouse
			//MassPoint* closest;
			//XMFLOAT3 temp(0.f, 0.f, 0.f);
			//float xPosProj = ((static_cast<float>(xPos)/g_windowWidth) * 2.f - 1.f);
			//float yPosProj = (((static_cast<float>(yPos)/g_windowHeight) * 2.f - 1.f) * -1.f);
			//float distanceMousePoint = D3D11_FLOAT32_MAX;

			///*XMMATRIX scale    = XMMatrixScaling(rb->getScale().x, rb->getScale().y, rb->getScale().z);
			//XMMATRIX trans    = XMMatrixTranslation(rb->getPosition().x,rb->getPosition().y,rb->getPosition().z);
			//XMMATRIX rotation = XMMatrixRotationQuaternion(XMLoadFloat4(&rb->getRotationQuaternion()));*/

			////cout << std:: endl << "Points: " << std::endl;
			//for (auto point = (*rb->getMassPoints()).begin(); point != (*rb->getMassPoints()).end(); point++) {
			//	//get the projection coordinates of a point
			//	XMStoreFloat3(&temp, XMVector3Rotate(XMLoadFloat3(&point->position), XMLoadFloat4(&rb->getRotationQuaternion())));
			//	temp = addVector(rb->getPosition(), temp);
			//	XMStoreFloat3(&temp, XMVector3Transform(XMLoadFloat3(&temp), 
			//		g_camera.GetProjMatrix() *
			//		g_camera.GetViewMatrix()));
			//	//cout << temp.x << "\t" << temp.y << "\t" << temp.z << std::endl;
			//	if (distanceMousePoint > sqrt(static_cast<double>(pow(2.f, temp.x - xPosProj) + pow(2.f, temp.y - yPosProj)))) {
			//		distanceMousePoint = sqrt(static_cast<double>(pow(2.f, temp.x - xPosProj) + pow(2.f, temp.y - yPosProj)));
			//		closest = point._Ptr;
			//	}
			//}

			//closest->force = vfForce;
			/*XMStoreFloat3(&temp, XMVector3Transform(XMLoadFloat3(&closest->position), 
					g_camera.GetProjMatrix() *
					g_camera.GetViewMatrix()));
			cout << std:: endl << "Closest: " << "\t" << temp.x << "\t" << temp.y << std::endl;
			cout << std:: endl << "Mouse: " << "\t" << xPos << "\t" << yPos << std::endl;
			cout << std:: endl << "Width:" << "\t" << g_windowWidth << "\tHeight:\t" << g_windowHeight << std::endl;
			cout << std:: endl << "Mouse [-1; 1]: " << "\t" << ((static_cast<float>(xPos)/g_windowWidth) * 2.f - 1.f) << "\t" << (((static_cast<double>(yPos)/g_windowHeight) * 2.f - 1.f) * -1.f) << std::endl;*/
			}
		break;
		}
	case 6:
		break;
	case 7:
	{
		if (bLeftButtonDown) {
			//cout << std::endl << "case 6" << std::endl;
			//cout << std::endl << sizeof(rbs) / sizeof(*rbs) << std::endl;
			applyForceByMouseDrag(xPos, yPos, xPosSave, yPosSave, rigidBodies->data(), rigidBodies->size(), 0.5f);
		}
		break;
	}
	default:
		break;
	}
   
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Send message to AntTweakbar first
    if (TwEventWin(hWnd, uMsg, wParam, lParam))
    {
        *pbNoFurtherProcessing = true;
        return 0;
    }

    // If message not processed yet, send to camera
	if(g_camera.HandleMessages(hWnd,uMsg,wParam,lParam))
    {
        *pbNoFurtherProcessing = true;
		return 0;
    }

	return 0;
}

void static colideWithBOundaryCube(float* vecs[2][3], float boundaryCubeHalfSide, float friction, float bounciness) {
	bool outOfBoundary = false;
	short whereToBounce;
	
	for (int i = 0; i < 3; i++) {
		outOfBoundary = false;

		if (*vecs[0][i] - g_fSphereSize < -1 * boundaryCubeHalfSide) {
			whereToBounce = -1;
			outOfBoundary = true;
		} else if (*vecs[0][i] + g_fSphereSize > boundaryCubeHalfSide) {
			whereToBounce = 1;
			outOfBoundary = true;
		}

		if (outOfBoundary) {
			*vecs[0][i] = boundaryCubeHalfSide * whereToBounce + g_fSphereSize;
			*vecs[1][i] = -*vecs[1][i] * bounciness; // [1] - velocity vector
			for (int j = 0; j < 3; j++) {
				if (j != i) {
					*vecs[1][j] = *vecs[1][j] * friction;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------
// Handle updates to the scene
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double dTime, float fElapsedTime, void* pUserContext)
{
	UpdateWindowTitle(L"Demo");

	// Move camera
	g_camera.FrameMove(fElapsedTime);

	// Update effects with new view + proj transformations
	g_pEffectPositionColor->SetView(g_camera.GetViewMatrix());
	g_pEffectPositionColor->SetProjection(g_camera.GetProjMatrix());

	g_pEffectPositionNormal->SetView(g_camera.GetViewMatrix());
	g_pEffectPositionNormal->SetProjection(g_camera.GetProjMatrix());

	g_pEffectPositionNormalColor->SetView(g_camera.GetViewMatrix());
	g_pEffectPositionNormalColor->SetProjection(g_camera.GetProjMatrix());

#ifdef TEMPLATE_DEMO

	if (g_iPreTestCase != g_iTestCase){// test case changed
		// clear old setup and build up new setup
		TwDeleteBar(g_pTweakBar);
		g_pTweakBar = nullptr;
		InitTweakBar(g_pPd3Device);
		switch (g_iTestCase)
		{
		case 0:
			cout << "Demo 1!\n";
			g_fSphereSize = 0.05f;
					//EULER
			deltaTime = 0.1;
			SpringPoint* a;
			Spring* b;
			g_demoCase = 0;
			g_integrationMethod = 0;
			ResetMassSprings(0.1f);
			for(auto spring = springs.begin(); spring != springs.end(); spring++)
				{
					b= &((Spring)*spring);
					b->computeElasticForces();
				}
			for(auto point = points.begin(); point != points.end();point++)
				{

					a =  ((SpringPoint*)*point);
					a->IntegratePosition(deltaTime);
					a->computeAcceleration();
					a->IntegrateVelocity(deltaTime);
					a->resetForces();
				}	
			for(auto spring = springs.begin(); spring != springs.end(); spring++)
					{
						b= &((Spring)*spring);
						std::cout << "\nEuler demo1 after one time step:\n";
						b->printSpring();
					}

			//Midpoint
			g_integrationMethod =1;
			ResetMassSprings(0.1);
			for(auto spring = springs.begin(); spring != springs.end();spring++)
				{
					b= &(((Spring)*spring));
					b->computeElasticForces();
				}
			for(auto point = points.begin(); point != points.end();point++)
			{	
				a =  (((SpringPoint*)*point));
				a->gp_posTemp = a->IntegratePositionTmp(deltaTime/2.0f);
				a->computeAcceleration();
				a->gp_velTemp = a->IntegrateVelocityTmp(deltaTime/2.0f);
				a->IntegratePosition(deltaTime, a->gp_velTemp);
				a->resetForces();
			}
			for(auto spring = springs.begin(); spring != springs.end();spring++)
			{
				b= &(((Spring)*spring));
				b->computeElasticForcesTmp();
			}
			for(auto point = points.begin(); point != points.end();point++)
			{
				a =  (((SpringPoint*)*point));
				a->IntegrateVelocity(deltaTime);
				a->resetForces();
			}
			for(auto spring = springs.begin(); spring != springs.end(); spring++)
				{
					b= &((Spring)*spring);
					std::cout << "\nMidpoint demo1 after one time step:\n";
					b->printSpring();
				}

			break;
		case 1:
			cout << "Demo2!\n";
			g_fSphereSize = 0.05f;
			deltaTime = 0.005f;
			ResetMassSprings(0.005f);
			break;
		case 2:
			cout << "Demo3!\n";
			g_fSphereSize = 0.05f;
			deltaTime = 0.005f;
			ResetMassSprings(0.005f);
			break;
		case 3:
			cout << "Demo4\n";
			g_integrationMethod = 0;
			g_demoCase = 1;
			g_fSphereSize = 0.05f;
			g_gravity = -9.81f;
			g_useDamping = true;
			g_useGravity = true;
			g_usingWalls = false;
			g_ceiling =1;
			g_xWall =1;
			g_zWall =1;
			currentTime = timeGetTime();
			g_bDrawMassSpringSystem = true;
			//here we might want to reset the points/springs
			ResetMassSprings(deltaTime);
			break;
		case 4:
		{
			cout << "Rigid Body Simulation (Demo 1)!" << std::endl;
			ResetRigidBodies();
			deltaTime = 2.0f;
			g_bDrawRigidBodySimulation = true;

			rb->integrateValues(deltaTime);
			
			cout << "\nlinear velocity: " << std::endl;
			cout << "rb_vel: " << rb->getVelocity().x << ", " << rb->getVelocity().y << ", " << rb->getVelocity().z << std::endl;
			cout << "\nangular velocity: " << std::endl;
			cout << "rb_angVel: " << rb->getAngularVelocity().x << ", " << rb->getAngularVelocity().y << ", " << rb->getAngularVelocity().z << std::endl;
			
			cout << "\nworld space velocity of (-0.3, -0.5, -0.25): " << std::endl;
			MassPoint* tmpMP = new MassPoint();
			*tmpMP = pointList->at(5);
			cout << "wsvel: " << tmpMP->velocity.x << ", " << tmpMP->velocity.y << ", " << tmpMP->velocity.z << std::endl;

			break;
		}
		case 5:
		{
			cout << "Rigid Body Simulation (Demo 2)!" << std::endl;
			ResetRigidBodies();
			//cout << "rb_pos: " << rb->r_position.x << ", " << rb->r_position.y << ", " << rb->r_position.z << std::endl;
			deltaTime = 0.01f;
			g_bDrawRigidBodySimulation = true;
			
			/*for (auto massPoint = *rb->getMassPoints().begin(); massPoint != rb->getMassPoints().end(); massPoint++) {
				if (massPoint->force.x != 0.f || massPoint->force.y != 0.f || massPoint->force.z != 0.f) {
					forceStart = XMFLOAT3(rb->getPosition().x + massPoint->position.x, rb->getPosition().y + massPoint->position.y, rb->getPosition().z + massPoint->position.z);
					forceEnd = XMFLOAT3(rb->getPosition().x + massPoint->position.x + massPoint->force.x, rb->getPosition().y + massPoint->position.y + massPoint->force.y, rb->getPosition().z + massPoint->position.z + massPoint->force.z);;
				}
			}*/

			rb->integrateValues(deltaTime);
			//cout << "rb_pos: " << rb->r_position.x << ", " << rb->r_position.y << ", " << rb->r_position.z << std::endl;
			break;
		}
		case 6:
		{
			cout << "Rigid Body Collision (Demo 3)!" << std::endl;
			ResetRigidBodies();
			//cout << "rb_pos: " << rb->r_position.x << ", " << rb->r_position.y << ", " << rb->r_position.z << std::endl;
			deltaTime = 0.005f;
			g_bDrawRigidBodyCollision = true;
			rb1->integrateValues(deltaTime);
			rb2->integrateValues(deltaTime);
			//cout << "rb_pos: " << rb->r_position.x << ", " << rb->r_position.y << ", " << rb->r_position.z << std::endl;
			break;
		}
		case 7:
		{
			cout << "Rigid Body Simulation (Demo 4)!" << std::endl;
			
			g_bDrawRigidBodySimulation = true;
			ResetRigidBodies();
			g_gravity = -1;
			break;
		}
		case 8:
			delete(fluid);
			fluid = new Fluid(XMFLOAT3(0.f, .0f, 0.f), XMINT3(2, 2, 2), 7, .03f, 1.f, 200.f, .01f);
			break;
		default:
			cout << "Empty Test!\n";
			break;
		}
		g_iPreTestCase = g_iTestCase;
	}
	if (g_bSimulateByStep && DXUTIsKeyDown(VK_SPACE)){
		g_bIsSpaceReleased = false;
	}
	if (g_bSimulateByStep && !g_bIsSpaceReleased)
		if (DXUTIsKeyDown(VK_SPACE))
			return;
	if (g_bSimulateByStep && g_bIsSpaceReleased)
		return;
	// update current setup for each frame
	SpringPoint* a;
	Spring* b;
	switch (g_iTestCase)
	{// handling different cases
	case 0:
		break;
	case 1:
		deltaTime = 0.005f;
		for(auto spring = springs.begin(); spring != springs.end(); spring++)
			{
				b= &((Spring)*spring);
				b->computeElasticForces();
			}
			for(auto point = points.begin(); point != points.end();point++)
			{

				a =  ((SpringPoint*)*point);
				a->IntegratePosition(deltaTime);
				a->computeAcceleration();
				a->IntegrateVelocity(deltaTime);
				a->resetForces();
			}	
		break;
	case 2:
			for(auto spring = springs.begin(); spring != springs.end();spring++)
			{
				b= &(((Spring)*spring));
				b->computeElasticForces();
			}
			for(auto point = points.begin(); point != points.end();point++)
			{	
				a =  (((SpringPoint*)*point));
				a->gp_posTemp = a->IntegratePositionTmp(deltaTime/2.0f);
				a->computeAcceleration();
				a->gp_velTemp = a->IntegrateVelocityTmp(deltaTime/2.0f);
				a->IntegratePosition(deltaTime, a->gp_velTemp);
				a->resetForces();
			}
			for(auto spring = springs.begin(); spring != springs.end();spring++)
			{
				b= &(((Spring)*spring));
				b->computeElasticForcesTmp();
			}
			for(auto point = points.begin(); point != points.end();point++)
			{
				a =  (((SpringPoint*)*point));
				a->IntegrateVelocity(deltaTime);
				a->resetForces();
			}
		break;
	case 3:
		previousTime = currentTime;
		currentTime = timeGetTime();
		deltaTime = (currentTime-previousTime)/1000.0f;
		if(g_fixedTimestep) {
			deltaTime = g_manualTimestep;
		}

		if(g_preIntegrationMethod != g_integrationMethod || g_preDemoCase != g_demoCase) {
			ResetMassSprings(deltaTime);
			g_preIntegrationMethod = g_integrationMethod;
			g_preDemoCase = g_demoCase;
		}
		switch (g_integrationMethod)
		{
		case 0: //EULER
			for(auto spring = springs.begin(); spring != springs.end(); spring++)
			{
				b= &((Spring)*spring);
				b->computeElasticForces();
			}
			for(auto point = points.begin(); point != points.end();point++)
			{

				a =  ((SpringPoint*)*point);
				if(g_useGravity) { a->addGravity(g_gravity); }
				if(g_useDamping) {a->addDamping(deltaTime); }
				a->IntegratePosition(deltaTime);
				a->computeAcceleration();
				a->IntegrateVelocity(deltaTime);
				a->resetForces();
				if(g_usingWalls)
					a->computeCollisionWithWalls(deltaTime,g_fSphereSize,g_xWall,g_zWall,g_ceiling);
				else
					a->computeCollision(deltaTime, g_fSphereSize);
			}	
			
			if(g_firstStep == true && g_demoCase == 0)
			{
				for(auto spring = springs.begin(); spring != springs.end(); spring++)
				{
					b= &((Spring)*spring);
					std::cout << "\nEuler demo1 after one time step:\n";
					b->printSpring();
				}
				g_firstStep = false;
			}
			break;
		case 1: //MIDPOINT
			for(auto spring = springs.begin(); spring != springs.end();spring++)
			{
				b= &(((Spring)*spring));
				b->computeElasticForces();
			}
			for(auto point = points.begin(); point != points.end();point++)
			{	
				a =  (((SpringPoint*)*point));
				if(g_useGravity) { a->addGravity(g_gravity); }
				a->gp_posTemp = a->IntegratePositionTmp(deltaTime/2.0f);
				a->computeAcceleration();
				a->gp_velTemp = a->IntegrateVelocityTmp(deltaTime/2.0f);
				if(g_useDamping) {a->addDamping(deltaTime); }
				a->IntegratePosition(deltaTime, a->gp_velTemp);
				a->resetForces();
			}
			for(auto spring = springs.begin(); spring != springs.end();spring++)
			{
				b= &(((Spring)*spring));
				b->computeElasticForcesTmp();
			}
			for(auto point = points.begin(); point != points.end();point++)
			{
				a =  (((SpringPoint*)*point));
				a->IntegrateVelocity(deltaTime);
				a->resetForces();				
				if(g_usingWalls)
					a->computeCollisionWithWalls(deltaTime,g_fSphereSize,g_xWall,g_zWall,g_ceiling);
				else
					a->computeCollision(deltaTime, g_fSphereSize);
			}

			if(g_firstStep == true && g_demoCase == 0)
			{
				for(auto spring = springs.begin(); spring != springs.end(); spring++)
				{
					b= &((Spring)*spring);
					std::cout << "\nMidpoint demo1 after one time step:\n";
					b->printSpring();
				}
				g_firstStep = false;
			}
			break;
		case 2: //LEAP FROG
			for(auto spring = springs.begin(); spring != springs.end();spring++)
			{
				b= &(((Spring)*spring));
				b->computeElasticForces();
			}
			for(auto point = points.begin(); point != points.end();point++)
			{
				a =  (((SpringPoint*)*point));
				if(g_useGravity) { a->addGravity(g_gravity); }
				a->computeAcceleration();
				a->IntegrateVelocity(deltaTime);
				if(g_useDamping) {a->addDamping(deltaTime); }
				a->IntegratePosition(deltaTime);
				a->resetForces();
				if(g_usingWalls)
					a->computeCollisionWithWalls(deltaTime,g_fSphereSize,g_xWall,g_zWall,g_ceiling);
				else
					a->computeCollision(deltaTime, g_fSphereSize);
			}	
			break;
		default:
			break;
		}

		// REALLY SIMPLE COLLISION DETECTION WITH GROUND PLANE
		/*
		for (auto point = points.begin(); point != points.end();point++)
		{
			float friction = 0.98; // 1.0 means NO friction
			float bounciness = 0.75;

			a =  (((SpringPoint*)*point));
			
			float* vecs[2][3] = {
				{&a->gp_position.x, &a->gp_position.y, &a->gp_position.z}, // old position
				{&a->gp_velocity.x, &a->gp_velocity.y, &a->gp_velocity.z} // velocity
			};

			colideWithBOundaryCube(vecs, 1, friction, bounciness);

			/*if (a->gp_position.y < -1+g_fSphereSize ) {
				a->setPosition(XMFLOAT3 (a->gp_position.x, -0.9999+g_fSphereSize, a->gp_position.z)); 
				a->setVelocity(XMFLOAT3 (a->gp_velocity.x*friction, -a->gp_velocity.y*bounciness, a->gp_velocity.z*friction)); 	
			}
		}
		*/



		break;
	case 4:
		rb->integrateValues(.01f);
		//cout << "rb_pos: " << rb->r_position.x << ", " << rb->r_position.y << ", " << rb->r_position.z << std::endl;
		break;
	case 5:
		rb->integrateValues(.01f);
		break;
	case 6:
		rb1->integrateValues(deltaTime);
		rb2->integrateValues(deltaTime);

		//CHECK COLLISIONS 
		mat1 = getObj2WorldMat(rb1); 
		mat2 = getObj2WorldMat(rb2);

		// Check if a corner of mat2 is in mat1
		simpletest = checkCollision(mat1, mat2);// should find out a collision here
		if (!simpletest.isValid){
			// Check if a corner of mat1 is in mat2
			simpletest = checkCollision(mat2, mat1);
			simpletest.normalWorld = -simpletest.normalWorld;// we compute the impulse to A
		}
		if (!simpletest.isValid)
		{
			//std::printf("No Collision\n");
		}
		else{
			std::printf("collision detected at normal: %f, %f, %f\n",XMVectorGetX(simpletest.normalWorld), XMVectorGetY(simpletest.normalWorld), XMVectorGetZ(simpletest.normalWorld));
			std::printf("collision point : %f, %f, %f\n",XMVectorGetX(simpletest.collisionPointWorld), XMVectorGetY(simpletest.collisionPointWorld), XMVectorGetZ(simpletest.collisionPointWorld));
			//angular velocities to linear velocity at the collision point

			XMFLOAT3 collisionPoint;// ,collisionNormal;
			XMStoreFloat3(&collisionPoint,simpletest.collisionPointWorld); 
			//XMStoreFloat3(&collisionNormal,simpletest.normalWorld); 
			contact = Contact(collisionPoint,/*collisionNormal*/simpletest.normalWorld, rb1, rb2);
			contact.calcRelativeVelocity();
		}

		//cout << "rb_pos: " << rb->r_position.x << ", " << rb->r_position.y << ", " << rb->r_position.z << std::endl;
		break;
	case 7:
		previousTime = currentTime;
		currentTime = timeGetTime();
		deltaTime = (currentTime-previousTime)/1000.0f;
		if(g_fixedTimestep) {
			deltaTime = g_manualTimestep;
		}
		for(auto rb = rigidBodies->begin(); rb != rigidBodies->end();rb++)
		{
			rb->integrateValues(deltaTime);
			if(g_useGravity)
				rb->addGravity(deltaTime, g_gravity);
			if(g_useDamping)
				rb->addDamping(deltaTime, g_damping_linear, g_damping_angular);
		}
		rigidBody* first;
		rigidBody* second;
		for(auto one = rigidBodies->begin(); one != rigidBodies->end();one++)
		{
			first = &(*one);
			for(auto two = one+1; two != rigidBodies->end();two++)
			{
				second = &(*two);

					mat1 = getObj2WorldMat(first);
					mat2 = getObj2WorldMat(second);
					simpletest = checkCollision(mat1, mat2);
					if (!simpletest.isValid){ // Check if a corner of mat1 is in mat2
						simpletest = checkCollision(mat2, mat1);
						simpletest.normalWorld = -simpletest.normalWorld;// we compute the impulse to A
					}
					if (!simpletest.isValid)
					{
					}
					else{
						XMFLOAT3 collisionPoint;// ,collisionNormal;
						XMStoreFloat3(&collisionPoint,simpletest.collisionPointWorld); 
						contact = Contact(collisionPoint,simpletest.normalWorld, first, second);
						contact.calcRelativeVelocity();
			
					}

			}
		}
		//collision with floor
		second = floorRB;
		for(auto one = rigidBodies->begin(); one != rigidBodies->end();one++)
		{				
			first = &*one;
			mat1 = getObj2WorldMat(first);
			mat2 = getObj2WorldMat(second);
			simpletest = checkCollision(mat1, mat2);
			if (!simpletest.isValid){ // Check if a corner of mat1 is in mat2
				simpletest = checkCollision(mat2, mat1);
				simpletest.normalWorld = -simpletest.normalWorld;// we compute the impulse to A
			}
			if (!simpletest.isValid)
			{			
				//std::printf("No Collision\n");
			}
			else
			{
				XMFLOAT3 collisionPoint;// ,collisionNormal;
				XMStoreFloat3(&collisionPoint,simpletest.collisionPointWorld); 
				//XMStoreFloat3(&collisionNormal,simpletest.normalWorld); 
				contact = Contact(collisionPoint,/*collisionNormal*/simpletest.normalWorld, first, second);
				contact.calcRelativeVelocity();
			}
		}
		break;
	case 8:
		FluidSimulation::integrateFluid(*fluid, .001f);
		break;
	default:
		break;
	}
	
	if (g_bSimulateByStep)
		g_bIsSpaceReleased = true;


	
#endif
#ifdef MASS_SPRING_SYSTEM
	//TODO: Calculate Euler/Midpoint here?
#endif
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
                                  double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

	// Clear render target and depth stencil
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    // Draw floor
    DrawFloor(pd3dImmediateContext);

    // Draw axis box
    DrawBoundingBox(pd3dImmediateContext);

#ifdef TEMPLATE_DEMO
	switch (g_iTestCase)
	{
	case 0:
		if (g_bDrawMassSpringSystem) DrawMassSpringSystem(pd3dImmediateContext);
		break;
	case 1:
		if (g_bDrawMassSpringSystem) DrawMassSpringSystem(pd3dImmediateContext);
		break;
	case 2:
		if (g_bDrawMassSpringSystem) DrawMassSpringSystem(pd3dImmediateContext);
		break;
#ifdef MASS_SPRING_SYSTEM
	case 3:
		if (g_bDrawMassSpringSystem) DrawMassSpringSystem(pd3dImmediateContext);
		break;
#endif
#ifdef RIGID_BODY_SIMULATION
	case 4:
	case 5:
		if (g_bDrawRigidBodySimulation) DrawCube(rb);
		break;
#endif
#ifdef RIGID_BODY_COLLISION
	case 6:
		if (g_bDrawRigidBodyCollision) {

		//	std::cout << "drawing rb collision\n";
		//	std::cout << rb1->getPosition().x << "," << rb1->getPosition().y << "," << rb1->getPosition().z << "\n";

			/*std::cout << "drawing rb collision\n";
			std::cout << rb1->getPosition().x << "," << rb1->getPosition().y << "," << rb1->getPosition().z << "\n";*/

			DrawCollisionCubes(rb1);
			DrawCollisionCubes(rb2);
		}
		break;
#endif
	case 7:
		for(auto rb = rigidBodies->begin(); rb != rigidBodies->end();rb++)
		{
			DrawCollisionCubes(&((rigidBody)*rb));
		}
	//	DrawCube(floorRB);
		break;
	case 8:
		{
			std::vector<Particle>* particles = fluid->getParticles();
			for (auto particle = particles->begin(); particle != particles->end(); particle++) {
				DrawParticle(static_cast<Particle>(*particle), fluid->getKernelSize() / 3);
			}
			break;
		}
	default:
		break;
	}
#endif
    
#ifdef MASS_SPRING_SYSTEM
	// Draw mass spring system
#endif

    // Draw GUI
    TwDraw();

    if (g_pFFmpegVideoRecorder) 
    {
        V(g_pFFmpegVideoRecorder->AddFrame(pd3dImmediateContext, DXUTGetD3D11RenderTargetView()));
    }
}

//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
#if defined(DEBUG) | defined(_DEBUG)
	// Enable run-time memory check for debug builds.
	// (on program exit, memory leaks are printed to Visual Studio's Output console)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

#ifdef _DEBUG
	std::wcout << L"---- DEBUG BUILD ----\n\n";
#endif

	// Set general DXUT callbacks
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackMouse( OnMouse, true );
	DXUTSetCallbackKeyboard( OnKeyboard );

	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

	// Set the D3D11 DXUT callbacks
	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    // Init camera
 	XMFLOAT3 eye(0.0f, 0.0f, -2.0f);
	XMFLOAT3 lookAt(0.0f, 0.0f, 0.0f);
	g_camera.SetViewParams(XMLoadFloat3(&eye), XMLoadFloat3(&lookAt));
    g_camera.SetButtonMasks(MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_RIGHT_BUTTON);

    // Init DXUT and create device
	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	//DXUTSetIsInGammaCorrectMode( false ); // true by default (SRGB backbuffer), disable to force a RGB backbuffer
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	DXUTCreateWindow( L"Demo" );
	DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, 1280, 960 );
    
	DXUTMainLoop(); // Enter into the DXUT render loop

	DXUTShutdown(); // Shuts down DXUT (includes calls to OnD3D11ReleasingSwapChain() and OnD3D11DestroyDevice())
	
	return DXUTGetExitCode();
}
