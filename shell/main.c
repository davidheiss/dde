#include "shell-application.h"

int main(int argc, char **argv)
{
    g_autoptr(GApplication) application = shell_application_new();
    return g_application_run(application, argc, argv);
}