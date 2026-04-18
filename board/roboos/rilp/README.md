# RILP - RoboOS Inter-Layer Protocol

## Overview

RILP is a lightweight, real-time capable inter-layer communication protocol for RoboOS. It provides a unified message format that works across all six layers of the RoboOS architecture.

## Protocol Format

Each RILP message has a 16-byte header followed by an optional payload:

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 1 | version | Protocol version (0x01) |
| 1 | 1 | src_layer | Source layer (1-6) |
| 2 | 1 | dst_layer | Destination layer (1-6, 0=broadcast) |
| 3 | 1 | msg_type | Message type (0x01-0x09, 0xFF) |
| 4 | 4 | seq_id | Sequence number (network byte order) |
| 8 | 4 | deadline_us | Latency budget in microseconds |
| 12 | 4 | payload_len | Payload size in bytes |

## Message Types

| Value | Name | Description |
|-------|------|-------------|
| 0x01 | ANNOUNCE | Component registration |
| 0x02 | HEARTBEAT | Liveness signal |
| 0x03 | SENSOR_DATA | Sensor reading (HAL → Middleware) |
| 0x04 | ACTUATOR_CMD | Motor/servo command (Middleware → HAL) |
| 0x05 | AI_RESULT | Inference output (AI → Middleware) |
| 0x06 | SECURITY_ALERT | Policy violation |
| 0x07 | OTA_REQUEST | Firmware update trigger |
| 0x08 | FLEET_STATUS | Robot health to dashboard |
| 0x09 | DIAG_QUERY | Cross-layer diagnostic |
| 0xFF | EMERGENCY | Immediate safe-stop broadcast |

## Building

```bash
cd board/roboos/rilp
make
make install

#Testing RoboOS in QEMU

bash

cd /workspaces/home/buildroot

# Check if your images are still there
ls -la output/images/

# Boot RoboOS
./output/images/start-qemu.sh