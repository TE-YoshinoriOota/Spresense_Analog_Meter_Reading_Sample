# Spresense_Analog_Meter_Reading_Sample
Spresense Analog Meter Reading Sample using Neural Network Console

## Demo
The demo video below shows measuring temperature and humidity by capturing the thermo-hygrometer. The camera system has two buttons. The upper side button is to measure the temperature meter. The lower side button is to measure the humidity meter.

![Analog Meter Reading](https://github.com/TE-YoshinoriOota/Spresense_Analog_Meter_Reading_Sample/assets/14106176/b97fc4ec-b966-42e5-bde1-1f310c970aff)


## How it works
The recognition step has three steps.

(1) Recognition of axial centers of meters 
Capture the thermo-hygrometer and recognize the blue dot or the green dot mark on the thermo-hygrometer. The blue dot must be pasted at the axial center of the temperature meter and the green dot must be pasted at the axial center of the hygrometer. The recognition method is done by binary semantic segmentation that can recognize objects having characteristics of color and shape specified by the dataset.

(2) Calibration of the axial centers
In many cases, the measuring of the center may have errors. The true center of the black meter needle has the lowest value when scanning clockwise around the centers. The calibration is to gather the values by shifting centers around the needle axial center found by AI to find the appropriate center.

(3) Scan the needle area clockwise to find the needle position
After finding the appropriate center, scan the needle area clockwise. When the black meter needle, the lowest value is the needle position. Lastly, map the degree of the needle position to physical value.

## The project tree
| Directory | Contents | Details |
|-----|-----|-----|
| blue-dot | Neural Network Console project for detecting blue-dots | Include datasets and generator script of the datasets |
|green-dot | Neural Network Console project for detecting green-dots | Include datasets and generator script of the datasets |
|semaseg_camera_analog_meter | Sketch of the analog meter reading for Spresense | Need Spresense Caemera, LCD and buttons |
