# esp-surveillance-cam

A hardware-backed audio and video recording system built on the ESP32-S3 microcontroller. The system is optimized for continuous motion-triggered capture with dedicated power-loss protection to ensure filesystem stability

---

## Technical Architecture

```
                       +-------------------+
                       | Main Power (5V)   |
                       +---------+---------+
                                 |
                                 v
  +------------------+     +-----+-------------+     +-------------------+
  | Voltage Divider  |<----+   TP4056 Module   +---->| 18650 Li-ion Bat  |
  | (ADC Power Loss) |     +-----+-------------+     +-------------------+
  +--------+---------+           |
           |                     | (VOUT)
           | (GPIO Interrupt)    v
           |               +-----+-------------+       (Wi-Fi)       +--------------+
           +-------------->| ESP32-S3 (N16R8)  |<------------------->| Telegram Bot |
                           +--+-------------+--+                     +--------------+
                              |             |
            (1-Bit SDMMC)     v             v  (I2S / DVP)
                     +--------+-----+     +-+---------------+
                     | 128GB TF Card|     | OV2640 Camera   |
                     | (FAT32)      |     | MS3625 MEMS Mic |
                     +--------------+     +-----------------+

```

---

## Core Engineering Features

### High-Bandwidth I/O Optimization

To capture synchronized video and audio data, this project utilizes the ESP32-S3 native 1-bit SDMMC peripheral connected to the onboard microSD slot

### Hardware-Driven Data Integrity

FAT32 filesystems are highly susceptible to corruption if power is abruptly disconnected while file tables are updating. This system implements a hardware-and-software "Panic Save" routine:

* **Hardware:** A TP4056 charging circuit paired with an 18650 Li-ion cell provides a seamless power transition during a blackout. An analog voltage divider monitors the primary 5V rail before the charging circuit
* **Software:** The voltage drop triggers a low-latency ADC interrupt on the ESP32-S3. The firmware immediately stops peripheral streams, flushes remaining buffers using `f_sync()`, and closes file handles safely before alerting the administrator

### Memory Management & Video Containerization

The firmware leverages the board's 8MB External PSRAM to queue compressed JPEG frames and raw PCM audio packets asynchronously

---

## File System & Storage Allocation

The system utilizes a rolling retention loop designed to retain 5 to 7 days of historical recordings based on motion activation

* **Container Format:** Audio and video streams are multiplexed and stored natively as `.avi` files
* **Naming Convention:** Files use a chronological timestamp format: `<startdate>_<enddate>.avi`
* **Audio Bitrate:** 16kHz mono (16-bit) = **31.25 KB/s**
* **Video Target:** SVGA (800x600) @ 10 fps = **~1 MB/s** (variable based on compression)
* **Total Throughput:** ~4 GB of storage consumed per hour of active recording. A 128GB capacity supports up to ~5 hours of active motion-triggered footage per day across a rolling 7-day loop

---

## Remote Interface (Telegram Bot)

The system integrates an asynchronous network layer utilizing Wi-Fi to expose a low-overhead Telegram Bot interface for remote asset retrieval.

* **On-Demand Polling:** Instead of streaming continuously over Wi-Fi (which degrades performance and consumes battery bandwidth), the camera remains silent and acts on discrete string queries from the user.
* **Query and Delivery:** The user can prompt the bot for a specific timeframe. The firmware parses the requested index, locates the corresponding `<startdate>_<enddate>.avi` file on the local storage partition, and uploads it directly to the chat interface as a single video file

---

## Hardware Component Selection

* **Microcontroller:** ESP32-S3 WROOM (N16R8: 16MB Flash, 8MB PSRAM)
* **Camera Sensor:** OV2640 with a 160° wide-angle lens 
* **Audio Input:** MS3625 MEMS Microphone via I2S interface
* **Storage:** 128GB microSDXC (formatted to FAT32)
* **Power Management:** TP4056 Protection Module + 18650 Li-Ion cell