# Middleware configuration

Plugins and examples resolve their configuration at startup using `EC_CFG`:

```bash
export EC_CFG=/home/hpousseur-iit.local/ws/ws_v3.x/src/ecat-middleware-advr/config/advrf_config/configs/ecat-pegasus-config/ec_cfg.yaml
```

The selected file must contain:

```yaml
robot:
  robot_middleware_folder: '${PWD}/middleware/'
```

`${PWD}` (or `$PWD`) denotes the directory of the actual `ec_cfg.yaml` file,
following symbolic links. Relative folders are resolved from that directory;
absolute folders are also supported. The middleware loads `config.yaml` from
this folder. Missing `EC_CFG`, unreadable files, or missing folder settings
produce an error. Plugins accept `--config <PATH>` to override this lookup.
Changing `EC_CFG` before starting a process requires no recompilation.
