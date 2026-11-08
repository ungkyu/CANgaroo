# CANgaroo Vector backend

Based on Qt Serial Bus **v6.12.0**, `src/plugins/canbus/vectorcan/`:
https://github.com/qt/qtserialbus/tree/v6.12.0/src/plugins/canbus/vectorcan

The four source files retain their upstream copyright and SPDX notices.
The upstream licensing alternatives remain intact. For the combined application
configuration using open-source Qt Charts, select the GPL-3.0-only alternative.
See `LICENSES/GPL-3.0-only.txt` at the repository root. The earlier README
statement selecting GPL-2.0-only was incomplete and is superseded by this notice.

Local modifications (2026-11-08): renamed backend classes and logging category,
added nominal/data sample point configuration, applied Classic CAN timing with
`xlCanSetChannelParams` and CAN FD timing with `xlCanFdSetConfiguration` before
activation, rejected missing initialization access and inexact FD timing, and
removed the post-activation bitrate replay that would overwrite explicit timing.
The XL library remains a runtime dependency; no vendor SDK is needed to build.

This backend is compiled into the application. The installed Qt VectorCAN plugin
is not modified. Hardware validation is still required.
