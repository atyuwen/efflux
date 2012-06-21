#include "common.hpp"
#include "d3d_app.hpp"

const int width  = 720;
const int height = 480;

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int show)
{
	D3DApp *app = D3DApp::GetApp();
	if (app->Initialize(instance, width, height)) {
		app->Run();
	}
	app->Destroy();
}
