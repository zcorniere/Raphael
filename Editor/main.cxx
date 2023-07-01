#include "EditorApplication.hxx"

#include <Engine/Core/Engine.hxx>

int main(int ac, char** av)
{
    return Engine::Start<EditorApplication>(ac, av);
}
