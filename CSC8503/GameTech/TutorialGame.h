#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"


namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			bool beHard;

			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();
			void CreateObjects();

			void Chase(GameObject* chaser);

			void InitWorld();
			void Respawn();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void BridgeConstraintTest();
			void BridgeConstraintDoor();
			void SimpleGJKTest();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void LockedCameraMovement();


			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size);
			GameObject* AddMiniFloorToWorld(const Vector3& position, const Vector3& size,const Vector4& colour);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddMenuToWorld(const Vector3& position, Vector3 dimensions);
			GameObject* AddTriggerToWorld(const Vector3& position, Vector3 dimensions,string name);
			GameObject* AddButtonToWorld(const Vector3& position, Vector3 dimensions, Vector3 colour, TextureBase* tex);
			GameObject* trigger;
			GameObject* picker;
			GameObject* chaser;
			GameObject* chaser2;
			GameObject* pickupItems;

			GameObject* menu = new GameObject("menu");


			/*GameObject* start, * block, *end;
			Constraint* slidingDoorConstraint = nullptr;*/

			vector<GameObject*> characters;
			GameObject* keeper;
			GameObject* character;
			//items saved positions
			Transform* saveParent = nullptr;
			Vector3 save;
			SphereVolume* appleV = new SphereVolume(0.7f);
			//navgrid pathfinding
			//NavigationGrid outPath;
			//IT'S HAPPENING
			GameObject* AddGooseToWorld(const Vector3& position);
			GameObject* AddParkKeeperToWorld(const Vector3& position);
			GameObject* AddCharacterToWorld(const Vector3& position);
			GameObject* AddAppleToWorld(const Vector3& position);
			GameObject* AddTreeToWorld(const Vector3& position);
			GameObject* AddTreeLeafToWorld(const Vector3& position);
			GameObject* AddTrampolineToWorld(const Vector3& position, const Vector3& size, const Vector4& colour,float bounciness);
			GameObject* AddIslandToWorld(const Vector3& position);
			GameObject* AddBaseFloorToWorld(const Vector3& position, const Vector3& size, const Vector4& colour);
			GameObject* AddWaterFloorToWorld(const Vector3& position, const Vector3& size, const Vector4& colour);


			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLTexture* gooseTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	gooseMesh	= nullptr;
			OGLMesh*	keeperMesh	= nullptr;
			OGLMesh*	appleMesh	= nullptr;
			OGLMesh*	raptorMesh	= nullptr;
			OGLMesh*	charA		= nullptr;
			OGLMesh*	charB		= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			GameObject* ch1	= nullptr;
			GameObject* difficulty	= nullptr;
			int killCounter=0;
			int applesPicked = 0;
			float stamina = 100;
			int caught = 0;
			int endTimer=10800;
			Vector3 lockedOffset = Vector3(0, 14, 20);

			vector<Vector3> testNodes;

			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}
		};
	}
}

