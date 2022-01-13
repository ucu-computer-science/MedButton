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
- Extracting GPGGA sentences: latitude / longitude and time 
- Parsing data in appropriate format
- Implemented AES encryption/decryption of transmitted via GPRS message
- Transferring parsed data after pressing the button using the GPRS module
- Working on connecting LoRa Onethink module and transferring data with LoRa

# Future plans:
- Connect temperature sensor to transfer additional data about a soldier's condition
- Think of adding a security cap to the buttonn's box
- Think of collaboration with existing initiatives for combat support in Ukraine

# Contacts:
- [Diana Hromyak](https://github.com/Diana-Doe)
- [Alina Muliak](https://github.com/alinamuliak)
- [Oleksandra Stasiuk](https://github.com/oleksadobush)
- [Vira Saliieva](https://github.com/vsaliievaa)
