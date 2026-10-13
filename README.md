# STAAM — Sensor Telemetry & Active Alarm Manager

A C99 console project implementing the core requirements in the Cisco Optical Apprentice C Systems Programming assessment.

## Build

```sh
make
```

Or compile exactly as requested by the assessment:

```sh
gcc -Wall -Wextra -Werror -std=c99 -pedantic *.c -o staam
```

## Run

Interactive:

```sh
./staam
```

Run the supplied sample input:

```sh
./staam < sample_input.txt
```

Or use:

```sh
make test
```

## Commands

- `FEED <id> <type> <value> <timestamp>` — submit a sensor reading.
- `SHOW ALARMS` — print active alarms by priority.
- `CLEAR <id>` — manually clear an alarm.
- `STATS` — show sample count, active alarms, and tick span.
- `EXIT` — free resources and exit.

Sensor type IDs: `1` temperature (°C), `2` optical receive power (dBm), `3` voltage (V), `4` fan speed (RPM).

## Architecture

- `types.h`: shared enums and alarm node/list structures.
- `alarm_list.h/.c`: doubly linked alarm list and memory management.
- `telemetry.h/.c`: thresholds, per-sensor debounce state, alarm lifecycle.
- `main.c`: input parser and command loop.

## Memory check

```sh
valgrind --leak-check=full --show-leak-kinds=all ./staam < sample_input.txt
```

## Notes

This is an educational implementation of the assessment brief. Review and test it before submission. In particular, confirm any ambiguous policy choices with the assessor: the brief says readings outside the normal range are abnormal, while its listed alarm thresholds primarily specify high-temperature/low-optical/low-fan cases. This implementation treats temperature below 20°C, optical power above -3 dBm, voltage outside range, and fan speed above 8000 RPM as minor abnormal conditions. It also uses the timestamp difference as the requested tick-span statistic.
