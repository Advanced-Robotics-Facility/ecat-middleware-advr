Interface Definition Language (IDL)
====================================

Custom ADVRF Rx Interfaces
--------------------------

These interfaces describe data received from the
`ecat-master-advrf <https://github.com/Advanced-Robotics-Facility/ecat-master-advrf>`_
hardware layer.

.. list-table::
   :header-rows: 1
   :widths: 20 25 55

   * - Category
     - Interface
     - Description
   * - Actuator
     - ``Motor_``
     - State and feedback data for motor drives.
   * - Actuator
     - ``Gripper_``
     - Gripper state data.
   * - Actuator
     - ``Valve_``
     - Valve state data.
   * - Actuator
     - ``Pump_``
     - Pump state data.
   * - Sensor
     - ``Imu_``
     - Inertial measurement data.
   * - Sensor
     - ``ForceTorque_``
     - Force and torque measurements.
   * - System
     - ``PowerBoard_``
     - Power-board status and diagnostics.

Actuator interfaces
~~~~~~~~~~~~~~~~~~~

Motor
^^^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::Motor_
   :members:

Gripper
^^^^^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::Gripper_
   :members:

Valve
^^^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::Valve_
   :members:

Pump
^^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::Pump_
   :members:

Sensor interfaces
~~~~~~~~~~~~~~~~~

IMU
^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::Imu_
   :members:

Force/Torque sensor
^^^^^^^^^^^^^^^^^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::ForceTorque_
   :members:

System interfaces
~~~~~~~~~~~~~~~~~

Power board
^^^^^^^^^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::PowerBoard_
   :members:


Custom ADVRF Tx Interfaces
--------------------------

These interfaces describe command and configuration data sent to the
hardware layer.

.. list-table::
   :header-rows: 1
   :widths: 20 25 55

   * - Category
     - Interface
     - Description
   * - Actuator
     - ``MotorTxPdo_``
     - Motor drive commands and references.
   * - Actuator
     - ``GripperTxPdo_``
     - Gripper commands and references.
   * - Actuator
     - ``ValveTxPdo_``
     - Valve commands and references.
   * - Actuator
     - ``PumpTxPdo_``
     - Pump commands and references.
   * - Sensor
     - ``ForceTorqueTxPdo_``
     - Force/torque sensor configuration or command data.
   * - System
     - ``PowerBoardTxPdo_``
     - Power-board commands and configuration.

Actuator interfaces
~~~~~~~~~~~~~~~~~~~

Motor
^^^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::MotorTxPdo_
   :members:

Gripper
^^^^^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::GripperTxPdo_
   :members:

Valve
^^^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::ValveTxPdo_
   :members:

Pump
^^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::PumpTxPdo_
   :members:

Sensor interfaces
~~~~~~~~~~~~~~~~~

Force/Torque sensor
^^^^^^^^^^^^^^^^^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::ForceTorqueTxPdo_
   :members:

System interfaces
~~~~~~~~~~~~~~~~~

Power board
^^^^^^^^^^^

.. doxygenstruct:: advrf_interfaces::msg::dds_::PowerBoardTxPdo_
   :members:
