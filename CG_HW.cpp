#include "PongGame.h"
#include "SolarSystem.h"
#include "Katamari.h"

int main()
{
	HINSTANCE hinst = GetModuleHandle(nullptr);
	LPCWSTR applicationName2 = L"Pong";
	LPCWSTR applicationName3 = L"SolarSys";
	LPCWSTR applicationName4 = L"Katamari";

	//Lab 2
	/*PongGame Pong(hinst, applicationName2);
	
	Pong.Initialize(15);

	Pong.PrepareResources();

	Pong.Run();*/

	//Lab 3
	/*SolarSystem SolarSys(hinst, applicationName3);
	SolarSys.Initialize(214);

	SolarSys.PrepareResources();

	SolarSys.Run();*/

	//Lab 4
	Katamari Katamari(hinst, applicationName4);

	Katamari.Initialize(2000, 100, 80000);

	Katamari.PrepareResources();

	Katamari.Run();

}