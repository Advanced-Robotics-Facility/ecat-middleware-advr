Configuration
=============

Each plugin reads one middleware YAML document at startup. ``RobotConfig`` in
core contains only robot identity, namespace, and EtherCAT ID names.
``RuntimeConfig`` in core controls execution frequencies. DDS domain, ROS graph
integration, and endpoint QoS belong to ``advrf_dds_common``; native translation
belongs to each DDS plugin. Zenoh session and serialization settings belong to
the Zenoh plugin. Adapters receive typed configuration and do not load files.

Schema version 1
----------------

.. code-block:: yaml

   version: 1
   robot:
     name: kyon
     namespace: advrf
     joint_map:
       1: lf_haa_joint
       4: imu
   runtime:
     publishers_hz: 1000
     subscribers_hz: 100
     service_hz: 10
   dds:
     domain_id: 42
     declare_to_ros: true
     qos:
       state:
         reliability: best_effort
         durability: volatile
         history: keep_last
         depth: 1
       command:
         reliability: reliable
         durability: volatile
         history: keep_last
         depth: 1
   zenoh:
     wire_format: protobuf
     session_config: session.json5

``robot.name`` is required; namespace defaults to empty. ID names must be
nonempty and IDs positive. At least one mapped device is required for discovery.
Runtime defaults are 1000, 100, and 10 Hz, respectively, and frequencies must be
between 1 and 1,000,000 Hz. DDS domain defaults to 0 (supported range 0--232), ROS
graph declaration to false, and both endpoint policies to best-effort, volatile,
keep-last depth 1. Select the state and command policies for the peers you use;
the example above is not a requirement. Both DDS backends apply reliability,
durability, and depth to state writers and command readers. Service and ROS
discovery QoS keep their existing dedicated policies.

Only ``keep_last`` history is supported. Reliability accepts ``reliable`` and
``best_effort``; durability accepts ``volatile``, ``transient_local``,
``transient``, and ``persistent``. Depth must be in 1--INT32_MAX. Invalid values,
unknown settings within the parsed sections, unsupported schema versions, and
malformed files fail startup before shared-memory discovery or middleware entity
creation. Inactive transport sections are not interpreted: Zenoh does not need a
valid DDS configuration, and DDS plugins do not need a valid Zenoh configuration.

Zenoh defaults to ``protobuf``. ``ros2cdr`` requires a build with
``ZENOH_ROS2_SUPPORT=ON``. ``session_config`` refers to a native Zenoh JSON5 file;
relative YAML references resolve against the middleware YAML's directory.
The plugin validates the native session file before starting discovery.
FastDDS and CycloneDDS retain their native library configuration mechanisms;
additional native YAML options should be added to their plugin-owned
``config/plugin_config.hpp``, not to core.

Overrides and migration
-----------------------

All three plugins accept ``--config PATH``, ``--rate-publishers HZ``,
``--rate-subscribers HZ``, and ``--rate-service HZ``. The old underscore rate
flags remain aliases. Only supplied flags override the YAML values:

.. code-block:: bash

   fastdds_plugin --config ./middleware/config.yaml --rate-publishers 500
   cyclonedds_plugin --config ./middleware/config.yaml
   zenoh_plugin --config ./middleware/config.yaml --wire-format protobuf

The common precedence is defaults, YAML, then explicit CLI overrides, followed
by validation. The default YAML path is resolved at startup from ``EC_CFG``. This variable
points to the robot ``ec_cfg.yaml``; ``robot.robot_middleware_folder`` in that
file selects the folder containing ``config.yaml``. Relative paths and
``${PWD}`` / ``$PWD`` resolve against the directory of the actual configuration
file, following symbolic links. ``--config`` overrides this lookup. Changing
robots requires no recompilation. Zenoh also accepts
``--zenoh-config PATH``: an explicit CLI path overrides the YAML reference and
resolves against the working directory. If neither CLI nor YAML selects a
session file, ``ZENOH_CONFIG`` is used when present, otherwise Zenoh defaults.


Legacy files remain readable: ``dds.robot_name``, ``dds.namespace``,
``dds.domain``, and top-level ``joint_map`` are accepted. A new ``robot`` block
takes precedence over legacy identity and mapping. A flat legacy ``dds.qos``
block applies to both state and command endpoints. Previously this block was
ignored by the robot loader; it now affects DDS delivery, so review existing
QoS values when migrating. Supplied robot files retain their explicit QoS values
in separate state and command profiles.

C++ callers should load ``ConfigFile`` once, call the selected plugin's
``parse_plugin_config()``, apply overrides, then call ``validate()``. DDS adapter
initialization now accepts a separate ``DdsConfig`` argument; pass the parsed
DDS settings to preserve ROS graph declaration and QoS. ``RobotConfig`` no
longer exposes ``domain_id`` or ``declare_to_ros``. Compatibility two-file robot
loaders return ``nullopt`` on failure; new startup paths report errors through
``ConfigError``. The robot builder returns an owned value.

Service execution remains disabled pending the existing master-side fix.
``service_hz`` is parsed and validated but does not enable the service adapter.
