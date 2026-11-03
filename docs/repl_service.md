# REPL service

Commands use the existing service:

```
DDS ReplCmd -> DDSAdapterService -> AdapterServiceServer -> ZMQ -> master
```

Select the transport in the middleware YAML:

```yaml
dds:
  service:
    transport: zmq  # default; v3 master supports REPL over ZMQ
    command_uri: tcp://127.0.0.1:5555
    timeout_ms: 1000
```

The same service handles motor commands. Set `request.type` to `CTRL_CMD` (2)
and fill `request.ctrl_cmd`:

| Operation | ctrl_cmd.type | Arguments |
| --- | --- | --- |
| Start | CTRL_CMD_START (11) | board_id, value = control mode, optional gains |
| Stop | CTRL_CMD_STOP (12) | board_id |
| Set gains | CTRL_SET_GAINS (17) | board_id, gains including gains.type |

For START, `gains.type = 0` means no explicit gains: the converter omits the
protobuf Gains message and the master reads the existing drive gains for the
requested mode. A nonzero `gains.type` explicitly supplies gains, including an
intentional all-zero set. No separate SET_GAINS command is issued.

Gains are the named fields of the existing REPL protocol; the service forwards
them unchanged. Clients must supply the values expected by the drive.
The request/reply topics remain `rq/<namespace>/<robot>/replCmdRequest` and
`rr/<namespace>/<robot>/replCmdReply`.

ZMQ uses `MASTER_CMD` for master/FOE commands and `ESC_CMD` for slave commands.
The v3 master does not process SHM service queues; selecting `shm` is rejected
with a configuration error. PDO state and command traffic still uses SHM.
No command is retried automatically
and there is no transport fallback. A timeout can occur after the master executed
the command; it must not be treated as proof that the motor stayed stopped.
An ACK reports master acceptance, not a verified physical motor state.

The core `003_adapter_service` test sends START/STOP and a master command through
a local ZMQ mock and checks ACK/NACK, header preservation, malformed replies and timeout.
Each transaction uses a fresh REQ socket so late replies cannot satisfy a later
request. DDS request identifiers remain managed by the DDS service layer; the
v3 Protobuf protocol has no `request_id` field.
