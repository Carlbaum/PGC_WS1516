#include "Contact.h"
#include <DirectXMath.h>
using namespace DirectX;

#include "vectorOperations.h"
#include "rigidBody.h"

XMFLOAT3 c_position;
XMVECTOR c_normal;
float depth;
rigidBody* body1, * body2;
float impulse;


void Contact::calcRelativeVelocity() {
			XMFLOAT3 cross;
			XMStoreFloat3(&cross, XMVector3Cross(XMLoadFloat3(&body1->getAngularVelocity()), XMLoadFloat3(&subVector(c_position,body1->getPosition()))));
			XMFLOAT3 v1 = addVector(body1->getVelocity(), cross);
			XMStoreFloat3(&cross, XMVector3Cross(XMLoadFloat3(&body2->getAngularVelocity()), XMLoadFloat3(&subVector(c_position,body2->getPosition()))));
			XMFLOAT3 v2 = addVector(body2->getVelocity(), cross);
			//v_relative_dot;
			v_relative = subVector(v1,v2);
			XMStoreFloat(&v_relative_dot, XMVector3Dot(c_normal,XMLoadFloat3(&v_relative)));

			if(v_relative_dot > 0 ) { //separating
			
			}
			else if ( v_relative_dot < 0) { //colliding
				calculateImpulse();
			}
			else { //sliding
			}
}


void Contact::calculateImpulse() {
	float c = 0.5f; //this should determine if the body is elastic or plastic.. for now i'll leave it as plastic!

	float ma = body1->getMassInverse(), mb = body2->getMassInverse();

	float numerator = -(1+c)*v_relative_dot;

	XMVECTOR tempVec_a, tempVec_b, center_1, center_2;
	center_1 = XMLoadFloat3(&(subVector(c_position,body1->getPosition())));
	center_2 = XMLoadFloat3(&(subVector(c_position,body2->getPosition())));
	tempVec_a = XMVector3Transform(XMVector3Cross(XMVector3Cross(center_1,c_normal),center_1),body1->getInertiaTensorInverse());
	tempVec_b = XMVector3Transform(XMVector3Cross(XMVector3Cross(center_2,c_normal),center_2),body1->getInertiaTensorInverse()); //TODO ?!?! is it really supposed to be 'body1' in this row??
	tempVec_a = XMVector3Dot(tempVec_a+tempVec_b,c_normal);

	float temp;
	XMStoreFloat(&temp, tempVec_a);
	float denominator = ma + mb + temp;

	impulse = numerator / denominator;

	XMFLOAT3 newVelocity_1, newVelocity_2, newAngMom_1, newAngMom_2,tempf3;
	c_normal *= impulse; //scale the normal with the impulse
	
	XMStoreFloat3(&tempf3, c_normal*ma);
	newVelocity_1 = addVector(body1->getAngularVelocity(),tempf3);

	XMStoreFloat3(&tempf3, c_normal*mb);
	newVelocity_2 = subVector(body2->getAngularVelocity(),tempf3);

	XMStoreFloat3(&tempf3, XMVector3Cross(center_1,c_normal));
	newAngMom_1 = addVector(body1->getAngularMomentum(),tempf3);

	XMStoreFloat3(&tempf3, XMVector3Cross(center_2,c_normal));
	newAngMom_2 = subVector(body2->getAngularMomentum(),tempf3);

	if(!body1->isStatic)
	{
		body1->setLinearVelocity(newVelocity_1);
		body1->setAngularMomentum(newAngMom_1);
	}
	if(!body2->isStatic)
	{
		body2->setLinearVelocity(newVelocity_2);
		body2->setAngularMomentum(newAngMom_2);
	}

}
	
	Contact::Contact(XMFLOAT3 pos, XMVECTOR norm, rigidBody* rb1,rigidBody* rb2)
{
	c_position = pos;
	c_normal = norm;
	//depth = d;
	body1 = rb1;
	body2 = rb2;
}

Contact::Contact(void)
{
}


Contact::~Contact(void)
{
}