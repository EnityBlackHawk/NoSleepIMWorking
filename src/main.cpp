#include "Application.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    Application app(hInstance);
    return app.run();
}
