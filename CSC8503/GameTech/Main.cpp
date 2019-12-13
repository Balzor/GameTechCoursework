#include "../../Common/Window.h"
#include "../CSC8503Common/NavigationGrid.h"

#include "TutorialGame.h"

using namespace NCL;
using namespace CSC8503;

//void TestStateMachine() {
	//StateMachine* testMachine = new StateMachine();
	//
	//int someData = 0;
	//
	//StateFunc AFunc = [](void* data) {
	//int* realData = (int*)data;
	//	 (*realData)++;
	//	 std::cout << "In State A!" << std::endl;
	//};
	//StateFunc BFunc = [](void* data) {
	//	 int* realData = (int*)data;
	//	 (*realData) --;
	//	 std::cout << "In State B!" << std::endl;
	//};
	//
	//GenericState * stateA = new GenericState(AFunc, (void*)& someData);
	//GenericState * stateB = new GenericState(BFunc, (void*)& someData);
	//testMachine -> AddState(stateA);
	//testMachine -> AddState(stateB);

	//GenericTransition <int&, int >* transitionA = new GenericTransition <int&, int >(
	//GenericTransition <int&, int >::GreaterThanTransition,
	//someData, 10, stateA, stateB); // if greater than 10 , A to B
	//
	//GenericTransition <int&, int > * transitionB = new GenericTransition <int&, int >(
	//GenericTransition <int&, int >::EqualsTransition,
	//someData, 0, stateB, stateA); // if equals 0 , B to A

	//testMachine -> AddTransition(transitionA);
	//testMachine -> AddTransition(transitionB);
	//
	//for (int i = 0; i < 100; ++i) {
	//	testMachine -> Update(); // run the state machine !
	//	
	//}
	//delete testMachine;
//}
/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/
bool begin = true;
int main() {
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);

	if (!w->HasInitialised()) {
		return -1;
	}	

	//TestStateMachine();
	//TestPathfinding();
	//TestNetworking();

	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	TutorialGame* g = new TutorialGame();

	w->GetTimer()->GetTimeDeltaSeconds();
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		try {
			float dt = w->GetTimer()->GetTimeDeltaSeconds();

			if (dt > 0.1f) {
				std::cout << "Skipping large time delta" << std::endl;
				continue; //must have hit a breakpoint or something to have a 1 second frame time!
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
				w->ShowConsole(true);
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
				w->ShowConsole(false);
			}

			w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));
			
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) {
				begin = !begin;
			}
			if (begin) {
				g->UpdateGame(dt);
			}
			//Debug::Print("Test", Vector2(1000, 40));
		} catch (int i) {
			delete g;
			g = new TutorialGame();
		}
	}
	Window::DestroyGameWindow();
}