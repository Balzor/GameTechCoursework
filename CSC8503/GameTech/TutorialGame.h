#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"


namespace NCL {
	namespace CSC8503 {
		class TestPacketReceiver : public PacketReceiver {
		public:
			TestPacketReceiver(string name) {
				this->name = name;
			}
			void ReceivePacket(int type, GamePacket* payload, int source) {
				if (type == String_Message) {
					StringPacket* realPacket = (StringPacket*)payload;
					size_t pos = 0;
					string msg = realPacket->GetStringFromData();
					string delimiter = ",";
					string token = msg.substr(0, msg.find(delimiter));
					vector<string> vec;
					std::cout << name << " received message : " << msg << std::endl;
					if (msg == "cc") {
						clientConnected = true;
					}

					if (token == "loc") {
						while ((pos = msg.find(delimiter)) != std::string::npos) {
							string token = msg.substr(0, pos);
							vec.push_back(token);
							msg.erase(0, pos + delimiter.length());
						}
						float x = stof(vec[1]);
						float y = stof(vec[2]);
						float z = stof(vec[3]);
						Vector3 position = Vector3(x, y, z);
						goose2pPos = position;
					}
					if (token == "score") {
						while ((pos = msg.find(delimiter)) != std::string::npos) {
							string token = msg.substr(0, pos);
							vec.push_back(token);
							msg.erase(0, pos + delimiter.length());
						}
						int x = stoi(vec[1]);
						score = x;
						
					}
					if (token == "state") {
						while ((pos = msg.find(delimiter)) != std::string::npos) {
							string token = msg.substr(0, pos);
							vec.push_back(token);
							msg.erase(0, pos + delimiter.length());
						}
						
						state = vec[1];

					}
				}
			}
			bool GetClientConnected() {
				return clientConnected;
			}
			Vector3 Get2pPosition() const { return goose2pPos; }

			int GetScore() { return score; }
			
			string GetState() { return state; }
		protected:
			string name;
			string state;
			Vector3 goose2pPos;
			int score;
			bool clientConnected;
		};

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

			void InitWorld();
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
			void Client();
			void Server();

			void SendPacket(bool server);

			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size);
			GameObject* AddMiniFloorToWorld(const Vector3& position, const Vector3& size,const Vector4& colour);
			GameObject* AddSlopeFloorToWorld(const Vector3& position, const Vector3& size,const Vector4& colour,float inverseMass,Vector3& angle);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddMenuToWorld(const Vector3& position, Vector3 dimensions);
			GameObject* AddTriggerToWorld(const Vector3& position, Vector3 dimensions,string name);
			GameObject* AddTrigger2ToWorld(const Vector3& position, float dimension, string name);
			GameObject* AddButtonToWorld(const Vector3& position, Vector3 dimensions, Vector3 colour, TextureBase* tex,string name);
			GameObject* trigger;
			GameObject* picker;
			GameObject* chaser;
			GameObject* chaser2;
			GameObject* pickupItems;

			GameObject* menu = new GameObject("menu");
			bool win;
			bool lose;

			int scoreP2;
			int scoreP1;
			bool chaseP1;
			GameObject* whoToChase;
			GameObject* state;

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
			GameObject* AddServerGooseToWorld(const Vector3& position);
			//GameObject* AddClientGooseToWorld(const Vector3& position);

			GameObject* goose2p;
			GameObject* AddParkKeeperToWorld(const Vector3& position);
			GameObject* AddHellKeeperToWorld(const Vector3& position);
			GameObject* AddCharacterToWorld(const Vector3& position);
			GameObject* AddAppleToWorld(const Vector3& position);
			GameObject* AddTreeToWorld(const Vector3& position);
			GameObject* AddTreeLeafToWorld(const Vector3& position);
			GameObject* AddTrampolineToWorld(const Vector3& position, const Vector3& size, const Vector4& colour,float bounciness);
			GameObject* AddIslandToWorld(const Vector3& position);
			GameObject* AddBaseFloorToWorld(const Vector3& position, const Vector3& size, const Vector4& colour);
			GameObject* AddWaterFloorToWorld(const Vector3& position, const Vector3& size, const Vector4& colour);
			GameObject* AddWaterFloor2ToWorld(const Vector3& position, const Vector3& size, const Vector4& colour);
			GameObject* AddPickablesToWorld(const Vector3& position, Vector3 dimensions, Vector4& colour);


			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			bool stunned = false;
			int stunnedCounter = 0;

			float		forceMagnitude;

			bool clientBool;
			bool serverBool;
			bool happened;
			bool clientConnectedServer;
			bool serverConnectedClient;
			bool single;

			TestPacketReceiver* serverReceiver;
			TestPacketReceiver* clientReceiver;

			GameClient* client;
			GameServer* server;

			GameObject* selectionObject = nullptr;

			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLTexture* gooseTex	= nullptr;
			OGLTexture* brickTex	= nullptr;
			OGLTexture* dogeTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	gooseMesh	= nullptr;
			OGLMesh*	hellkeeperMesh	= nullptr;
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
			int itemsPicked = 0;
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

