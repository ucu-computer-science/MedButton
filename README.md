# Project name: MedButton

### Semester project of Principles of Computer Organization course

# Project aim:
The task of combat medics is to provide first aid to the wounded soldiers as soon as possible. The device for calling combat medics with geolocation makes it possible to find the wounded in a minimum of time. 

# General requirements:

- secure data transferring
- small power consumption of the device
- fuse (does not allow a soldier to use the device accidentally)
- secure wounded soldier identification  (to minimize the call of medics by enemies) 
- convenient device allocation

# Project Architecture:
![button_scheme](https://user-images.githubusercontent.com/57792587/104817794-31aa9400-582c-11eb-9132-d88ef78cf1a9.jpg)


# Modules:
Currently, MedButton is based on [PSOC 6 BLE PIONEER KIT](https://www.cypress.com/documentation/development-kitsboards/psoc-6-ble-pioneer-kit-cy8ckit-062-ble) microcontroller:
![image](https://user-images.githubusercontent.com/57792587/104818743-2bb7b180-5832-11eb-9194-ea758e3505d8.png)


Data is located with Ublox NEO-6M GPS module:


![image](https://user-images.githubusercontent.com/57792587/104818846-aed90780-5832-11eb-8680-ed09a42f007a.png)


The processed data is transferred to a operator PC using [LoRaWAN Onethinx Core Module](https://www.onethinx.com/module.html), but in case of some interruptions in a communication line we are using also [GSM/GPRS SIM900A](https://www.itead.cc/sim900-sim900a-gsm-gprs-minimum-system-module.html):

![image](https://user-images.githubusercontent.com/57792587/104819286-2b6ce580-5835-11eb-83d5-5dda4e13de9a.png)

![image](https://user-images.githubusercontent.com/57792587/104819149-425f0800-5834-11eb-9384-cc11adebe060.png)

# Working progress:
- Connected GPS and receive data in raw NMEA(National Marine Electronics Association) format
- Extracting GPGLL sentences: Geographic Position, Latitude / Longitude and time 
- Parsing data in appropriate format
- Connected button to our MCU
- Transferring parsed data after pressing the button using the GPRS module
- Working on connecting LoRa Onethink module and transferring data with LoRa

# Future plans:
- Connect temperature and blood pressure sensors to transfer additional data
- Create an algorithm for unique identification of a soldier
- Think about fuse

# Contacts:
- [Nataliia Romanyshyn](https://github.com/romanyshyn-natalia)
- [Diana Hromyak](https://github.com/Diana-Doe)
- [Anna Korabliova](https://github.com/anika02)
