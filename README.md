# Project name: MedButton_2.0

### Semester project of Principles of Computer Organization course

# Project aim:
The task of combat medics is to provide first aid to the wounded soldiers as soon as possible. The device for calling combat medics with geolocation makes it possible to find the wounded in a minimum of time. 

# General requirements:

- secure data transferring
- small power consumption of the device
- fuse (does not allow a soldier to use the device accidentally)
- secure wounded soldier identification (to minimize the call of medics by enemies) 
- convenient device allocation

# Project Architecture:
![button_scheme](https://user-images.githubusercontent.com/70766505/149558098-0538d740-d670-4503-ae2c-2dadcc385b62.png)


# Modules:
Currently, MedButton is based on [LoRaWAN Onethinx Core Module](https://www.onethinx.com/module.html) 
microcontroller:
![image](https://user-images.githubusercontent.com/57792587/104819286-2b6ce580-5835-11eb-83d5-5dda4e13de9a.png)


Data is located with Ublox NEO-6M GPS module:

![image](https://user-images.githubusercontent.com/57792587/104818846-aed90780-5832-11eb-8680-ed09a42f007a.png)


The processed data is transferred to a operator PC using [LoRaWAN Onethinx Core Module](https://www.onethinx.com/module.html), but in case of some interruptions in a communication line we are using also [GSM/GPRS SIM900A](https://www.itead.cc/sim900-sim900a-gsm-gprs-minimum-system-module.html):

![image](https://user-images.githubusercontent.com/57792587/104819149-425f0800-5834-11eb-9384-cc11adebe060.png)

As a button we use Joystick for Arduino:
![arduino-joystick-arduino-5879-700x700](https://user-images.githubusercontent.com/70766505/149559588-d53efa7e-c2ad-497b-8512-b0b715c08ab8.jpg)

# Working progress:
- Connected GPS and receive data in raw NMEA(National Marine Electronics Association) format
- Extracting GPGGA sentences: latitude / longitude and time 
- Parsing data in appropriate format
- Implemented AES encryption/decryption of transmitted via GPRS message
- Transferring parsed data after pressing the button using the GPRS module
- Connected LoRa Onethink module to transfer data with it

# Future plans:
- Connect temperature sensor to transfer additional data about a soldier's condition
- Think of adding a security cap to the buttonn's box
- Think of collaboration with existing initiatives for combat support in Ukraine
- For GPRS we need 5V and we use it from PSoC6 WIFI BT Prototyping Kit, and that's its only usage, so we need to replace it.

# Contacts:
- [Diana Hromyak](https://github.com/Diana-Doe)
- [Alina Muliak](https://github.com/alinamuliak)
- [Oleksandra Stasiuk](https://github.com/oleksadobush)
- [Vira Saliieva](https://github.com/vsaliievaa)
