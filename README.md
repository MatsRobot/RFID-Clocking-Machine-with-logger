# 🕒 Portable RFID Clocking Machine & Logger

**RFID Logger** is a high-reliability, low-power portable solution designed for time-attendance tracking and race waypoint monitoring. Utilizing 125KHz RFID technology, it provides a tamper-proof method for logging personnel or participant movements in real-time.



---

<table>
  <tr>
    <td width="60%">
      <h2>🚀 The Backstory</h2>
      <p>
        Originally developed for a care agency, this project provides a tamper-proof method of logging attendance at service users' homes. By placing an RFID tag at a location, workers "clock in" to verify the exact date and time of their visit, ensuring accountability and safety.
      </p>
      <p>
        Beyond home care, the device is perfectly suited for <b>bicycle races or orienteering</b>. Participants carry the tags, and the device acts as a checkpoint logger, tracking exact arrival times and participant health (battery/temp) at various physical waypoints.
      </p>
    </td>
    <td width="40%">
      <img src="https://github.com/user-attachments/assets/79eed358-6a81-447c-b9f7-1024a7ddc5df" alt="RFID Clocking Machine Main View" />
    </td>
  </tr>
</table>

---

## ✨ Key Features & Technical Specs

* **Tamper-Proof Logging:** Records RFID UID, Precision Timestamp, Temperature, and Battery Voltage.
* **Universal Data Format:** Generates `.CSV` files directly to an SD card, compatible with Excel and Google Sheets.
* **Optimized for Portability:** Powered by 3x AAA batteries with a high-efficiency **MT3608 DC-DC boost converter** for a stable 5V rail.
* **Custom Visuals:** Features a bespoke 48x48 RFID logo and dynamic battery indicators on a low-power OLED.
* **High-Precision Timing:** Utilizes the **DS3231 RTC** for temperature-compensated timekeeping (±2ppm accuracy), ensuring long-term reliability.

## 🛠️ Hardware Stack

* **Microcontroller:** Arduino Nano (ATmega328P).
* **RFID Reader:** RDM6300 (125KHz UART).
* **Real-Time Clock:** DS3231 (I2C) with integrated TCXO.
* **Storage:** Full-size SD Card Module (optimized for 2GB cards, FAT16/32).
* **Display:** 0.96" SSD1306 OLED (I2C).
* **Power:** 3x AAA via DC-DC Step-Up Converter.

---

## 📐 Engineering Challenges & Solutions

### 1. Memory Management (Zero-Heap Optimization)
The Arduino Nano provides only 2KB of SRAM. Standard `String` libraries cause heap fragmentation and crashes in long-running logs.
* **The Fix:** The entire codebase uses **Fixed-width Char Arrays (`char[]`)**. This ensures predictable memory allocation and prevents the system from hanging during 24+ hour deployments.

### 2. Timekeeping Accuracy
The initial prototype used the DS1307, which drifted significantly due to ambient temperature changes.
* **The Fix:** Swapped to the **DS3231**, which uses an integrated temperature-compensated crystal oscillator (TCXO). The "clock-in" time remains accurate to within seconds per month, even in outdoor race conditions.

### 3. Custom Graphics on OLED
To provide a professional UI without exhausting RAM with a full 1024-byte framebuffer:
* **The Fix:** I modified nine characters in the 8x8 font set. Using the `LogoMaker-48x48 - RFID.xlsx` spreadsheet, I mapped out bits and injected them into the `MatsRobotFont8x8` directory to render a large, seamless graphic.

---

## 📊 Data Output

Logs are stored in a simple, easy-to-read CSV format:



<p align="center">
  <img width="600" src="https://github.com/user-attachments/assets/905b70bd-ff58-4378-9158-a06ad05413fb" alt="CSV Data Output Example" />
</p>

## 📂 Repository Structure

* **`/Firmware`**: Arduino `.ino` files (Requires `MatsRobotFont8x8` library).
* **`/Libraries`**: Custom font sets and the `LogoMaker` bit-mapping spreadsheet.
* **`/Hardware`**: Wiring schematics, internal photos, and enclosure suggestions.

<p align="center">
  <img width="200" src="https://github.com/user-attachments/assets/a26205e3-3552-4827-8e55-8889f97cc5c1" alt="Device Internal 1" />
  <img width="200" src="https://github.com/user-attachments/assets/33001cb0-82e6-49d3-aacd-ae88e653e0f6" alt="Device Internal 2" />
</p>

---

<small>© 2026 MatsRobot | Licensed under the [MIT License](https://github.com/MatsRobot/matsrobot.github.io/blob/main/LICENSE)</small>
