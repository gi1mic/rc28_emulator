
# **Icom RC-28 Emulator**
This is a work in progress

Copyright GI1MIC (2021)

**   For non-commercial use only**

<img src="https://github.com/gi1mic/rc28_emulator/blob/main/images/RC-28_Emulator.jpg?raw=true" width=30%>

   
##    **Required Hardware:**
   
-    3 x Arduino Pro Mini (32u4, 16Mhz, 5V version)
-    3 x momentary push-to-make buttons to act as TX, F1 and F2
-    3 x LED's to show TX, F1, F2 status
-    1 x Optical encoder (5V-24V 600P/R with 6mm shaft)
-    1 x ~30mm knob for 6mm shaft preferable a machined aluminum for weight
-    1 x enclosure of choice

##   **Installation:**
  
  This project requires modification of the standard Arduino USB configuration files. For this reason it is recommend you setup a standalone version of the Arduino IDE. This will ensure you do not interfere with any other Arduino projects you may have.
  
  To do this under Windows  download the ZIP file version of the Arduino IDE and extract it into a directory. Within this directory create another folder called "portable". Double click on the arduino.exe file you just extracted and it will create a directory structure under the portable directory you just created.
  
  At this point you now have a standalone (portable) version of the Arduino IDE where all library files and sketches are held in the portable directory.
  
  Next install the "HID-Project" library by NicoHood (https://github.com/NicoHood/HID) and the "Rotaryencoder" library by mathertel (https://github.com/mathertel/RotaryEncoder). Both are available via the standard Arduino library manager.
  
  Now download the provided source file and place it under the portable folder in "portable\sketchbook\rc28_emulator". At this point, if you select "Arduino Leonardo" as the target platform you should be able to compile the code as a quick test.
  
  If all is well exit the IDE and add the following to hardware\arduino\avr\boards.txt (just after the Leonardo section) to create a "new" board variation that reports the necessary device name and USB VID/PID

	  //##############################################################
	  leonardo.name=Arduino Leonardo - RC-28
	  leonardo.vid.0=0x2341
	  leonardo.pid.0=0x0036
	  leonardo.vid.1=0x2341
	  leonardo.pid.1=0x8036
	  leonardo.vid.2=0x2A03
	  leonardo.pid.2=0x0036
	  leonardo.vid.3=0x2A03
	  leonardo.pid.3=0x8036
  
	  leonardo.upload.tool=avrdude
	  leonardo.upload.protocol=avr109
	  leonardo.upload.maximum_size=28672
	  leonardo.upload.maximum_data_size=2560
	  leonardo.upload.speed=57600
	  leonardo.upload.disable_flushing=true
	  leonardo.upload.use_1200bps_touch=true
	  leonardo.upload.wait_for_upload_port=true
  
	  leonardo.bootloader.tool=avrdude
	  leonardo.bootloader.low_fuses=0xff
	  leonardo.bootloader.high_fuses=0xd8
	  leonardo.bootloader.extended_fuses=0xcb
	  leonardo.bootloader.file=caterina/Caterina-Leonardo.hex
	  leonardo.bootloader.unlock_bits=0x3F
	  leonardo.bootloader.lock_bits=0x2F
  
	  leonardo.build.mcu=atmega32u4
	  leonardo.build.f_cpu=16000000L
	  leonardo.build.vid=0x0C26
	  leonardo.build.pid=0x001E
	  leonardo.build.usb_product="Icom RC-28 REMOTE ENCODER"
	  leonardo.build.board=AVR_LEONARDO
	  leonardo.build.core=arduino
	  leonardo.build.variant=leonardo
	  leonardo.build.extra_flags={build.usb_flags}


  Now modify hardware\arduino\avr\cores\arduino\USBcore.cpp and change the #if section at the top of the file to the following:

	  #if USB_VID == 0x2341
	  #  if defined(USB_MANUFACTURER)
	  #    undef USB_MANUFACTURER
	  #  endif
	  #  define USB_MANUFACTURER "Arduino LLC"
	  #elif USB_VID == 0x1b4f
	  #  if defined(USB_MANUFACTURER)
	  #    undef USB_MANUFACTURER
	  #  endif
	  #  define USB_MANUFACTURER "SparkFun"
	  #elif USB_VID == 0x0C26
	  #  if defined(USB_MANUFACTURER)
	  #    undef USB_MANUFACTURER
	  #  endif
	  #  define USB_MANUFACTURER "Icom Inc."
	  #elif !defined(USB_MANUFACTURER)
	  // Fall through to unknown if no manufacturer name was provided in a macro
	  #  define USB_MANUFACTURER "Unknown"
	  #endif


  further down the file around line 536 change
`      return USB_SendStringDescriptor((uint8_t*)name, strlen(name), 0);`
  to
`      return USB_SendStringDescriptor((uint8_t*)"RC-28 0102001", strlen("RC-28 0102001"), 0);`

Going by the manual the serial number should be in the format "02XXXXX" but I do not think it is checked.

If you restart the Arduino IDE you should now have a new custom board type called "Arduino Leonardo - RC-28".
	Select this and upload the result to a standard Adruino Pro Micro.
	
You can always select "Arduino Leonardo - ETH" as the target if you want to re-use the board on another project.


##   **Wiring:**

 - VCC Encoder red 
 - PIN2 Encoder White 
 - PIN3 Encoder Green 
 - GND Encoder Black
   
 - PIN8 F1 Button
 - PIN9 F1 LED
 - PIN4 F2 Button
 - PIN 5F2 LED
 - PIN 6 TX Button
 - PIN7 TX LED

Other side of the buttons and LED's go to GND

##   **Notes:**
 - This is not a true emulation of the RC-28 as the arduino is configured to emulate both a HID and USB serial device simultaneously. 
 - The RC-28 sends it reports via USB Interrupt messages but straight HID messages seem to work just fine which is what I am using.
 - Not sure I have the response bit patterns 100% but without a RC-28 I cannot verify them other than testing them with RS-BA1.
 - I do not know if this will work with the IC-7610 but should you try it please let me know what happens.
 - A big thank you Philippe for his help in this project. Without his help I would never have got this to work.
