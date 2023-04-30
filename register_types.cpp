#include "register_types.h"
#include "core/object/class_db.h"

//#include "Bitwise/BitwiseCharacter.h"
//#include "Bitwise/BaseStream.h"

//#include "Character/Character.h"
//#include "Character/Controller.h"
//#include "Character/InteractionServer.h"
//#include "Character/RealCharacter3D.h"

#include "TouchScreenUI/TouchControl.h"
#include "TouchScreenUI/TouchScreenPad.h"
#include "TouchScreenUI/TouchScreenDPad.h"
#include "TouchScreenUI/TouchScreenJoystick.h"
#include "TouchScreenUI/TouchScreenButton.h"

void initialize_authorMarthvon_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	//BaseStreamSignalStringNames::create();
	//
	//GDREGISTER_CLASS(BaseStream);
	//GDREGISTER_CLASS(InputPlug);
	//GDREGISTER_CLASS(OutputPlug);
	//
	//GDREGISTER_CLASS(Character3D);
	//GDREGISTER_CLASS(Player3D);
	//GDREGISTER_CLASS(RealCharacter3D);
	//GDREGISTER_CLASS(BitwiseCharacter);

	//Player3DController::Player3DStringNames::create();
	//GDREGISTER_CLASS(Controller);
	//GDREGISTER_CLASS(Player3DController);

	//GDREGISTER_CLASS(InteractionServer);
	//GDREGISTER_CLASS(Interactables);

	GDREGISTER_ABSTRACT_CLASS(TouchControl);
	GDREGISTER_ABSTRACT_CLASS(TouchScreenPad);
	GDREGISTER_CLASS(TouchScreenDPad);
	GDREGISTER_CLASS(TouchScreenJoystick);
	GDREGISTER_CLASS(TouchScreenButton);
}

void uninitialize_authorMarthvon_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}
