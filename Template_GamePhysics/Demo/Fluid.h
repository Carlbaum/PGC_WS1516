#pragma once

#include <vector>
#include "Particle.h"
#include <DirectXMath.h>

using namespace DirectX;

class Fluid
{
	friend class FluidSimulation;
	friend class Grid;
protected:
	int exp;
	float kernelSize;
	float stiffness;
	float restDensity;
	float viscosity;
	float positioningStep;

	//num of particles on X, Y and Z axes
	XMINT3 numParticles;
	
	std::vector<Particle*> particles;
public:
	float getKernelSize();
	//get all particles
	std::vector<Particle*>& getParticles();
	virtual std::vector<Particle*>& getNeighbourParticles(Particle& particle);

	Fluid(XMFLOAT3 initialPostion, XMINT3 numParticles, int exp, float kernelSize, float positioningStep, float stiffness, float restDensity, float viscosity);
	virtual ~Fluid(void);
};

