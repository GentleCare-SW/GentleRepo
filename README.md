# GentleRepo

This repository is a mono-repo that contains all code used in the systems/processes at GentleCare. At a high level, it is structured as such:

```
applications/
    GentleApp/

firmware/
    remote/
    vessel/

docs/
```

The `applications/` folder contains all repositories for high-level application code (e.g. mobile apps, web apps, etc.), the `firmware/` folder contains all repositories for low-level firmware code (e.g. ESP32 platform code, remote controller, etc.) and the `docs/` folder contains high level documentation of the systems.