#include <DirectXMath.h>
using namespace DirectX;

#include "point.h"

XMFLOAT3 gp_position;
XMFLOAT3 gp_velocity;
XMFLOAT3 gp_force;
float gp_mass = 10.0f;
float gp_damping;

void Point::setPosition(XMFLOAT3 newpos) {
	gp_position = newpos;
};
void Point::setVelocity(XMFLOAT3 newvel) {
	gp_velocity = newvel;
};
void Point::setForce(XMFLOAT3 newf) {
	gp_force = newf;
};

void Point::setMass(float newmass) {
	gp_mass = newmass;
};
void Point::setDamping(float newdamp) {
	gp_damping = newdamp;
};