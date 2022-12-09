#include <Engine/Core/Application.hxx>

int main(int ac, char **av)
{
    logger.start();

    Raphael::Application app({});
    app.Run();
    return 0;
}
