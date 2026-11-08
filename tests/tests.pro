# Fork changes (2026-11-08): Windows build or CAN/Tx Generator test integration.
# See docs/windows-development_eng.md for the change record.
TEMPLATE = subdirs

# Unit tests. Build and run everything with:
#   cd tests && qmake6 && make -j$(nproc) && make check
SUBDIRS += \
    bus_message_signal \
    bus_message_frame \
    can_db_signal \
    dbc_parser \
    can_db \
    trace_file_format \
    trace_line_format \
    pcapng_format \
    trace_file_writer \
    trace_recorder \
    decoders \
    autosar_e2e \
    ldf_parser \
    slcan_codec \
    ulc_codec \
    measurement_setup

win32: SUBDIRS += candle_timing
SUBDIRS += vector_timing
SUBDIRS += tx_generator_list
SUBDIRS += tx_live_edit
SUBDIRS += tx_signal_list
