# Motor mode and activation service

Motor commands now follow this path:

```
ROS 2 service client -> DDSAdapterMotorControl
    -> core adapters_mode::MotorControlAdapter -> ZeroMQ -> EtherCAT master
```

The implementation belongs to `advrf_middleware_core/adapters_mode/motor_control.hpp`
and `src/motor_control.cpp`. It has no ROS or DDS dependency. It uses the existing
master command protocol: a ZeroMQ REQ socket sends a multipart message containing
the string `ESC_CMD` first, followed by serialized `iit.advrf.Repl_cmd`. It parses and checks the
master's `Cmd_reply`. Motor command transport does not use SHM. Cyclic PDOs still
use their existing DDS/SHM path; discovery still reads RX PDO SHM.

## Configuration

In the middleware YAML loaded by `cyclonedds_plugin`:

```yaml
adapters_mode:
  motor_control: true

motor_control:
  command_uri: tcp://127.0.0.1:5555
  timeout_ms: 1000
  allowed_motor_ids: [1, 2, 3]
```

The adapter is disabled when `adapters_mode.motor_control` is absent/false.
Omit `allowed_motor_ids` to permit the discovered motors of this robot. An explicit
list can narrow that set but cannot add unknown devices or sensors. The plugin
uses discovery to distinguish ADVRF and CiA402 drives. For ADVRF position/velocity
commands it applies the same gain reordering as `EcReplCmd`; Synapticon gains are
passed unchanged. Mode/gain semantics otherwise remain those of the master.

The adapter runs in its own worker at `runtime.service_hz`, with a bounded
synchronous ZeroMQ transaction. It does not block the publisher/subscriber workers.
`is_ok()` indicates local transport readiness, not a confirmed master connection.
Starting the adapter itself does not activate any motor.

## Commands

| Value | Command | Master operation |
| --- | --- | --- |
| 0 | SHUTDOWN | CTRL_CMD_STOP |
| 1 | START | CTRL_CMD_START with control mode and five gains |
| 2 | STOP | CTRL_CMD_STOP |
| 3 | SET_GAINS | CTRL_SET_GAINS with control mode and five gains |

SHUTDOWN is an alias of the existing STOP operation, not a power-off operation or
an independent implementation of the CiA402 state machine. The master owns drive
state transitions, brakes, limits and activation. This service does not issue
fault resets, motor references or automatic retries. START acknowledgement does
not verify that a physical motor has reached Operation Enabled.

## DDS service transport

The plugin exposes only the ROS-compatible request/reply endpoint pair below.
The earlier custom `/motor_control/request` and `/motor_control/reply` endpoints
have been removed. Native DDS callers can use the service wire IDL, while ROS
clients use the normal `.srv` without any transport-specific code.

Requests are reliable and volatile, so they are not retained for late service
instances. The GUID/sequence correlation header is copied to replies. Recent
replies are cached (256 entries) to suppress duplicate execution; this is not a
persistent exactly-once guarantee across restarts or cache eviction. No motor
command is automatically retried.

## Standard ROS 2 service

The middleware now also exposes `/advrf/pegasus/motor_control` as
`advrf_interfaces/srv/MotorControl`, without a separate bridge process:

- Request topic: `rq/advrf/pegasus/motor_controlRequest`.
- Reply topic: `rr/advrf/pegasus/motor_controlReply`.
- Wire IDL: `advrf_interfaces/srv/MotorControl.idl`.
- Correlation header: the GUID/sequence pair used by the deployed CycloneDDS RMW.
- Reader/writer endpoints are advertised in `ros_discovery_info` by the middleware.

The middleware adds/removes the service correlation framing; it is not part of
the public ROS `.srv`. All ROS nodes use standard `rclpy`/`rclcpp` APIs with
`advrf_interfaces` messages, without importing/linking native DDS libraries.
The current native service mapping is tested with `rmw_cyclonedds_cpp` on Jazzy;
it must not be assumed to implement other vendors' service wire mappings. The
ROS package itself does not select or depend on an RMW vendor. Details of the
implemented framing follow the upstream
[request header](https://github.com/ros2/rmw_cyclonedds/blob/jazzy/rmw_cyclonedds_cpp/src/serdata.hpp)
and [service mapping](https://github.com/ros2/rmw_cyclonedds/blob/jazzy/rmw_cyclonedds_cpp/src/rmw_node.cpp).

```bash
export ROS_DOMAIN_ID=42
ros2 service call /advrf/pegasus/motor_control advrf_interfaces/srv/MotorControl \
  '{motor_id: 1, command: 1, control_mode: 59, gains: [0.1, 0.0, 0.0005, 0.0, 0.0]}'

ros2 service call /advrf/pegasus/motor_control advrf_interfaces/srv/MotorControl \
  '{motor_id: 1, command: 2}'
```

The `motor_command` node in `advrf_motor_control` is a normal ROS service client.
It provides `start`, `stop`, `shutdown`, `set-gains`, configuration and ID overrides.
No Python DDS environment or bridge node is needed. The trajectory server also
uses a normal ROS publisher for `MotorTxPdoVector`. Its launch calls the ROS service
when `start_motors:=true`. The master ZeroMQ endpoint remains middleware-only.

`success=true` means a checked master ACK, not verification of physical drive
state. A missing or malformed response after sending yields
`outcome_unknown=true`: the operation may already have happened. Clients must not
automatically retry. The ROS client's default timeout is 3 seconds; configure it
above the middleware ZeroMQ budget (1000 ms per send/receive by default).
Commands to multiple motors are sequential; earlier motors can remain enabled
if a later command fails.

## Tests

Core `003_motor_control`: actual ZeroMQ multipart framing and Protobuf commands
against a local mock, gain reordering, invalid IDs/modes/gains, ACK/NACK, malformed
reply and timeout. CycloneDDS `005_motor_control`: DDS correlation, ID rejection,
duplicate suppression and START/STOP/SHUTDOWN via the same core implementation.
Both keep checks enabled in Release builds. The latter also offers `--serve-ros` for
the standard ROS service test in `advrf_motor_control/test/motor_service_integration.py`; it uses
DDS domain 217 and a random mock ZeroMQ endpoint, with no hardware connection.
