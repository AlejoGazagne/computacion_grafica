/**
 * @file main.cpp
 * @brief OpenGL Flight Simulator - Entry Point
 * @version 2.0
 *
 * Punto de entrada principal del simulador de vuelo.
 * El motor gráfico ha sido modularizado en graphics_engine.
 */

#include <iostream>
#include "core/graphics_engine.h"

int main()
{
    try
    {
        GraphicsEngine engine;

        if (!engine.initialize())
        {
            std::cerr << "Failed to initialize graphics engine" << std::endl;
            return -1;
        }

        engine.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught" << std::endl;
        return -1;
    }

    return 0;
}
