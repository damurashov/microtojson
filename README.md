# microtojson

[![builds.sr.ht status](https://builds.sr.ht/~rkta/microtojson.svg)](https://builds.sr.ht/~rkta/microtojson?)

A simple embedded friendly JSON serializer.

This is still beta, use at own risk.

Supported types are:
- array
- JSON object
- string
- boolean
- integer
- unsigned integer

To create an arbitrary value use `t_to_value` and pass the correctly formatted value as char array.

See `test_mtojson.c` for usage.

Make sure structs are NULL terminated.

This project is mainly developed on [sourcehut](https://sr.ht/~rkta/microtojson/).
It is mirrored on [gitlab](https://gitlab.com/rkta/microtojson) and [github](https://github.com/rkta/microtojson).

If you want to contribute or find any bugs, feel free to choose the way that best fits your work style.

This project is heavily inspired by [microjson](https://gitlab.com/esr/microjson/) from Eric S. Raymond.

**Branches starting with `rk/` are considered private and will be force pushed!**
