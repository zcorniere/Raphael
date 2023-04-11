#include <Engine/Core/Log.hxx>
#include <Engine/Core/Application.hxx>

int main(int, char **)
{
    Application *App = new Application();

    check(App->Initialize());
    while (!App->ShouldExit()) {
        App->Tick(0.0f);
    }
    App->Shutdown();

    delete App;
    return 0;
}
