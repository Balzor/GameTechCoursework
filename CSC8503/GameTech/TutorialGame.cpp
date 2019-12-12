#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include <algorithm>
//#include "Camera.h"
#include <thread>
#include <chrono>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream> 

#include "../CSC8503Common/PositionConstraint.h"
#include "../CSC8503Common/NavigationGrid.h"

using namespace NCL;
using namespace CSC8503;
TutorialGame::TutorialGame()	{
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	useGravity = true;
	beHard = false;
	inSelectionMode = true;

	
	physics->UseGravity(useGravity);
	Debug::SetRenderer(renderer);

	InitialiseAssets();
}
bool inStart = true;
/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("CenteredGoose.msh"	 , &gooseMesh);
	loadFunc("CharacterA.msh", &keeperMesh);
	loadFunc("CharacterM.msh", &charA);
	loadFunc("CharacterF.msh", &charB);
	loadFunc("Apple.msh"	 , &appleMesh);
	loadFunc("raptor.msh"	 , &raptorMesh);
	loadFunc("hellknight.msh"	 , &hellkeeperMesh);


	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	gooseTex = (OGLTexture*)TextureLoader::LoadAPITexture("goose.jpg");
	brickTex = (OGLTexture*)TextureLoader::LoadAPITexture("brick.png");
	dogeTex = (OGLTexture*)TextureLoader::LoadAPITexture("doge.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();

	InitWorld();
}


TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete gooseMesh;
	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}
bool again=false;
bool started = false;
string line;
int check;
void TutorialGame::UpdateGame(float dt) {
	
	if (started) {
		if (endTimer < 1) {
			std::ifstream readScore("highscore.txt");
			if (readScore.is_open()) {
				while (std::getline(readScore, line)) {
					std::stringstream s(line);
					s >> check;
				}
				readScore.close();
			}
			if (check < (applesPicked + (itemsPicked * 2) + (killCounter * 3) - caught)) {
				std::ofstream writeScore("highscore.txt");
				if (writeScore.is_open()) {
					writeScore << std::to_string(applesPicked + (itemsPicked * 2) + (killCounter * 3) - caught) << std::endl;
					check = applesPicked + (itemsPicked * 2) + (killCounter * 3) - caught;
					writeScore.close();
				}
			}
			renderer->DrawString("TIME UP", Vector2(180, 180));
			renderer->DrawString("Final Score: " + std::to_string(applesPicked +(itemsPicked*2) + (killCounter * 3) - caught), Vector2(500, 180));
			renderer->DrawString("HighScore: " + std::to_string(check), Vector2(500, 200));
			renderer->DrawString("Press F1 to play again", Vector2(180, 160));
			renderer->DrawString("Press ESC to exit game", Vector2(180, 140));
			
			
			
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
				inStart = true;
				endTimer = 10800;
				//again = true;
			}
			dt = 0;

		}
		else {
			endTimer -= 1;

			if (stamina < 100) {
				stamina += 0.05;
			}
			else {
				stamina = 100;
			}

			renderer->DrawString("Time left: " + std::to_string(endTimer), Vector2(10, 160));
			renderer->DrawString("Humans killed: " + std::to_string(killCounter), Vector2(10, 60));
			renderer->DrawString("Apples Picked: " + std::to_string(applesPicked), Vector2(10, 80));
			renderer->DrawString("Items Picked: " + std::to_string(itemsPicked), Vector2(10, 100));
			renderer->DrawString("Score: " + std::to_string(applesPicked+(itemsPicked*2) + (killCounter * 3) - caught), Vector2(10, 120));
			renderer->DrawString("Stamina: " + std::to_string(stamina), Vector2(10, 140));
			renderer->DrawString("Times Caught: " + std::to_string(caught), Vector2(10, 180), Vector4(1, 0, 0, 1));
			if (useGravity) {
				//Debug::Print("(G)ravity on", Vector2(10, 40));
			}
			else {
				//Debug::Print("(G)ravity off", Vector2(10, 40));
			}
			if (beHard) {
				Debug::Print("Press H for easy mode", Vector2(20, 20));
			}
			else {
				Debug::Print("Press H for hard mode", Vector2(20, 20));
			}
		}
		if (!inSelectionMode) {
			world->GetMainCamera()->UpdateCamera(dt);
			LockedCameraMovement();
		}
		else {
			world->GetMainCamera()->UpdateCamera(dt);
		}

		for (GameObject* i : characters) {
			i->Pathfind(i, i->GetTransform().GetLocalPosition(), lockedObject, lockedObject->GetTransform().GetLocalPosition(), beHard);
			if (i->GetTransform().GetLocalPosition().y <= -30) {
				killCounter++;
				i->GetTransform().SetLocalPosition(i->GetInitPos());
				//AddHellKeeperToWorld(Vector3(50, 2, 0));
			}
			
		}

		UpdateKeys();
		CreateObjects();


		if (lockedObject->GetTransform().GetLocalPosition().y < -25) {
			//InitWorld();
			lockedObject->GetTransform().SetLocalPosition(lockedObject->GetInitPos());
			//Respawn();
		}
	}
	else {
		renderer->DrawString("Single Player", Vector2(50, 450));
	}
	SelectObject();
	MoveSelectedObject();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);
	
	Debug::FlushRenderables();
	renderer->Render();
}
void TutorialGame::CreateObjects() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::C)) {
		AddSphereToWorld(world->GetMainCamera()->GetPosition(), 10.0f, 10.0f);

	}
}
void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::H)) {
		beHard = !beHard;

	}
	
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		inStart = true;
		throw 20;
		//InitWorld(); //We can reset the simulation at any time with F1
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		//InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (!inSelectionMode) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

int po = 1;
int po2 = 0;
int countPo = 0;
Vector3 byebye = Vector3(0, -100, 0);
void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();
	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!
	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!
	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 20, 0), rightAxis);
	if (stunned) {
		stunnedCounter++;
		renderer->DrawString("stunned", Vector2(200, 200), Vector4(1, 1, 1, 1));
		if (stunnedCounter == 150) {
			stunned = false;
			stunnedCounter = 0;
		}
	}
	if (lockedObject->GetTag() == "hold") {
		for (GameObject* check : physics->GetPickupList()) {
			if (check->GetName() == "character") {
				
				caught++;
				pickupItems->GetTransform().SetParent(saveParent);
				pickupItems->GetTransform().SetLocalPosition(pickupItems->GetInitPos());
				pickupItems->GetPhysicsObject()->SetInverseMass(1);
				pickupItems->SetBoundingVolume((CollisionVolume*)appleV);
				lockedObject->SetTag("");

			}
			if (check->GetName() == "keeper") {
				lockedObject->GetPhysicsObject()->AddForceAtPosition(Vector3(0,5000,0),lockedObject->GetTransform().GetLocalPosition());
				//std::cout << check->GetInitPos() << std::endl;
				caught++;
				pickupItems->GetTransform().SetParent(saveParent);
				pickupItems->GetTransform().SetLocalPosition(pickupItems->GetInitPos());
				pickupItems->GetPhysicsObject()->SetInverseMass(1);
				pickupItems->SetBoundingVolume((CollisionVolume*)appleV);
				lockedObject->SetTag("");
				stunned = true;
			}
		}
	}
	if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::RIGHT)) {
		for (GameObject* obj : physics->GetTriggerList()) {
			obj->GetPhysicsObject()->AddForce(fwdAxis*10);
			obj->GetPhysicsObject()->AddTorque(Vector3(0,100,0));
		}
	}
	if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
		for (GameObject* obj : physics->GetPickupList()) {
			if (obj->GetName() != "goose") {
				obj->GetPhysicsObject()->AddForce(fwdAxis * 100);
				obj->GetPhysicsObject()->AddTorque(fwdAxis*100);
			}
		}
	}
	
		
	
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		if (lockedObject->GetTag() != "hold") {
			for (GameObject* obj : physics->GetPickupList()) {
				if (obj->GetName() == "apple" || obj->GetName() == "pickable") {
					saveParent = obj->GetTransform().GetParent();
					lockedObject->SetTag("hold");
					obj->GetTransform().SetParent(&lockedObject->GetTransform());
					obj->GetTransform().SetLocalPosition(Vector3(-1, 1, 2));
					obj->GetPhysicsObject()->SetInverseMass(0);
					obj->SetBoundingVolume(0);					
					
					pickupItems = obj;
				}
			}
		}
		else {
			std::cout << "release what u hold" << std::endl;
		}
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R)) {
		if (lockedObject->GetTag() != "hold") {
			std::cout << "u hold shit" << std::endl;
		}
		else {
			lockedObject->SetTag("");
			for (GameObject* obj : physics->GetPickupList()) {
				if (obj->GetName() == "base") {
					std::cout << "u on base now" << std::endl;
					if (pickupItems->GetName() == "apple") {
						applesPicked++;
					}else {
						itemsPicked++;
					}
					pickupItems->GetTransform().SetParent(nullptr);
					pickupItems->GetTransform().SetLocalPosition(byebye);
					pickupItems->GetPhysicsObject()->SetInverseMass(0);
					pickupItems->SetBoundingVolume((CollisionVolume*)appleV);
					stamina = 100;
					//character->GetTransform().SetLocalPosition(character->GetInitPos());
					for (GameObject* i : characters) {
						i->GetTransform().SetLocalPosition(i->GetInitPos());
					}
				}
				else {
					pickupItems->GetTransform().SetParent(saveParent);
					pickupItems->GetTransform().SetLocalPosition(pickupItems->GetInitPos());
					pickupItems->GetPhysicsObject()->SetInverseMass(1);
					pickupItems->SetBoundingVolume((CollisionVolume*)appleV);
				}
			}
		}
	}
	if (!stunned) {
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {//LEFT
			lockedObject->GetPhysicsObject()->AddForce(-rightAxis * 100);
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {//RIGHT
			lockedObject->GetPhysicsObject()->AddForce(rightAxis * 100);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {//FRWD
			for (GameObject* obj : physics->GetPickupList()) {
				if (obj->GetName() == "water") {
					//std::cout << "u on water now" << std::endl;
					lockedObject->GetPhysicsObject()->AddForce(fwdAxis * 0.1);

				}
			}
			if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SHIFT)) {
				if (stamina > 0) {
					lockedObject->GetPhysicsObject()->AddForce(fwdAxis * 100);
					stamina -= 0.5;
				}
				else {
					lockedObject->GetPhysicsObject()->AddForce(fwdAxis * 3);
				}
			}
			else {
				lockedObject->GetPhysicsObject()->AddForce(fwdAxis * 5);
			}
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {//BWRD
			if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SHIFT)) {
				if (stamina > 0) {
					lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * 10);
					stamina -= 0.5;
				}
				else {
					lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * 3);
				}
			}
			else {
				lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * 5);
			}
		}
		//fly
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE)) {
			lockedObject->GetPhysicsObject()->AddForce(Vector3(0, 2000, 0));
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::V)) {
			lockedObject->GetPhysicsObject()->AddForce(Vector3(0, -1000, 0));
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM7)) {
			endTimer = 10;
		}
	}
	
	if (pickupItems != nullptr) {
		for (GameObject* obj : physics->GetPickupList()) {
			if (obj->GetName() == "water") {
				if (lockedObject->GetTag() == "hold") {
					pickupItems->GetTransform().SetParent(saveParent);
					pickupItems->GetTransform().SetLocalPosition(pickupItems->GetInitPos());
					pickupItems->GetPhysicsObject()->SetInverseMass(1);
					pickupItems->SetBoundingVolume((CollisionVolume*)appleV);
					lockedObject->SetTag("");
				}
			}
		}
	}
	
	
	
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM0)) {
		std::cout << lockedObject->GetTransform().GetLocalPosition();
		std::cout << " tag:"<<lockedObject->GetTag();
	}
	
	
	trigger->GetTransform().SetParent(&lockedObject->GetTransform());
	trigger->GetTransform().SetLocalPosition(Vector3(0, 2, 25));
	
	picker->GetTransform().SetParent(&lockedObject->GetTransform());
	picker->GetTransform().SetLocalPosition(Vector3(0, 0, 0));
}
bool d3 = false;

void  TutorialGame::LockedCameraMovement() {
	float pitchLocked = 0;
	float yawLocked = 0;
	
	if (lockedObject != nullptr) {

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
			//d3 = !d3;
		}
		if (d3) {
			pitchLocked -= (Window::GetMouse()->GetRelativePosition().y);
		}
		else {
			pitchLocked = 0;
		}
		yawLocked -= (Window::GetMouse()->GetRelativePosition().x);

		Quaternion objOri = lockedObject->GetTransform().GetWorldOrientation();
		Vector3 objPos = lockedObject->GetTransform().GetWorldPosition();

		//pitchLocked = min(pitchLocked, 90.0f);
		//pitchLocked = max(pitchLocked, -90.0f);

		Quaternion q = Quaternion::EulerAnglesToQuaternion(0, yawLocked, 0);

		Quaternion n = objOri * q;
		lockedObject->GetTransform().SetLocalOrientation(n);

		Vector3 npos = n *  Vector3(0, 20, -40);
		Vector3 camPos = objPos + npos;
		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));
		
		Matrix4 modelMat = temp.Inverse();

		Quaternion q1(modelMat);
		Vector3 angles = q1.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);	
	}
}
void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}
/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool mouse = false;
bool TutorialGame::SelectObject() {
	if (inSelectionMode) {
		Window::GetWindow()->ShowOSPointer(true);
		Window::GetWindow()->LockMouseToWindow(false);
		//started = false;
	}
	else {
		if (!mouse) {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
		else {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM6)) {
		mouse = !mouse;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM6)) {
		inSelectionMode = !inSelectionMode;
	}
	if (inStart) {
		started = false;
		int yawLockedMenu = 0;
		if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::LEFT)) {
			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				GameObject* selection = (GameObject*)closestCollision.node;
				std::cout << selection->GetName() << std::endl;
				if (selection->GetName() == "cube") {
					std::cout << "ready" << std::endl;
					started = true;
					inStart = false;
					inSelectionMode = false;
					
				}
				return true;
			}
			else {
				return false;
			}
		}
		yawLockedMenu -= (Window::GetMouse()->GetRelativePosition().x);

		Quaternion menuOri = menu->GetTransform().GetWorldOrientation();
		Vector3 menuPos = menu->GetTransform().GetWorldPosition();


		//pitchLocked = min(pitchLocked, 90.0f);
		//pitchLocked = max(pitchLocked, -90.0f);

		Quaternion q = Quaternion::EulerAnglesToQuaternion(0, 0, 0);
		Quaternion m = menuOri * q;
		menu->GetTransform().SetLocalOrientation(m);

		Vector3 mpos = m * Vector3(0, 0, -30);
		Vector3 camMenu = menuPos + mpos;
		Matrix4 tempMenu = Matrix4::BuildViewMatrix(camMenu, menuPos, Vector3(0, 1, 0));

		Matrix4 modelMatMenu = tempMenu.Inverse();

		Quaternion q2(modelMatMenu);
		Vector3 anglesMenu = q2.ToEuler(); //nearly there now!

		
		world->GetMainCamera()->SetPosition(camMenu);
		world->GetMainCamera()->SetPitch(anglesMenu.x);
		world->GetMainCamera()->SetYaw(anglesMenu.y);
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	
	//renderer -> DrawString(" Click Force :" + std::to_string(forceMagnitude),Vector2(10, 20)); // Draw debug text at 10 ,20
	//forceMagnitude += Window::GetMouse() -> GetWheelMovement() * 100.0f;
	//
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::MIDDLE)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			GameObject* selection = (GameObject*)closestCollision.node;
			std::cout << selection->GetName() << std::endl;
			selectionObject = selection;
			
			
			return;
		}
		else {
			return;
		}
	}
	if (!selectionObject) {
		return;// we haven ’t selected anything 
	}
	else {
		int x = selectionObject->GetTransform().GetLocalPosition().x;
		int y = selectionObject->GetTransform().GetLocalPosition().y;
		int z = selectionObject->GetTransform().GetLocalPosition().z;
		int ox = selectionObject->GetTransform().GetLocalOrientation().x;
		int oy = selectionObject->GetTransform().GetLocalOrientation().y;
		int oz = selectionObject->GetTransform().GetLocalOrientation().z;
		renderer->DrawString("Name: " + selectionObject->GetName(), Vector2(0, 520));
		renderer->DrawString("Location: " + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) , Vector2(0, 500));
		renderer->DrawString("State: " + selectionObject->GetTag() , Vector2(0, 460));
	}
	// Push the selected object !
	
	if (Window::GetMouse() -> ButtonPressed(NCL::MouseButtons::RIGHT)) {
		//Ray ray = CollisionDetection::BuildRayFromMouse(* world -> GetMainCamera());
		//RayCollision closestCollision;
		//if (world -> Raycast(ray, closestCollision, true)) {
		//	/*if (closestCollision.node == selectionObject) {
		//		selectionObject -> GetPhysicsObject() -> AddForce(ray.GetDirection() * forceMagnitude);
		//	}*/
		//	if (closestCollision.node == selectionObject) {
		//		 selectionObject -> GetPhysicsObject() -> AddForceAtPosition(ray.GetDirection() * 100, closestCollision.collidedAt);
		//	}
		//}
		selectionObject = nullptr;
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.5f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	inStart = true;
	mouse = false;
	killCounter = 0;
	applesPicked = 0;
	itemsPicked = 0;
	stamina = 100;
	caught = 0;
	characters.clear();
	world->ClearAndErase();
	physics->Clear();

	//InitMixedGridWorld(10, 10, 3.5f, 3.5f);
	AddGooseToWorld(Vector3(-112, 0, -238));
	AddAppleToWorld(Vector3(35, 2, 0));
	AddAppleToWorld(Vector3(-105, -13.9, -200));

	//AddHellKeeperToWorld(Vector3(50, 2, 0));
	AddParkKeeperToWorld(Vector3(40, 2, 0));
	AddCharacterToWorld(Vector3(152.607, -16.0649, -44.5025));
	AddCharacterToWorld(Vector3(152.607 + 10, -16.0649, -44.5025));
	AddCharacterToWorld(Vector3(291, -16.0649, 4));
	AddCharacterToWorld(Vector3(201, -16.0649, -69));


	AddIslandToWorld(Vector3(200, -19, 200));
	AddFloorToWorld(Vector3(0, -19, 0), Vector3(100, 2, 100));
	AddMiniFloorToWorld(Vector3(0, -19, 0), Vector3(20, 0.1, 50), Vector4(0.25, 0.47, 0.30, 1));

	AddTreeToWorld(Vector3(0, -18, 0));
	AddTreeToWorld(Vector3(0, -17, 250));
	AddTreeToWorld(Vector3(20, -15, 258));
	//AddTreeToWorld(Vector3(0, -18, 0));

	AddTrampolineToWorld(Vector3(50, -17, 50), Vector3(4, 0.5, 4), Vector4(0.55, 0.27, 0.07, 1), 20.0f);

	AddMenuToWorld(Vector3(0, 200, 0), Vector3(22, 14, 3));

	BridgeConstraintTest();
	BridgeConstraintDoor();
	
}


//From here on it's functions to add in objects to the world!

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, const Vector3& size) {
	GameObject* floor = new GameObject("floor");

	Vector3 floorSize = size;
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(floorSize);
	floor->GetTransform().SetWorldPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume(),.0f));
	floor->GetRenderObject()->SetColour(Vector4(0.87, 0.59, .12, 1));
	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}
GameObject* TutorialGame::AddIslandToWorld(const Vector3& position) {
	
	
	AddMiniFloorToWorld(position + Vector3(0, 1, 0), Vector3(100, 1, 100), Vector4(0.25, 0.47, 0.30, 1));
	
	AddMiniFloorToWorld(position + Vector3(0, 1, -200), Vector3(100, 1, 100), Vector4(0.25, 0.47, 0.30, 1));

	AddMiniFloorToWorld(Vector3(0, -18, 200), Vector3(100, 1, 100), Vector4(0.25, 0.47, 0.30, 1));

	//water
	AddWaterFloorToWorld(Vector3(-150, -19, 200), Vector3(50, 0.3, 100), Vector4(0.00, 0.41, 0.58, .5));
	AddWaterFloor2ToWorld(Vector3(-150, -20, 200), Vector3(50, 0.3, 100), Vector4(0.00, 0.41, 0.58, .5));

	AddWaterFloorToWorld(Vector3(-150, -19, 0), Vector3(50, 0.3, 100), Vector4(0.00, 0.41, 0.58, .5));
	AddWaterFloor2ToWorld(Vector3(-150, -20, 0), Vector3(50, 0.3, 100), Vector4(0.00, 0.41, 0.58, .5));

	AddWaterFloorToWorld(Vector3(250, -19, -200), Vector3(50, 0.3,100 ), Vector4(0.00, 0.41, 0.58, .5));
	AddWaterFloor2ToWorld(Vector3(250, -20, -200), Vector3(50, 0.3, 100), Vector4(0.00, 0.41, 0.58, .5));
	
	AddWaterFloorToWorld(Vector3(350, -19, -200), Vector3(50, 0.3,100 ), Vector4(0.00, 0.41, 0.58, .5));
	AddWaterFloor2ToWorld(Vector3(350, -20, -200), Vector3(50, 0.3, 100), Vector4(0.00, 0.41, 0.58, .5));
	
	AddWaterFloorToWorld(Vector3(350, -19, 0), Vector3(50, 0.3,100 ), Vector4(0.00, 0.41, 0.58, .5));
	AddWaterFloor2ToWorld(Vector3(350, -20, 0), Vector3(50, 0.3, 100), Vector4(0.00, 0.41, 0.58, .5));
	
	AddWaterFloorToWorld(Vector3(350, -19, 200), Vector3(50, 0.3,100 ), Vector4(0.00, 0.41, 0.58, .5));
	AddWaterFloor2ToWorld(Vector3(350, -20, 200), Vector3(50, 0.3, 100), Vector4(0.00, 0.41, 0.58, .5));
	
	AddWaterFloorToWorld(Vector3(300, -19, 350), Vector3(100, 0.3, 50 ), Vector4(0.00, 0.41, 0.58, .5));
	AddWaterFloor2ToWorld(Vector3(300, -20, 350), Vector3(100, 0.3, 50), Vector4(0.00, 0.41, 0.58, .5));
	
	AddWaterFloorToWorld(Vector3(100, -19, 350), Vector3(100, 0.3, 50 ), Vector4(0.00, 0.41, 0.58, .5));
	AddWaterFloor2ToWorld(Vector3(100, -20, 350), Vector3(100, 0.3, 50), Vector4(0.00, 0.41, 0.58, .5));
	
	AddWaterFloorToWorld(Vector3(-100, -19, 350), Vector3(100, 0.3, 50 ), Vector4(0.00, 0.41, 0.58, .5));
	AddWaterFloor2ToWorld(Vector3(-100, -20, 350), Vector3(100, 0.3, 50), Vector4(0.00, 0.41, 0.58, .5));

	AddWaterFloorToWorld(position + Vector3(-100, 0, -400), Vector3(100, 0.3, 100), Vector4(0.00, 0.41, 0.58, .5));
	AddWaterFloor2ToWorld(position + Vector3(-100, -1, -400), Vector3(100, 0.3, 100), Vector4(0.00, 0.41, 0.58, .5));

	AddWaterFloorToWorld(position + Vector3(-300, 0, -400), Vector3(100, 0.3, 100), Vector4(0.00, 0.41, 0.58, .5));
	AddWaterFloor2ToWorld(position + Vector3(-300, -1, -400), Vector3(100, 0.3, 100), Vector4(0.00, 0.41, 0.58, .5));
	
	AddMiniFloorToWorld(position + Vector3(-300, 1, -400), Vector3(50, 3, 50), Vector4(0.40, 0.27, 0.0, 1)); //brown on den
	AddMiniFloorToWorld(position + Vector3(-300, 4, -400), Vector3(50, 0.1, 50), Vector4(0.25, 0.47, 0.30, 1));//green on den

	for (double i = 0; i < 39; i += 1) {//fraxtis
		AddMiniFloorToWorld(position + Vector3(90-i*4, 5, -290), Vector3(1, 3, 1), Vector4(0.85, 0.77, 0.53, 1));
	}
	AddMiniFloorToWorld(position + Vector3(14, 4, -290), Vector3(76, 1, 0.5), Vector4(0.85, 0.77, 0.53, 1));

	for (double i = 0; i < 23; i += 1) {//fraxtis
		AddMiniFloorToWorld(position + Vector3(90, 5, -290+i*4), Vector3(1, 3, 1), Vector4(0.85, 0.77, 0.53, 1));
	}
	for (double i = 0; i < 39; i += 1) {//fraxtis
		AddMiniFloorToWorld(position + Vector3(90-i*4, 5, -190), Vector3(1, 3, 1), Vector4(0.85, 0.77, 0.53, 1));
	}
	AddMiniFloorToWorld(position + Vector3(14, 4, -190), Vector3(76, 1, 0.5), Vector4(0.85, 0.77, 0.53, 1));

	AddMiniFloorToWorld(position + Vector3(30, 10, -230), Vector3(20, 20, 20), Vector4(0.25, 0.25, 0.25, 1));//house
	
	AddMiniFloorToWorld(Vector3(200, -9, 100), Vector3(10, 10, 40), Vector4(0.25, 0.25, 0.25, 1));//house
	AddMiniFloorToWorld(Vector3(190, -12, 120), Vector3(1, 6, 4), Vector4(0.75, 0.75, 0.75, 1));//door
	
	//rgb(0.29, 0.59, 0.04)
	AddMiniFloorToWorld(Vector3(240, -11, 100), Vector3(5, 7, 80), Vector4(0.29, 0.59, 0.04, 1));//bush
	AddMiniFloorToWorld(Vector3(190, -11, 175), Vector3(45, 7, 5), Vector4(0.29, 0.59, 0.04, 1));//bush
	AddMiniFloorToWorld(Vector3(195, -11, 25), Vector3(40, 7, 5), Vector4(0.29, 0.59, 0.04, 1));//bush
	AddMiniFloorToWorld(Vector3(140, -11, 100), Vector3(5, 7, 80), Vector4(0.29, 0.59, 0.04, 1));//bush

	AddCharacterToWorld(Vector3(185, -10, 125));


	for (double i = 0; i < 7; i += 1) {//celing
		AddMiniFloorToWorld(position + Vector3(30, 31+i*2, -230), Vector3(22-i*2, 2, 22-i*2), Vector4(1, 1, 1, 1));	
		
	}
	//door
	AddMiniFloorToWorld(Vector3(208.987, -10, -24.2353), Vector3(1, 8, 4), Vector4(0.55, 0.27, 0.07, 1));
	//trampoline
	AddTrampolineToWorld(Vector3(237, -15.2873, -55), Vector3(4, 0.5, 4), Vector4(0.55, 0.27, 0.07, 1),20.0f);
	//apple
	AddAppleToWorld(Vector3(223.925, 28, -34.5038));
	AddTreeToWorld(position + Vector3(-340, 4, -440));
	//home base
	AddBaseFloorToWorld(Vector3(-112.015, -13.9, -238.057), Vector3(10, 1, 10), Vector4(0.55, 0.27, 0.07, 1));
	
	AddAppleToWorld(Vector3(0, 20, 0));
	AddAppleToWorld(Vector3(137, 20, 77));
	AddAppleToWorld(Vector3(137, 20, 177));
	AddAppleToWorld(Vector3(137, 20, 26));
	AddAppleToWorld(Vector3(240, 20, 175));
	AddAppleToWorld(Vector3(240, 20, 26));

	AddPickablesToWorld(Vector3(10, 0, 10), Vector3(1, 1, 1), Vector4(0, 1, 0, 1));
	AddPickablesToWorld(Vector3(240, 0, -76), Vector3(1, 1, 1), Vector4(0, 1, 0, 1));
	AddPickablesToWorld(Vector3(238, 0, -55), Vector3(1, 1, 1), Vector4(0, 1, 0, 1));
	AddPickablesToWorld(Vector3(220, 25, -20), Vector3(1, 1, 1), Vector4(0, 1, 0, 1));
	AddPickablesToWorld(Vector3(220, 25, 120), Vector3(1, 1, 1), Vector4(0, 1, 0, 1));
	AddPickablesToWorld(Vector3(201, 25, 126), Vector3(1, 1, 1), Vector4(0, 1, 0, 1));
	AddPickablesToWorld(Vector3(202, 25, 91), Vector3(1, 1, 1), Vector4(0, 1, 0, 1));
	
	return 0;
}
GameObject* TutorialGame::AddWaterFloorToWorld(const Vector3& position, const Vector3& size, const Vector4& colour) {
	GameObject* water = new GameObject("water");

	Vector3 floorSize = size;
	AABBVolume* volume = new AABBVolume(floorSize);
	//water->SetBoundingVolume((CollisionVolume*)volume);
	water->GetTransform().SetWorldScale(floorSize);
	water->GetTransform().SetWorldPosition(position);

	water->SetRenderObject(new RenderObject(&water->GetTransform(), cubeMesh, nullptr, basicShader));
	water->SetPhysicsObject(new PhysicsObject(&water->GetTransform(), water->GetBoundingVolume(), .0f));
	water->GetRenderObject()->SetColour(colour);
	water->GetPhysicsObject()->SetInverseMass(0);
	water->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(water);

	return water;
}
GameObject* TutorialGame::AddWaterFloor2ToWorld(const Vector3& position, const Vector3& size, const Vector4& colour) {
	GameObject* water = new GameObject("water");

	Vector3 floorSize = size;
	AABBVolume* volume = new AABBVolume(floorSize);
	water->SetBoundingVolume((CollisionVolume*)volume);
	water->GetTransform().SetWorldScale(floorSize);
	water->GetTransform().SetWorldPosition(position);

	water->SetRenderObject(new RenderObject(&water->GetTransform(), cubeMesh, nullptr, basicShader));
	water->SetPhysicsObject(new PhysicsObject(&water->GetTransform(), water->GetBoundingVolume(), .0f));
	water->GetRenderObject()->SetColour(colour);
	water->GetPhysicsObject()->SetInverseMass(0);
	water->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(water);

	return water;
}
GameObject* TutorialGame::AddBaseFloorToWorld(const Vector3& position, const Vector3& size, const Vector4& colour) {
	GameObject* base = new GameObject("base");

	Vector3 floorSize = size;
	AABBVolume* volume = new AABBVolume(floorSize);
	base->SetBoundingVolume((CollisionVolume*)volume);
	base->GetTransform().SetWorldScale(floorSize);
	base->GetTransform().SetWorldPosition(position);

	base->SetRenderObject(new RenderObject(&base->GetTransform(), cubeMesh, basicTex, basicShader));
	base->SetPhysicsObject(new PhysicsObject(&base->GetTransform(), base->GetBoundingVolume(), .0f));
	base->GetRenderObject()->SetColour(colour);
	base->GetPhysicsObject()->SetInverseMass(0);
	base->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(base);

	return base;
}
GameObject* TutorialGame::AddMiniFloorToWorld(const Vector3& position, const Vector3& size, const Vector4& colour) {
	GameObject* minifloor = new GameObject("minifloor");

	Vector3 floorSize = size;
	AABBVolume* volume = new AABBVolume(floorSize);
	minifloor->SetBoundingVolume((CollisionVolume*)volume);
	minifloor->GetTransform().SetWorldScale(floorSize);
	minifloor->GetTransform().SetWorldPosition(position);

	minifloor->SetRenderObject(new RenderObject(&minifloor->GetTransform(), cubeMesh, basicTex, basicShader));
	minifloor->SetPhysicsObject(new PhysicsObject(&minifloor->GetTransform(), minifloor->GetBoundingVolume(),.0f));
	minifloor->GetRenderObject()->SetColour(colour);
	minifloor->GetPhysicsObject()->SetInverseMass(0);
	minifloor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(minifloor);

	return minifloor;
}

GameObject* TutorialGame::AddTrampolineToWorld(const Vector3& position, const Vector3& size, const Vector4& colour, float bounciness) {
	GameObject* trampoline = new GameObject("trampoline");

	Vector3 trampolineSize = size;
	//Vector3(4, 0.5, 4)
	AABBVolume* volume = new AABBVolume(trampolineSize);
	trampoline->SetBoundingVolume((CollisionVolume*)volume);
	trampoline->GetTransform().SetWorldScale(trampolineSize);
	trampoline->GetTransform().SetWorldPosition(position);

	trampoline->SetRenderObject(new RenderObject(&trampoline->GetTransform(), cubeMesh, basicTex, basicShader));
	trampoline->SetPhysicsObject(new PhysicsObject(&trampoline->GetTransform(), trampoline->GetBoundingVolume(), bounciness));
	trampoline->GetRenderObject()->SetColour(colour);
	trampoline->GetPhysicsObject()->SetInverseMass(0);
	trampoline->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(trampoline);

	return trampoline;
}
GameObject* TutorialGame::AddTreeToWorld(const Vector3& position) {
	GameObject* treeLog = new GameObject("tree");

	Vector3 treeSize = Vector3(3, 6, 3);
	AABBVolume* volume = new AABBVolume(treeSize);
	treeLog->SetBoundingVolume((CollisionVolume*)volume);
	treeLog->GetTransform().SetWorldScale(treeSize);
	treeLog->GetTransform().SetWorldPosition(position);

	treeLog->SetRenderObject(new RenderObject(&treeLog->GetTransform(), cubeMesh, basicTex, basicShader));
	treeLog->SetPhysicsObject(new PhysicsObject(&treeLog->GetTransform(), treeLog->GetBoundingVolume(),0.0f));
	treeLog->GetRenderObject()->SetColour(Vector4(0.83, 0.53, .10, 1));
	treeLog->GetPhysicsObject()->SetInverseMass(0);
	treeLog->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(treeLog);
	AddTreeLeafToWorld(position+Vector3(0,14,0));
	return treeLog;
}
GameObject* TutorialGame::AddTreeLeafToWorld(const Vector3& position) {
	GameObject* treeLeaf = new GameObject("treeLeaf");

	Vector3 leafSize = Vector3(8, 8, 8);
	AABBVolume* volume = new AABBVolume(leafSize);
	treeLeaf->SetBoundingVolume((CollisionVolume*)volume);
	treeLeaf->GetTransform().SetWorldScale(leafSize);
	treeLeaf->GetTransform().SetWorldPosition(position);

	treeLeaf->SetRenderObject(new RenderObject(&treeLeaf->GetTransform(), cubeMesh, basicTex, basicShader));
	treeLeaf->SetPhysicsObject(new PhysicsObject(&treeLeaf->GetTransform(), treeLeaf->GetBoundingVolume(),0.0f));
	treeLeaf->GetRenderObject()->SetColour(Vector4(0.74, 0.76, .31, 1));
	treeLeaf->GetPhysicsObject()->SetInverseMass(0);
	treeLeaf->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(treeLeaf);
	return treeLeaf;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject("sphere");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetWorldScale(sphereSize);
	sphere->GetTransform().SetWorldPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume(),0.7f));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject("cube");

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(dimensions);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume(),0.5f));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}
GameObject* TutorialGame::AddPickablesToWorld(const Vector3& position, Vector3 dimensions,Vector4& colour) {
	GameObject* pickable = new GameObject("pickable");

	AABBVolume* volume = new AABBVolume(dimensions);

	pickable->SetBoundingVolume((CollisionVolume*)volume);

	pickable->GetTransform().SetWorldPosition(position);
	pickable->GetTransform().SetWorldScale(dimensions);
	pickable->SetInitPos(position);
	pickable->SetRenderObject(new RenderObject(&pickable->GetTransform(), cubeMesh, basicTex, basicShader));
	pickable->SetPhysicsObject(new PhysicsObject(&pickable->GetTransform(), pickable->GetBoundingVolume(),0.1f));
	pickable->GetRenderObject()->SetColour(colour);
	pickable->GetPhysicsObject()->SetInverseMass(1);
	pickable->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(pickable);

	return pickable;
}
GameObject* TutorialGame::AddButtonToWorld(const Vector3& position, Vector3 dimensions,Vector3 colour,TextureBase* tex) {
	GameObject* cube = new GameObject("cube");

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(dimensions);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, tex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume(), 0.5f));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}
GameObject* TutorialGame::AddMenuToWorld(const Vector3& position, Vector3 dimensions) {
	AABBVolume* volume = new AABBVolume(dimensions);

	menu->SetBoundingVolume((CollisionVolume*)volume);

	menu->GetTransform().SetLocalPosition(position);
	menu->GetTransform().SetWorldScale(dimensions);

	menu->SetRenderObject(new RenderObject(&menu->GetTransform(), cubeMesh, gooseTex, basicShader));
	menu->SetPhysicsObject(new PhysicsObject(&menu->GetTransform(), menu->GetBoundingVolume(),0.5f));

	menu->GetPhysicsObject()->SetInverseMass(0);
	menu->GetPhysicsObject()->InitCubeInertia();

	//AddCubeToWorld(position+Vector3(13,4,-2), Vector3(7, 1, 1), 0);
	AddButtonToWorld(position+Vector3(13,4,-2), Vector3(7, 1, 1),Vector4(0,0,0,1),nullptr);

	world->AddGameObject(menu);

	return menu;
}
GameObject* TutorialGame::AddTriggerToWorld(const Vector3& position, Vector3 dimensions,string name) {
	GameObject* cube = new GameObject(name);

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(dimensions);

	//cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume(),0.0f));

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddGooseToWorld(const Vector3& position)
{	
	if (lockedObject) {
		delete lockedObject;
	}
	float size			= 1.0f;
	float inverseMass	= 1.0f;

	GameObject* goose = new GameObject("goose");


	SphereVolume* volume = new SphereVolume(size);
	goose->SetBoundingVolume((CollisionVolume*)volume);

	goose->GetTransform().SetWorldScale(Vector3(size,size,size) );
	goose->GetTransform().SetWorldPosition(position);

	goose->SetRenderObject(new RenderObject(&goose->GetTransform(), gooseMesh, nullptr, basicShader));
	goose->SetPhysicsObject(new PhysicsObject(&goose->GetTransform(), goose->GetBoundingVolume(),0.2f));

	goose->GetPhysicsObject()->SetInverseMass(inverseMass);
	goose->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(goose);

	lockedObject = goose;
	//selectionObject = goose;
	lockedObject->SetInitPos(position);
	trigger = AddTriggerToWorld(position, Vector3(5, 3, 20),"trigger");
	picker = AddTriggerToWorld(position, Vector3(3, 3, 3),"pickup");

	return goose;
}
void TutorialGame::Respawn() {
	
	AddGooseToWorld(Vector3(30, 2, 0));
}
//void TutorialGame::Chase(GameObject* chaser) {
//	//write here
//	for (GameObject* obj : physics->GetChaseList()) {
//		if (obj->GetName() == "goose") {
//			if (obj->GetTag() == "hold") {
//				//std::cout << "gonna chase a goose" << std::endl;
//				//character->GetTransform().SetLocalPosition(lockedObject->GetTransform().GetLocalPosition());
//				int whereX = chaser->GetTransform().GetLocalPosition().x - lockedObject->GetTransform().GetLocalPosition().x;
//
//				int whereY = chaser->GetTransform().GetLocalPosition().y;
//				int whereZ = chaser->GetTransform().GetLocalPosition().z - lockedObject->GetTransform().GetLocalPosition().z;
//
//				//chaser->GetTransform().SetLocalPosition(lockedObject->GetTransform().GetLocalPosition() + Vector3(whereX, 0, whereZ));
//
//				if (beHard) {
//					chaser->GetTransform().SetLocalPosition(lockedObject->GetTransform().GetLocalPosition());
//				}
//				else {
//					chaser->GetTransform().SetLocalPosition(lockedObject->GetTransform().GetLocalPosition() + Vector3(whereX, 0, whereZ));
//				}
//
//				
//			}
//			else {
//				//std::cout << "not gonna chase a goose" << std::endl;
//			}
//		}
//	}
//}
GameObject* TutorialGame::AddParkKeeperToWorld(const Vector3& position){
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	keeper = new GameObject("keeper");

	AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
	keeper->SetBoundingVolume((CollisionVolume*)volume);

	keeper->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	keeper->GetTransform().SetWorldPosition(position);

	keeper->SetInitPos(keeper->GetTransform().GetWorldPosition());

	keeper->SetRenderObject(new RenderObject(&keeper->GetTransform(), keeperMesh, nullptr, basicShader));
	keeper->SetPhysicsObject(new PhysicsObject(&keeper->GetTransform(), keeper->GetBoundingVolume(),0.2f));

	keeper->GetPhysicsObject()->SetInverseMass(inverseMass);
	keeper->GetPhysicsObject()->InitCubeInertia();



	world->AddGameObject(keeper);

	characters.push_back(keeper);

	return keeper;
}
GameObject* TutorialGame::AddHellKeeperToWorld(const Vector3& position){
	float meshSize = 10.f;
	float inverseMass = 0.5f;

	GameObject* hellkeeper = new GameObject("hell");

	AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
	hellkeeper->SetBoundingVolume((CollisionVolume*)volume);

	hellkeeper->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	hellkeeper->GetTransform().SetWorldPosition(position);

	hellkeeper->SetInitPos(hellkeeper->GetTransform().GetWorldPosition());

	hellkeeper->SetRenderObject(new RenderObject(&hellkeeper->GetTransform(), keeperMesh, dogeTex, basicShader));
	hellkeeper->SetPhysicsObject(new PhysicsObject(&hellkeeper->GetTransform(), hellkeeper->GetBoundingVolume(),0.2f));

	hellkeeper->GetPhysicsObject()->SetInverseMass(inverseMass);
	hellkeeper->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(hellkeeper);

	characters.push_back(hellkeeper);

	return hellkeeper;
}

GameObject* TutorialGame::AddCharacterToWorld(const Vector3& position) {
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	auto pos = keeperMesh->GetPositionData();

	Vector3 minVal = pos[0];
	Vector3 maxVal = pos[0];

	for (auto& i : pos) {
		maxVal.y = max(maxVal.y, i.y);
		minVal.y = min(minVal.y, i.y);
	}

	character = new GameObject("character");
	float r = rand() / (float)RAND_MAX;
	

	AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	character->GetTransform().SetWorldPosition(position);

	character->SetInitPos(character->GetTransform().GetWorldPosition());

	character->SetRenderObject(new RenderObject(&character->GetTransform(), r > 0.5f ? charA : charB, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume(),0.2f));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(character);

	//chaser = AddTriggerToWorld(position, Vector3(40, 40, 40), "chaser");

	characters.push_back(character);
	
	return character;
	
}

GameObject* TutorialGame::AddAppleToWorld(const Vector3& position) {
	GameObject* apple = new GameObject("apple");
	
	SphereVolume* volume = new SphereVolume(0.7f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetWorldScale(Vector3(4, 4, 4));
	apple->GetTransform().SetWorldPosition(position);

	apple->SetInitPos(apple->GetTransform().GetWorldPosition());

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), appleMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume(),0.3f));
	apple->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	

	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(5, 1, 3);

	float	invCubeMass = 1;
	int		numLinks	= 3;
	float	maxDistance	= 25;
	float	cubeDistance = 20;

	Vector3 startPos = Vector3(-79, -17, -145);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);

	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 40), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);

		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);

		world->AddConstraint(constraint);
		previous = block;
	}

	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);

}
void TutorialGame::BridgeConstraintDoor() {
	Vector3 cubeSize = Vector3(1, 5, 1);
	Vector3 cubeSize2 = Vector3(3, 5, 1);

	float	invCubeMass = 1;
	int		numLinks	= 1;
	float	maxDistance	= 20;

	Vector3 startPos = Vector3(145, -12, 31);

	//GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* start = AddMiniFloorToWorld(startPos, cubeSize, Vector4(0.40, 0.33, 0.00, 1));

	//GameObject* end = AddCubeToWorld(startPos + Vector3(11, 0, 0), cubeSize, 0);
	GameObject* end = AddMiniFloorToWorld(startPos + Vector3(11, 0, 0), cubeSize, Vector4(0.40, 0.33, 0.00, 1));

	GameObject* previous = start;

	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);

	for (int i = 0; i < numLinks; ++i) {
		//GameObject* block = AddCubeToWorld(startPos + Vector3( 0, 0, 0), cubeSize2, invCubeMass);
		GameObject* block = AddCubeToWorld(startPos + Vector3( 0, 0, 0), cubeSize2, invCubeMass);

		PositionConstraint* constraint = new PositionConstraint(previous, block, 5);

		world->AddConstraint(constraint);
		previous = block;
	}

	
#if 0

	if (!start)
		start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);

	if (!end)
		end = AddCubeToWorld(startPos + Vector3(20, 0, 0), cubeSize, 0);

	//GameObject* previous = start;
	//std::vector< PositionConstraint*> stack;
	//stack.reserve(numLinks);

	//for (int i = 0; i < numLinks; ++i) {
	if (!block)
		block = AddCubeToWorld(startPos + Vector3(1, 0, 0), cubeSize2, invCubeMass);



	//stack.emplace_back(constraint);
	//previous = block;
//}

	if (isStart) {
		PositionConstraint* constraint2 = new PositionConstraint(block, end, maxDistance);
		world->AddConstraint(constraint2);
	}

	if ((!isStart) || (slidingDoorConstraint)) {
		world->RemoveConstraint(slidingDoorConstraint);
		delete slidingDoorConstraint;
		slidingDoorConstraint = nullptr;
	}
	slidingDoorConstraint = new PositionConstraint(start, block, maxDistance);
	slidingDoorConstraint->debug = true;
	world->AddConstraint(slidingDoorConstraint);
#endif

}

void TutorialGame::SimpleGJKTest() {
	Vector3 dimensions		= Vector3(5, 5, 5);
	Vector3 floorDimensions = Vector3(100, 2, 100);

	GameObject* fallingCube = AddCubeToWorld(Vector3(0, 20, 0), dimensions, 10.0f);
	GameObject* newFloor	= AddCubeToWorld(Vector3(0, 0, 0), floorDimensions, 0.0f);

	delete fallingCube->GetBoundingVolume();
	delete newFloor->GetBoundingVolume();

	fallingCube->SetBoundingVolume((CollisionVolume*)new OBBVolume(dimensions));
	newFloor->SetBoundingVolume((CollisionVolume*)new OBBVolume(floorDimensions));

}

