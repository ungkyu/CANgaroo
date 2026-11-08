# CANgaroo Vector backend

Based on Qt Serial Bus **v6.12.0**, `src/plugins/canbus/vectorcan/`:
https://github.com/qt/qtserialbus/tree/v6.12.0/src/plugins/canbus/vectorcan

The four source files retain their upstream copyright and SPDX notices.
CANgaroo uses these files under the GPL-2.0-only alternative; the license is
available in the repository root `LICENSE`.

Local modifications (2026-11-08): renamed backend classes and logging category,
added nominal/data sample point configuration, applied Classic CAN timing with
`xlCanSetChannelParams` and CAN FD timing with `xlCanFdSetConfiguration` before
activation, rejected missing initialization access and inexact FD timing, and
removed the post-activation bitrate replay that would overwrite explicit timing.
The XL library remains a runtime dependency; no vendor SDK is needed to build.

This backend is compiled into the application. The installed Qt VectorCAN plugin
is not modified. Hardware validation is still required.
