#include "launcher-application.h"

int main(int argc, char **argv)
{
    g_autoptr(AdwApplication) app = launcher_application_new();
    return g_application_run(G_APPLICATION(app), argc, argv);
}