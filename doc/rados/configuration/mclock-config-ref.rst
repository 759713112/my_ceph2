========================
 mClock Config Reference
========================

.. index:: mclock; configuration

QoS support in Ceph is implemented using a queuing scheduler based on `the
dmClock algorithm`_. See :ref:`dmclock-qos` section for more details.

.. note:: The *mclock_scheduler* is supported for BlueStore OSDs. For Filestore
          OSDs the *osd_op_queue* is set to *wpq* and is enforced even if you
          attempt to change it.

To make the usage of mclock more user-friendly and intuitive, mclock config
profiles are introduced. The mclock profiles mask the low level details from
users, making it easier to configure and use mclock.

The following input parameters are required for a mclock profile to configure
the QoS related parameters:

* total capacity (IOPS) of each OSD (determined automatically -
  See `OSD Capacity Determination (Automated)`_)

* an mclock profile type to enable

Using the settings in the specified profile, an OSD determines and applies the
lower-level mclock and Ceph parameters. The parameters applied by the mclock
profile make it possible to tune the QoS between client I/O and background
operations in the OSD.


.. index:: mclock; mclock clients

mClock Client Types
===================

The mclock scheduler handles requests from different types of Ceph services.
Each service can be considered as a type of client from mclock's perspective.
Depending on the type of requests handled, mclock clients are classified into
the buckets as shown in the table below,

+------------------------+----------------------------------------------------+
|  Client Type           | Request Types                                      |
+========================+====================================================+
| Client                 | I/O requests issued by external clients of Ceph    |
+------------------------+----------------------------------------------------+
| Background recovery    | Internal recovery/backfill requests                |
+------------------------+----------------------------------------------------+
| Background best-effort | Internal scrub, snap trim and PG deletion requests |
+------------------------+----------------------------------------------------+

The mclock profiles allocate parameters like reservation, weight and limit
(see :ref:`dmclock-qos`) differently for each client type. The next sections
describe the mclock profiles in greater detail.


.. index:: mclock; profile definition

mClock Profiles - Definition and Purpose
========================================

A mclock profile is *“a configuration setting that when applied on a running
Ceph cluster enables the throttling of the operations(IOPS) belonging to
different client classes (background recovery, scrub, snaptrim, client op,
osd subop)”*.

The mclock profile uses the capacity limits and the mclock profile type selected
by the user to determine the low-level mclock resource control configuration
parameters and apply them transparently. Additionally, other Ceph configuration
parameters are also applied. Please see sections below for more information.

The low-level mclock resource control parameters are the *reservation*,
*limit*, and *weight* that provide control of the resource shares, as
described in the :ref:`dmclock-qos` section.


.. index:: mclock; profile types

mClock Profile Types
====================

mclock profiles can be broadly classified into *built-in* and *custom* profiles,

Built-in Profiles
-----------------
Users can choose between the following built-in profile types:

.. note:: The values mentioned in the tables below represent the percentage
          of the total IOPS capacity of the OSD allocated for the service type.

high_client_ops (*default*)
^^^^^^^^^^^^^^^^^^^^^^^^^^^
This profile optimizes client performance over background activities by
allocating more reservation and limit to client operations as compared to
background operations in the OSD. This profile is enabled by default. The table
shows the resource control parameters set by the profile:

+------------------------+-------------+--------+-------+
|  Service Type          | Reservation | Weight | Limit |
+========================+=============+========+=======+
| client                 | 50%         | 2      | MAX   |
+------------------------+-------------+--------+-------+
| background recovery    | 25%         | 1      | 100%  |
+------------------------+-------------+--------+-------+
| background best-effort | 25%         | 2      | MAX   |
+------------------------+-------------+--------+-------+

high_recovery_ops
^^^^^^^^^^^^^^^^^
This profile optimizes background recovery performance as compared to external
clients and other background operations within the OSD. This profile, for
example, may be enabled by an administrator temporarily to speed-up background
recoveries during non-peak hours. The table shows the resource control
parameters set by the profile:

+------------------------+-------------+--------+-------+
|  Service Type          | Reservation | Weight | Limit |
+========================+=============+========+=======+
| client                 | 30%         | 1      | 80%   |
+------------------------+-------------+--------+-------+
| background recovery    | 60%         | 2      | 200%  |
+------------------------+-------------+--------+-------+
| background best-effort | 1 (MIN)     | 2      | MAX   |
+------------------------+-------------+--------+-------+

balanced
^^^^^^^^
This profile allocates equal reservation to client I/O operations and background
recovery operations. This means that equal I/O resources are allocated to both
external and background recovery operations. This profile, for example, may be
enabled by an administrator when external client performance requirement is not
critical and there are other background operations that still need attention
within the OSD.

+------------------------+-------------+--------+-------+
|  Service Type          | Reservation | Weight | Limit |
+========================+=============+========+=======+
| client                 | 40%         | 1      | 100%  |
+------------------------+-------------+--------+-------+
| background recovery    | 40%         | 1      | 150%  |
+------------------------+-------------+--------+-------+
| background best-effort | 20%         | 2      | MAX   |
+------------------------+-------------+--------+-------+

.. note:: Across the built-in profiles, internal background best-effort clients
          of mclock ("scrub", "snap trim", and "pg deletion") are given lower
          reservations but no limits(MAX). This ensures that requests from such
          clients are able to complete quickly if there are no other competing
          operations.


Custom Profile
--------------
This profile gives users complete control over all the mclock configuration
parameters. This profile should be used with caution and is meant for advanced
users, who understand mclock and Ceph related configuration options.


.. index:: mclock; built-in profiles

mClock Built-in Profiles
========================

When a built-in profile is enabled, the mClock scheduler calculates the low
level mclock parameters [*reservation*, *weight*, *limit*] based on the profile
enabled for each client type. The mclock parameters are calculated based on
the max OSD capacity provided beforehand. As a result, the following mclock
config parameters cannot be modified when using any of the built-in profiles:

- :confval:`osd_mclock_scheduler_client_res`
- :confval:`osd_mclock_scheduler_client_wgt`
- :confval:`osd_mclock_scheduler_client_lim`
- :confval:`osd_mclock_scheduler_background_recovery_res`
- :confval:`osd_mclock_scheduler_background_recovery_wgt`
- :confval:`osd_mclock_scheduler_background_recovery_lim`
- :confval:`osd_mclock_scheduler_background_best_effort_res`
- :confval:`osd_mclock_scheduler_background_best_effort_wgt`
- :confval:`osd_mclock_scheduler_background_best_effort_lim`

The following Ceph options will not be modifiable by the user:

- :confval:`osd_max_backfills`
- :confval:`osd_recovery_max_active`

This is because the above options are internally modified by the mclock
scheduler in order to maximize the impact of the set profile.

By default, the *high_client_ops* profile is enabled to ensure that a larger
chunk of the bandwidth allocation goes to client ops. Background recovery ops
are given lower allocation (and therefore take a longer time to complete). But
there might be instances that necessitate giving higher allocations to either
client ops or recovery ops. In order to deal with such a situation, the
alternate built-in profiles may be enabled by following the steps mentioned
in the next section.

If any mClock profile (including "custom") is active, the following Ceph config
sleep options will be disabled,

- :confval:`osd_recovery_sleep`
- :confval:`osd_recovery_sleep_hdd`
- :confval:`osd_recovery_sleep_ssd`
- :confval:`osd_recovery_sleep_hybrid`
- :confval:`osd_scrub_sleep`
- :confval:`osd_delete_sleep`
- :confval:`osd_delete_sleep_hdd`
- :confval:`osd_delete_sleep_ssd`
- :confval:`osd_delete_sleep_hybrid`
- :confval:`osd_snap_trim_sleep`
- :confval:`osd_snap_trim_sleep_hdd`
- :confval:`osd_snap_trim_sleep_ssd`
- :confval:`osd_snap_trim_sleep_hybrid`

The above sleep options are disabled to ensure that mclock scheduler is able to
determine when to pick the next op from its operation queue and transfer it to
the operation sequencer. This results in the desired QoS being provided across
all its clients.


.. index:: mclock; enable built-in profile

Steps to Enable mClock Profile
==============================

As already mentioned, the default mclock profile is set to *high_client_ops*.
The other values for the built-in profiles include *balanced* and
*high_recovery_ops*.

If there is a requirement to change the default profile, then the option
:confval:`osd_mclock_profile` may be set during runtime by using the following
command:

  .. prompt:: bash #

    ceph config set osd.N osd_mclock_profile <value>

For example, to change the profile to allow faster recoveries on "osd.0", the
following command can be used to switch to the *high_recovery_ops* profile:

  .. prompt:: bash #

    ceph config set osd.0 osd_mclock_profile high_recovery_ops

.. note:: The *custom* profile is not recommended unless you are an advanced
          user.

And that's it! You are ready to run workloads on the cluster and check if the
QoS requirements are being met.


Switching Between Built-in and Custom Profiles
==============================================

There may be situations requiring switching from a built-in profile to the
*custom* profile and vice-versa. The following sections outline the steps to
accomplish this.

Steps to Switch From a Built-in to the Custom Profile
-----------------------------------------------------

The following command can be used to switch to the *custom* profile:

  .. prompt:: bash #

    ceph config set osd osd_mclock_profile custom

For example, to change the profile to *custom* on all OSDs, the following
command can be used:

  .. prompt:: bash #

    ceph config set osd osd_mclock_profile custom

After switching to the *custom* profile, the desired mClock configuration
option may be modified. For example, to change the client reservation IOPS
allocation for a specific OSD (say osd.0), the following command can be used:

  .. prompt:: bash #

    ceph config set osd.0 osd_mclock_scheduler_client_res 3000

.. important:: Care must be taken to change the reservations of other services like
   recovery and background best effort accordingly to ensure that the sum of the
   reservations do not exceed the maximum IOPS capacity of the OSD.

.. tip::  The reservation and limit parameter allocations are per-shard based on
   the type of backing device (HDD/SSD) under the OSD. See
   :confval:`osd_op_num_shards_hdd` and :confval:`osd_op_num_shards_ssd` for
   more details.

Steps to Switch From the Custom Profile to a Built-in Profile
-------------------------------------------------------------

Switching from the *custom* profile to a built-in profile requires an
intermediate step of removing the custom settings from the central config
database for the changes to take effect.

The following sequence of commands can be used to switch to a built-in profile:

#. Set the desired built-in profile using:

   .. prompt:: bash #

     ceph config set osd <mClock Configuration Option>

   For example, to set the built-in profile to ``high_client_ops`` on all
   OSDs, run the following command:

   .. prompt:: bash #

     ceph config set osd osd_mclock_profile high_client_ops
#. Determine the existing custom mClock configuration settings in the central
   config database using the following command:

   .. prompt:: bash #

     ceph config dump
#. Remove the custom mClock configuration settings determined in the previous
   step from the central config database:

   .. prompt:: bash #

     ceph config rm osd <mClock Configuration Option>

   For example, to remove the configuration option
   :confval:`osd_mclock_scheduler_client_res` that was set on all OSDs, run the
   following command:

   .. prompt:: bash #

     ceph config rm osd osd_mclock_scheduler_client_res
#. After all existing custom mClock configuration settings have been removed
   from the central config database, the configuration settings pertaining to
   ``high_client_ops`` will come into effect. For e.g., to verify the settings
   on osd.0 use:

   .. prompt:: bash #

     ceph config show osd.0

Switch Temporarily Between mClock Profiles
------------------------------------------

To switch between mClock profiles on a temporary basis, the following commands
may be used to override the settings:

.. warning:: This section is for advanced users or for experimental testing. The
   recommendation is to not use the below commands on a running cluster as it
   could have unexpected outcomes.

.. note:: The configuration changes on an OSD using the below commands are
   ephemeral and are lost when it restarts. It is also important to note that
   the config options overridden using the below commands cannot be modified
   further using the *ceph config set osd.N ...* command. The changes will not
   take effect until a given OSD is restarted. This is intentional, as per the
   config subsystem design. However, any further modification can still be made
   ephemerally using the commands mentioned below.

#. Run the *injectargs* command as shown to override the mclock settings:

   .. prompt:: bash #

     ceph tell osd.N injectargs '--<mClock Configuration Option>=<value>'

   For example, the following command overrides the
   :confval:`osd_mclock_profile` option on osd.0:

   .. prompt:: bash #

     ceph tell osd.0 injectargs '--osd_mclock_profile=high_recovery_ops'


#. An alternate command that can be used is:

   .. prompt:: bash #

     ceph daemon osd.N config set <mClock Configuration Option> <value>

   For example, the following command overrides the
   :confval:`osd_mclock_profile` option on osd.0:

   .. prompt:: bash #

     ceph daemon osd.0 config set osd_mclock_profile high_recovery_ops

The individual QoS-related config options for the *custom* profile can also be
modified ephemerally using the above commands.


OSD Capacity Determination (Automated)
======================================

The OSD capacity in terms of total IOPS is determined automatically during OSD
initialization. This is achieved by running the OSD bench tool and overriding
the default value of ``osd_mclock_max_capacity_iops_[hdd, ssd]`` option
depending on the device type. No other action/input is expected from the user
to set the OSD capacity.

.. note:: If you wish to manually benchmark OSD(s) or manually tune the
          Bluestore throttle parameters, see section
          `Steps to Manually Benchmark an OSD (Optional)`_.

You may verify the capacity of an OSD after the cluster is brought up by using
the following command:

  .. prompt:: bash #

    ceph config show osd.N osd_mclock_max_capacity_iops_[hdd, ssd]

For example, the following command shows the max capacity for "osd.0" on a Ceph
node whose underlying device type is SSD:

  .. prompt:: bash #

    ceph config show osd.0 osd_mclock_max_capacity_iops_ssd


Steps to Manually Benchmark an OSD (Optional)
=============================================

.. note:: These steps are only necessary if you want to override the OSD
          capacity already determined automatically during OSD initialization.
          Otherwise, you may skip this section entirely.

.. tip:: If you have already determined the benchmark data and wish to manually
         override the max osd capacity for an OSD, you may skip to section
         `Specifying  Max OSD Capacity`_.


Any existing benchmarking tool can be used for this purpose. In this case, the
steps use the *Ceph OSD Bench* command described in the next section. Regardless
of the tool/command used, the steps outlined further below remain the same.

As already described in the :ref:`dmclock-qos` section, the number of
shards and the bluestore's throttle parameters have an impact on the mclock op
queues. Therefore, it is critical to set these values carefully in order to
maximize the impact of the mclock scheduler.

:Number of Operational Shards:
  We recommend using the default number of shards as defined by the
  configuration options ``osd_op_num_shards``, ``osd_op_num_shards_hdd``, and
  ``osd_op_num_shards_ssd``. In general, a lower number of shards will increase
  the impact of the mclock queues.

:Bluestore Throttle Parameters:
  We recommend using the default values as defined by
  :confval:`bluestore_throttle_bytes` and
  :confval:`bluestore_throttle_deferred_bytes`. But these parameters may also be
  determined during the benchmarking phase as described below.

OSD Bench Command Syntax
------------------------

The :ref:`osd-subsystem` section describes the OSD bench command. The syntax
used for benchmarking is shown below :

.. prompt:: bash #

  ceph tell osd.N bench [TOTAL_BYTES] [BYTES_PER_WRITE] [OBJ_SIZE] [NUM_OBJS]

where,

* ``TOTAL_BYTES``: Total number of bytes to write
* ``BYTES_PER_WRITE``: Block size per write
* ``OBJ_SIZE``: Bytes per object
* ``NUM_OBJS``: Number of objects to write

Benchmarking Test Steps Using OSD Bench
---------------------------------------

The steps below use the default shards and detail the steps used to determine
the correct bluestore throttle values (optional).

#. Bring up your Ceph cluster and login to the Ceph node hosting the OSDs that
   you wish to benchmark.
#. Run a simple 4KiB random write workload on an OSD using the following
   commands:

   .. note:: Note that before running the test, caches must be cleared to get an
             accurate measurement.

   For example, if you are running the benchmark test on osd.0, run the following
   commands:

   .. prompt:: bash #

     ceph tell osd.0 cache drop

   .. prompt:: bash #

     ceph tell osd.0 bench 12288000 4096 4194304 100

#. Note the overall throughput(IOPS) obtained from the output of the osd bench
   command. This value is the baseline throughput(IOPS) when the default
   bluestore throttle options are in effect.
#. If the intent is to determine the bluestore throttle values for your
   environment, then set the two options, :confval:`bluestore_throttle_bytes`
   and :confval:`bluestore_throttle_deferred_bytes` to 32 KiB(32768 Bytes) each
   to begin with. Otherwise, you may skip to the next section.
#. Run the 4KiB random write test as before using OSD bench.
#. Note the overall throughput from the output and compare the value
   against the baseline throughput recorded in step 3.
#. If the throughput doesn't match with the baseline, increment the bluestore
   throttle options by 2x and repeat steps 5 through 7 until the obtained
   throughput is very close to the baseline value.

For example, during benchmarking on a machine with NVMe SSDs, a value of 256 KiB
for both bluestore throttle and deferred bytes was determined to maximize the
impact of mclock. For HDDs, the corresponding value was 40 MiB, where the
overall throughput was roughly equal to the baseline throughput. Note that in
general for HDDs, the bluestore throttle values are expected to be higher when
compared to SSDs.


Specifying  Max OSD Capacity
----------------------------

The steps in this section may be performed only if you want to override the
max osd capacity automatically set during OSD initialization. The option
``osd_mclock_max_capacity_iops_[hdd, ssd]`` for an OSD can be set by running the
following command:

  .. prompt:: bash #

     ceph config set osd.N osd_mclock_max_capacity_iops_[hdd,ssd] <value>

For example, the following command sets the max capacity for a specific OSD
(say "osd.0") whose underlying device type is HDD to 350 IOPS:

  .. prompt:: bash #

    ceph config set osd.0 osd_mclock_max_capacity_iops_hdd 350

Alternatively, you may specify the max capacity for OSDs within the Ceph
configuration file under the respective [osd.N] section. See
:ref:`ceph-conf-settings` for more details.


.. index:: mclock; config settings

mClock Config Options
=====================

.. confval:: osd_mclock_profile
.. confval:: osd_mclock_max_capacity_iops_hdd
.. confval:: osd_mclock_max_capacity_iops_ssd
.. confval:: osd_mclock_cost_per_io_usec
.. confval:: osd_mclock_cost_per_io_usec_hdd
.. confval:: osd_mclock_cost_per_io_usec_ssd
.. confval:: osd_mclock_cost_per_byte_usec
.. confval:: osd_mclock_cost_per_byte_usec_hdd
.. confval:: osd_mclock_cost_per_byte_usec_ssd
.. confval:: osd_mclock_force_run_benchmark_on_init
.. confval:: osd_mclock_skip_benchmark

.. _the dmClock algorithm: https://www.usenix.org/legacy/event/osdi10/tech/full_papers/Gulati.pdf
