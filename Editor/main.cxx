#include <Engine/Core/Log.hxx>
#include <Engine/Platforms/Application.hxx>

int main(int, char **)
{
    Application *App = new Application();

    check(App->Initialize());
    while (!App->ShouldExit()) {
        App->Tick(0.0f);
    }
    App->Shutdown();

    return 0;
}
