# Middleware configuration

Plugins and examples resolve their configuration at startup using `EC_CFG`:

```bash
export EC_CFG=/home/hpousseur-iit.local/ws/ws_v3.x/src/ecat-middleware-advr/config/advrf_config/configs/ecat-pegasus-config/ec_cfg.yaml
```

The selected file must contain:

```yaml
middleware:
  folder: '${PWD}/middleware/'
```

`${PWD}` (or `$PWD`) denotes the directory of the actual `ec_cfg.yaml` file,
following symbolic links. Relative folders are resolved from that directory;
absolute folders are also supported. The middleware loads `config.yaml` from
this folder. Missing `EC_CFG`, unreadable files, or missing folder settings
produce an error. Plugins accept `--config <PATH>` to override this lookup.
Changing `EC_CFG` before starting a process requires no recompilation.
The legacy `robot.robot_middleware_folder` key is accepted when
`middleware.folder` is absent; the new key takes priority when both are present.

When `EC_CFG` is set, `robot.robot_id_map_path` in that file selects the
YAML containing `joint_map`. This map replaces the middleware's `robot.joint_map`
completely; the middleware still supplies `robot.name` and `robot.namespace`.
The same relative-path and `${PWD}` rules apply. An invalid or missing referenced
map is an error. Without `EC_CFG`, standalone `--config` files retain their
inline joint map support.
