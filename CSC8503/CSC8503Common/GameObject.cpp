#include "GameObject.h"
#include "CollisionDetection.h"
#include "NavigationGrid.h"
#include "Debug.h"
using namespace NCL::CSC8503;

GameObject::GameObject(string objectName)	{
	name			= objectName;
	isActive		= true;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	networkObject	= nullptr;

	grid = new NavigationGrid ("TestGrid1.txt");
}
GameObject::~GameObject()	{
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
	delete networkObject;
	delete grid;
}
float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}
void GameObject::Pathfind(GameObject* chaserObj,Vector3 chaserPos, GameObject* gooseObj,Vector3 goose,bool hard) {
	testNodes.clear();
	NavigationPath outPath;

	
	Vector3 startPos(chaserPos.x+90,0,chaserPos.z+90);
	Vector3 endPos(goose.x + 90, 0, goose.z + 90);
	
	bool found = grid->FindPath(startPos, endPos, outPath);
	Matrix4 vie2 = chaserObj->GetTransform().GetWorldMatrix().BuildViewMatrix(chaserPos, goose,Vector3(0,0,0));
	Matrix4 camWorld = vie2;
	Vector3 rightAxis = Vector3(camWorld.GetColumn(0));
	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 40, 0), rightAxis);
	
	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		pos.x = pos.x - 90;
		pos.y = -14;
		pos.z = pos.z - 90;
		testNodes.push_back(pos);
	}
	for (int i = 1; i < testNodes.size(); ++i) {
		float diff = chaserPos.z - goose.z;
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];

		Debug::DrawLine(a, b, Vector4(1, 0, 0, 1));

		chaserObj->GetTransform().SetLocalOrientation(gooseObj->GetTransform().GetLocalOrientation());
		
		
		if (gooseObj->GetTag() == "hold") {
			if (hard != true) {
				chaserObj->GetPhysicsObject()->AddForce(-fwdAxis);

				if (chaserPos.z > goose.z) {
					chaserObj->GetPhysicsObject()->AddForce(Vector3(0, 0, -50));
				}
				else {
					chaserObj->GetPhysicsObject()->AddForce(Vector3(0, 0, 50));
				}
			}
			else {
				chaserObj->GetTransform().SetLocalPosition(goose);
			}				
		}
	}
}
bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

//These would be better as a virtual 'ToAABB' type function, really...
void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(transform.GetWorldOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
}