```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

```
python main.py graphics.frag [result.out]
```

## Current Restraints
* OpenGL Version fixed to 450
* Only tested (somewhat-ish) on Fragment Shaders

Also 
* pybind11 has to be installed globally