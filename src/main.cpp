#include "VulkanApp.h"

int main()
{
    VulkanApp app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}