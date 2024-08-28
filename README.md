# Portable-RFID-Clocking-Machine-with-logger
Clocking machine using RFID tag to log to confirm data and time of clocking on an SD card 

![20240828_064613](https://github.com/user-attachments/assets/79eed358-6a81-447c-b9f7-1024a7ddc5df)

This portable logger was initially developed for a care agency to log the attendance of the care workers at service user’s house.
The portable logger could also be used to track time on bicycle races where it could track positions at different stops where RFID tags could be placed at these locations.

The date is in a CSV file format which could be opened with any Office software. 

![Screenshot 2024-08-28 083508](https://github.com/user-attachments/assets/905b70bd-ff58-4378-9158-a06ad05413fb)

![20240828_062618 - Copy](https://github.com/user-attachments/assets/a26205e3-3552-4827-8e55-8889f97cc5c1)
![20240828_062618](https://github.com/user-attachments/assets/33001cb0-82e6-49d3-aacd-ae88e653e0f6)

The design is simple, however I hit the limitations of Arduino with the allocation of variables, so the software got more complicated than necessary as all String variables had to be replaced with Char Arrays which is more difficult to work with.

An Arduino Nano is used as the base processor.

As this processor has not got a real time clock, initially DS1307 was used as the clock, however it was found that this module is not keeping accurate time and later was replaced with DS3231 which compensate for temperature to provide a more stable time. The logger also logs the temperature and the battery voltage for further accuracy analysis.

125KHz RFID reader RDM6300 was used as it could be purchased for less that £2. 

The full size SD card reader was used as the limitation appears to be 2GB, so any small SD or micro SD with the adapter could be used.  I must admit getting hold of 2GB cards proved difficult, however they are still available to purchase.

3 AAA battery is used as they fit comfortably in the used case. A DC to DC converter is used to provide a steady 5V supply to the circuit from either 2 AA or 3 AAA supply batteries.

The initial design used an LCD display, however they use a lot of battery and the final design was based on an OLED.
Working with OLED allowed the ability to create RFID Logo, Battery sign and other characters such as the degree sign that is not available in the standard character set. 

3x3 characters (1-9) of the 8x8 character font sets were changed to represent the logo. Spreadsheet template was used to calculate the required values for each of the nine characters.
