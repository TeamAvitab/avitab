# Avitab internal interfaces

This directory contains the abstract interfaces between the Avitab core
library, its environment, and the product wrappers that bring everything
together.

The core library is (or should be) agnostic about its environment and accesses
all environmental services through the interfaces defined in the headers in this
directory.

The environment is made up of drivers for the simulation, window, and platform.
Implementations are available for the different simulations, window services, and
platforms.

A product is built by creating a wrapper that instantiates the Avitab core and the
appropriate drivers, and then hooking them together.
