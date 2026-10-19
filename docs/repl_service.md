# REPL service

Commands use the existing service:

```
DDS ReplCmd -> DDSAdapterService -> AdapterServiceServer -> SHM or ZMQ -> master
```

Select the transport in the middleware YAML:

```yaml
dds:
  service:
    transport: zmq  # zmq or shm; default: shm
    command_uri: tcp://127.0.0.1:5555
    timeout_ms: 1000
```

The same service handles motor commands. Set `request.type` to `CTRL_CMD` (2)
and fill `request.ctrl_cmd`:

| Operation | ctrl_cmd.type | Arguments |
| --- | --- | --- |
| Start | CTRL_CMD_START (11) | board_id, value = control mode, gains |
| Stop | CTRL_CMD_STOP (12) | board_id |
| Set gains | CTRL_SET_GAINS (17) | board_id, gains including gains.type |

Gains are the named fields of the existing REPL protocol; the service forwards
them unchanged. Clients must supply the values expected by the drive.
The request/reply topics remain `rq/<namespace>/<robot>/replCmdRequest` and
`rr/<namespace>/<robot>/replCmdReply`.

ZMQ uses `MASTER_CMD` for master/FOE commands and `ESC_CMD` for slave commands.
SHM uses the existing master service queues. No command is retried automatically
and there is no transport fallback. A timeout can occur after the master executed
the command; it must not be treated as proof that the motor stayed stopped.
An ACK reports master acceptance, not a verified physical motor state.

The core `003_adapter_service` test sends START/STOP and a master command through
a local ZMQ mock and checks ACK/NACK, correlation, malformed replies and timeout.
