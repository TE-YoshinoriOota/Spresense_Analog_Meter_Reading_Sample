# Spresense_Analog_Meter_Reading_Sample
Spresense Analog Meter Reading Sample using Neural Network Console

## Demo
The demo video below shows measuring temperature and humidity by capturing the thermo-hygrometer. The camera system has two buttons. The upper side button is to measure the temperature meter. The lower side button is to measure the humidity meter.

https://github.com/TE-YoshinoriOota/Spresense_Analog_Meter_Reading_Sample/assets/14106176/b97fc4ec-b966-42e5-bde1-1f310c970aff


## How it works
The recognition process has three steps.

**(1) Recognition of axial centers of meters** <br/>
Capture the thermo-hygrometer and recognize the blue dot or the green dot mark on the thermo-hygrometer. The blue dot must be pasted at the axial center of the temperature meter and the green dot must be pasted at the axial center of the hygrometer. The recognition method is done by binary semantic segmentation that can recognize objects having characteristics of color and shape specified by the dataset.

**(2) Calibration of the axial centers** <br/>
In many cases, the measuring of the center may have errors. The true center of the black meter needle has the lowest value when scanning clockwise around the centers. The calibration is to gather the values by shifting centers around the needle axial center found by AI to find the appropriate center.

**(3) Scan the needle area clockwise to find the needle position** <br/>
After finding the appropriate center, scan the needle area clockwise. When the black meter needle, the lowest value is the needle position. Lastly, map the degree of the needle position to physical value.

## Repository Tree
### Repository directories
| Directory | Contents | Details |
|-----|-----|-----|
| blue-dot | Neural Network Console project for detecting blue-dots | Include datasets and generator script of the datasets |
|green-dot | Neural Network Console project for detecting green-dots | Include datasets and generator script of the datasets |
|semaseg_camera_analog_meter | Sketch of the analog meter reading for Spresense | Need Spresense Caemera, LCD and buttons |

### Configuration of Blue-dot or Green-dot directory
| Files or directories| Contents | 
|----|----|
| image | source input image for generating dataset | 
| mask | source output mask image for generating dataset | 
| background.jpg | background image for the source input images | 
| binary_semaseg_blue/green_dot.sdcproj | Neural Network Console project | 
| mksegdata.py | dataset generator script |
| train.zip | dataset for training generated by mksegdata.py | 
| valid.zip | dataset for validation generated by mksegdata.py | 

### Configuration of semaseg_camera_analog_meter
| Files | Contents | 
|----|----|
| semaseg_camera_analog_meter.ino | main sketch of this program | 
| region_detect.ino | detection of dot area (x,y,width,height) |
| displayUtil.ino | LCD display utilities |
| analog_meter_reading.h | Header file for analog_meter_reading.cpp |
| analog_meter_reading.cpp | Scanning algorithm for meter needles |

## Hardware Configuration
The system consists of Spresensee Main Board, Spresense LTE-M Extension Board, Spresense Camera Board, LCD, and Buttons. The connection is shown below.

![image](https://github.com/TE-YoshinoriOota/Spresense_Analog_Meter_Reading_Sample/assets/14106176/21d2d806-7cd3-4e36-be0b-8bf5d27424b1)





