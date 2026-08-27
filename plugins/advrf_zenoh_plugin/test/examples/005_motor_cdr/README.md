# ROS 2 to middleware: motor command over Zenoh CDR

This example shows a ROS 2 `advrf_interfaces/msg/MotorTxPdoVector` command as
the Zenoh plugin converts it into an `Ec_slave_pdo`. The viewer uses the same
`Ros2CdrDeserializer` as the plugin and may run beside it: Zenoh delivers the
sample to both subscribers.

## Build

Select and build the Zenoh middleware with ROS 2 CDR support and examples:

```bash
./scripts/select_middleware.sh zenoh
colcon build \
  --packages-select advrf_zenoh_plugin \
  --cmake-args -DBUILD_ZENOH=ON -DZENOH_ROS2_SUPPORT=ON -DBUILD_EXAMPLES=ON
source install/setup.bash
```

## Run

The ROS 2 terminal must also source the interface workspace that provides
`advrf_interfaces/msg/MotorTxPdoVector`. This repository generates compatible
CDR C++ types for the plugin, but does not register that package with the ROS 2
CLI. Verify the publisher environment before continuing:

```bash
ros2 interface show advrf_interfaces/msg/MotorTxPdoVector
```

Use four terminals, each with the appropriate workspace sourced.

1. Start the EtherCAT process that owns `/ecat_tx_pdo`, then start the plugin:

   ```bash
   zenoh_plugin --wire-format ros2-cdr
   ```

2. Start the ROS 2 bridge:

   ```bash
   zenoh-bridge-ros2dds
   ```

3. Start this viewer:

   ```bash
   zenoh_examples_005_motor_cdr_viewer
   ```

   It derives the key from the installed robot configuration. To inspect a
   different robot or namespace, pass the exact Zenoh key explicitly:

   ```bash
   zenoh_examples_005_motor_cdr_viewer --key rt/advrf/pegasus/tx/motors
   ```

4. Publish one motor command from ROS 2. The ROS topic is the Zenoh key with a
   leading slash:

   ```bash
   ros2 topic pub --once \
     /rt/advrf/pegasus/tx/motors \
     advrf_interfaces/msg/MotorTxPdoVector \
     "{data: [{ecat_id: 3, pos_ref: 1.25, vel_ref: 0.0, tor_ref: 0.0, cur_ref: 0.0, gain_0: 10.0, gain_1: 2.0, gain_2: 0.0, gain_3: 0.0, gain_4: 0.0, fault_ack: 0, ts: 42, op_idx_aux: 0, aux: 0.0}]}"
   ```

The viewer prints the CDR payload size followed by the converted middleware
fields. For the legacy motor command path, `gain_0` becomes `gainp` and
`gain_1` becomes `gaind`; fields not represented by the legacy `Motor_tx_pdo`
are intentionally absent from the printed middleware PDO.

To inspect the command after the real plugin has enqueued it in shared memory,
run the existing inspector and select the `motor` queue:

```bash
shm_inspector --tx /ecat_tx_pdo --history
```

If the installed configuration does not use `pegasus`, replace that component
in the ROS topic with the robot name printed by the viewer's startup line.
