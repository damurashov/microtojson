# microtojson

A simple embedded friendly JSON serializer.

This is still beta, use at own risk.

Make sure structs are NULL terminated.

Currently only string and integer are valid types and all input must be char arrays.
Using an char array and abusing type integer it is possible to create any type.

See `test_mtojson.c` for usage.

This project is heavily inspired by microjson from Eric S. Raymond (https://gitlab.com/esr/microjson/).

**Branches starting with `rk/` are considered private and will be force pushed!**
