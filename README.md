# microtojson

[![builds.sr.ht status](https://builds.sr.ht/~rkta/microtojson.svg)](https://builds.sr.ht/~rkta/microtojson?)

A simple embedded friendly JSON serializer.

This is still beta, use at own risk.

Make sure structs are NULL terminated.

Currently only string, boolean and integer are valid types.
To create an arbitrary value use `t_to_value` and pass the correctly formatted value as char array.

See `test_mtojson.c` for usage.

This project is heavily inspired by [microjson](https://gitlab.com/esr/microjson/) from Eric S. Raymond.

**Branches starting with `rk/` are considered private and will be force pushed!**
