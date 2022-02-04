# Project name: MedButton_2.0

### Semester project of Principles of Computer Organization course

# Project aim:
In the battlefield, nearly one fifth of combatants die because of abscence of medical help. Two main causes of death are bleeding (is lethal for half of the wounded soldiers) and pneumothorax. Both these causes could be eliminated or at least minimized if there was a way to provide in-time help to soldiers in the field. Medical Button 2.0 is the second version of MedButton project - a device which allows combat medics to see where they are needed on the battlefield and help the wounded in a more secure and faster way.

# General requirements
Key requirements to the device are as follows:
- secure data transfer
- minor power consumption of the device
- a fuse (does not allow a soldier to activate the device accidentally)
- secure wounded soldier identification (to minimize the call of medics by enemies) 
- convenient device location

# Project Architecture:
<img src="https://user-images.githubusercontent.com/70766505/149573640-c482787c-71df-4347-9f59-c3a62ad88cb3.png" width=75% height=75%>


# First Prototype
<img src="https://user-images.githubusercontent.com/70766505/149617270-de258abd-31c2-479a-ba21-adbb4920f9cd.jpeg" width=50% height=50%>
<img src="https://user-images.githubusercontent.com/70766505/149617271-cfdb6521-735a-4758-8e96-0d2fa5efc60e.jpeg" width=50% height=50%>

# What's inside
<img src="https://user-images.githubusercontent.com/70766505/149617504-5f2a9ef6-6e56-40f7-acc4-ee41a6902e45.jpeg" width=70% height=70%>


# Modules:
Currently, MedButton is based on [LoRaWAN Onethinx Core Module](https://www.onethinx.com/module.html) 
microcontroller:</br>
<img src="https://user-images.githubusercontent.com/70766505/149617739-9bc14a93-2d03-4c54-b18c-26b3fb32dc07.jpg" width=40% height=40%>


Data is gathered with the help of Ublox NEO-6M GPS module:</br>

<img src="https://user-images.githubusercontent.com/57792587/104818846-aed90780-5832-11eb-8680-ed09a42f007a.png" width=45% height=45%>


A message, composed of the processed data and timestamps, is transferred to an operator PC using [LoRaWAN Onethinx Core Module](https://www.onethinx.com/module.html). In case of any interruptions in a communication line, we are using also [GSM/GPRS SIM900A](https://www.itead.cc/sim900-sim900a-gsm-gprs-minimum-system-module.html):

<img src="https://user-images.githubusercontent.com/57792587/104819286-2b6ce580-5835-11eb-83d5-5dda4e13de9a.png" width=30% height=30%>
<img src="https://user-images.githubusercontent.com/57792587/104819149-425f0800-5834-11eb-9384-cc11adebe060.png" width=30% height=30%>

If, after switching to GSM module, the transfer is still unsuccessful, the device tries to send the message via LoRa again. The switching continues until the message is sent to the medic's PC.

As a button, we use Joystick for Arduino:

![81772579-7fcc0280-94fb-11ea-941a-461bd82a1822](https://user-images.githubusercontent.com/70766505/149573321-62e244ec-0380-4ab2-a78d-8bfc87e9ada4.jpg)



# Working progress:
- Connected GPS and receive data in raw NMEA (National Marine Electronics Association) format;
- Extracting GPGGA sentences: latitude / longitude and time;
- Parsing data in appropriate format;
- Implemented AES encryption/decryption of the message transmitted via GPRS;
- Implemented the algorithms of waking up GPRS and LoRa tasks accorrding to their priority using Free RTOS;
- Connected LoRa Onethink module to transfer data with it.

# Future plans
Future developments can be done is several main spheres:
#### Security & reliability
- Implement CBC AES algorithm to make the encryption even more secure;
- Add a security cap to the button's box to prevent accidental activation + replace joystick with comfortable and safe button;
- Fix all movable modules and wires inside the box to prevent disassembly while the soldier is moving.

#### Power supply
To power GPRS with 5V, we use a separate PSoC 6 MCU. Apparently, it needs to be replaced with another source of power (e.g. a battery).

#### Interface
- Write a convenient application to decrypt all incoming messages on the medics' PC (those coming from LoRa and from GPRS).

#### New funcitonality & features
- Connect temperature and/or pulse sensor(s) to transfer additional data about a soldier's condition.

#### Future development
- Perform first tests on the poligons;
- Think of collaboration with existing initiatives for combat support in Ukraine.

# Repository structure
The project's main code for programming and debugging can be found in the `firmware/MedButton` folder.

# Contributors:
- Bohdan Yaremkiv
- Maksym Maystrenko
- Oleg Farenyuk
- [Anna Korabliova](https://github.com/anika02)
- [Diana Hromyak](https://github.com/Diana-Doe)
- [Natalia Romanyshyn](https://github.com/romanyshyn-natalia)
- [Alina Muliak](https://github.com/alinamuliak)
- [Oleksandra Stasiuk](https://github.com/oleksadobush)
- [Vira Saliieva](https://github.com/vsaliievaa)
