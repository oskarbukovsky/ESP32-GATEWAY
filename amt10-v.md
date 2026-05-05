

date09/12/2024
page
1 of 8
## SERIES: AMT10 | DESCRIPTION: MODULAR INCREMENTAL ENCODER
sameskydevices.com
## FEATURES
- patented capacitive ASIC technology
- low power consumption
- CMOS outputs
- 16 DIP switch selectable resolutions
- index pulse
- modular package design
- straight (radial) and right-angle (axial) versions
- 9 mounting hole options for radial version
- 8 mounting hole options for axial version
- -40~100°C operating temperature
## ELECTRICAL
parameterconditions/descriptionmintypmaxunits
power supplyVDD3.655.5V
current consumptionwith unloaded output6mA
output high level
## VDD-0.8V
output low level
## 0.4V
output current
CMOS sink/source per channel2mA
rise/fall time
## 30ns
## INCREMENTAL CHARACTERISTICS
parameterconditions/descriptionmintypmaxunits
channelsquadrature A, B, and X index
waveformCMOS voltage square wave
phase differenceA leads B for CCW rotation (viewed from front)90degrees
quadrature resolutions
## 1
## 48, 96, 100, 125, 192, 200, 250, 256, 384, 400, 500,
## 512, 800, 1000, 1024, 2048
## PPR
index
## 2
one pulse per 360 degree rotation
accuracy0.25degrees
quadrature duty cycle (at each
resolution)
## 256, 512, 1024, 2048
## 48, 96, 100, 125, 192, 200, 250, 384, 400, 500
## 800, 1000
## 49
## 47
## 43
## 50
## 50
## 50
## 51
## 53
## 56
## %
## %
## %
Notes: 1. Resolution selected via adjustable DIP switch, pre-set to 2048 PPR.  All resolutions are listed as pre-quadrature, meaning the final number of counts is PPR x 4.
- Some stepper motors may leak a magnetic field causing the AMT index pulse to not function properly (non-magnetic version available with 8 pulses per revolution).
CUI Devices is now Same Sky!    Learn More
## Additional Resources:    Product Page

SAME SKY | SERIES: AMT10 | DESCRIPTION: MODULAR INCREMENTAL ENCODERdate 09/12/2024 | page 2 of 8
sameskydevices.com
## WAVEFORMS
## X
## A
## B
## I
## P
## T
## S     S     S     S
Quadrature signals with index showing
counter-clockwise rotation
## Figure 1
## MECHANICAL
parameterconditions/descriptionmintypmaxunits
motor shaft length9mm
motor shaft toleranceNOM +0/-0.015mm
weight
## AMT102
## AMT103
## 20.5
## 14.0
g
g
axial play±0.3mm
rotational speed (at each
resolution)
## 192, 384, 400, 500, 800, 1000, 1024, 20487500RPM
## 48, 96, 100, 125, 200, 250, 256, 51215000RPM
## ENVIRONMENTAL
parameterconditions/descriptionmintypmaxunits
operating temperature
## 1
## -40100°C
humiditynon-condensing95%
vibration20~500 Hz, 1 hour on each XYZ10G
shock11 ms, ±XYZ direction50G
RoHSyes
Note: 1. Encoders with operating temperature of -40~125°C are available as a custom order
ParameterDescriptionExpressionUnitsNotes
PPRresolutionPulses Per Revolution
This is the user selected value and the format
all resolutions are listed in
CPRcountsPPR x 4Counts Per Revolution
This is the number of quadrature counts the
encoder has
Tperiod360/Rmechanical degrees
Ppulse widthT/ 2mechanical degrees
SA/B state widthT/4mechanical degreesThis is the width of a quadrature state
Iindex widthT/4mechanical degrees
The width of a once per turn index is the state
width for A & B lines
The following parameters are defined by the resolution selected for each encoder. The
encoders resolution is listed as Pulses Per Revolution (PPR), which is the number of
periods (or high pulses) over the encoders revolution.
Note:             For more information regarding PPR, CPR, or LPR (Lines Per Revolution) view  https://www.sameskydevices.com/blog/what-is-encoder-ppr-cpr-and-lpr
## Additional Resources:    Product Page

SAME SKY | SERIES: AMT10 | DESCRIPTION: MODULAR INCREMENTAL ENCODERdate 09/12/2024 | page 3 of 8
sameskydevices.com
## PART NUMBER KEY
For customers that prefer a specific AMT10 configuration, please reference the custom configuration key below.
## AMT10-V KITS
In order to provide maximum flexibility for our customers, the AMT10 series is provided in kit form standard.  This allows the user to implement the encoder into a range of
applications using one sku#, reducing engineering and inventory costs.
## (4.76mm)
## 3mm4m
mmm2mm5
3/16 inch1/4 inch
## (6.35mm)(3.175mm)
## 6mm8mm
1/8 inch
SnoweulB ykS thgiLegnarOelpruPyarGwolleYdeRGreenBlue
## SLEEVES
## 103
## BASE
## 103
## TOP COVER
## 102
## TOOL B
## 102
## TOP COVER
## SHAFT
## ADAPTER
## 102
## BASE
## TOOL A
## WIDE BASE
## AMT10X-V
## Orientation:
## 2 = Radial
## 3 = Axial
*See Mechanical Drawings
## ORDERING GUIDE
## AMT10X - X XXXX - X  XXXX - X
## Resolution
## 1
## (ppr):
## 0048        0384
## 0096        0400
## 0100         0500
## 0125          0512
## 0192          0800
## 0200        1000
## 0250        1024
## 0256         2048
## Resolution Option:
“blank” = fixed resolution
D = adjustable resolution
## Sleeve Bore Diameter:
2000 = 2 mm
3000 = 3 mm
3175 = 3.175 mm (1/8”)
4000 = 4 mm
4760 = 4.76 mm (3/16”)
5000 = 5 mm
6000 = 6 mm
6350 = 6.35 mm (1/4”)
8000 = 8 mm
## Base Number
## Orientation:
## 2 = Radial
## 3 = Axial
*See Mechanical Drawings
## Mounting Base:
## S = Standard
W = Wide (AMT102 only)
## Index Pulse:
I = single index pulse
per revolution
N = 8 index pulses
per revolution
Note: 1. Fixed resolutions are permanently set at this value; adjustable resolutions are preset via DIP switch to this value upon shipment.
## Additional Resources:    Product Page

SAME SKY | SERIES: AMT10 | DESCRIPTION: MODULAR INCREMENTAL ENCODERdate 09/12/2024 | page 4 of 8
sameskydevices.com
## ENCODER INTERFACE
## PINOUT CONNECTOR
## Function
## #AMT102AMT103
## BB CHANNELB CHANNEL
## 5V+5 V+5 V
## AA CHANNELA CHANNEL
## XINDEX CHANNELINDEX CHANNEL
## GGNDGND
## TUNUSEDN /A
## AMT102AMT103
## Mating Connector:
AMP 3-640440-5 (tin)
AMP 3-641237-5 (gold)
## Mating Connector:
## Molex 50-57-9405 Housing
## Molex 16-02-0086 Terminals
## RESOLUTION SETTINGS
## 4321
## (off) 0
## (on)1
Example setting: 500 PPR
DIP switch:
## 1 = On, 0 = Off
Resolution (PPR)Maximum RPM1234
## 204875000000
## 102475000010
## 100075001000
## 80075000100
## 512150000001
## 50075001010
## 40075000110
## 38475001100
## 256150000011
## 250150001001
## 200150000101
## 19275001110
## 125150001011
## 100150000111
## 96150001101
## 48150001111
## Additional Resources:    Product Page

SAME SKY | SERIES: AMT10 | DESCRIPTION: MODULAR INCREMENTAL ENCODERdate 09/12/2024 | page 5 of 8
sameskydevices.com
## 15.340.604
## 9.000.354
## 43.381.708
## 52.502.067
## 15.300.602R0.61015.50
## 28.771.133
## MECHANICAL DRAWING
units: mm[inch]
tolerance: ±0.1mm
units: mm[inch]
tolerance: ±0.1mm
## AMT102
## AMT102 WIDE BASE
## 0.610R
## 15.50
## 15.300.602
## 28.771.133
## 15.340.604
## 9.000.354
## 43.381.708
units: mm[inch]
tolerance: ±0.1mm
## AMT103
## 0.488
## 15.300.602
## 0.269
## 1.12628.60
## 12.396.83
## R15.500.610
## 15.340.604
## 9.000.354
## 1.34734.21
## 11.500.453
## Additional Resources:    Product Page

SAME SKY | SERIES: AMT10 | DESCRIPTION: MODULAR INCREMENTAL ENCODERdate 09/12/2024 | page 6 of 8
sameskydevices.com
## MECHANICAL DRAWING (CONTINUED)
units: mm[inch]
tolerance: ±0.1mm
## MOUNTING HOLE PATTERNS
## R1.050.041
## .280.022
## DETAIL A
## SCALE 4 : 1
## 25.401.000
## 16.000.630
## 22.000.866
## 0.067
## (2 PLCS)
## 19.050.750
## 1.70
## 20.900.823
## 0.116
## 2.10
## 0.825
## 3.96
## 2.95
## (4 PLCS)
## (2 PLCS)
## 20.96
## 0.156
## 0.083
## (2 PLCS)
## 0.620
## 15.75
## 12.600.496
## A (3 PLCS)
## 1.09
## 1.09
## 1.09
## 1.09
## AMT102
units: mm[inch]
tolerance: ±0.1mm
## 0.118
## 3.000
## (2 PLCS)
## 1.27732.430
## 1.81246.025
## 2.8700.113
## (2 PLCS)
## AMT102 WIDE BASE
units: mm[inch]
tolerance: ±0.1mm
## R1.050.041
## .280.011
## DETAIL A
## SCALE 4 : 1
## 20.900.823
## 22.000.866
## 25.401.000
## 19.050.750
## 0.49612.60
## (4 PLCS)
## 0.079
## 1.700.067
## (2 PLCS)
## 2.950.116
## (2 PLCS)
## 2.00
## 16.000.630
## A (3 PLCS)
## AMT103
## Additional Resources:    Product Page

SAME SKY | SERIES: AMT10 | DESCRIPTION: MODULAR INCREMENTAL ENCODERdate 09/12/2024 | page 7 of 8
sameskydevices.com
## ASSEMBLY PROCEDURE
## Additional Resources:    Product Page

SAME SKY | SERIES: AMT10 | DESCRIPTION: MODULAR INCREMENTAL ENCODERdate 09/12/2024 | page 8 of 8
Same Sky offers a one (1) year limited warranty.  Complete warranty information is listed on our website.
Same Sky reserves the right to make changes to the product at any time without notice.  Information provided by Same Sky is believed to be accurate and
reliable.  However, no responsibility is assumed by Same Sky for its use, nor for any infringements of patents or other rights of third parties which may result
from its use.
Same Sky products are not authorized or warranted for use as critical components in equipment that requires an extremely high level of reliability.  A critical
component is any component of a life support device or system whose failure to perform can be reasonably expected to cause the failure of the life support
device or system, or to affect its safety or effectiveness.
sameskydevices.com
rev.descriptiondate
1.0initial release05/04/2011
1.01updated electrical specifications07/11/2011
1.02updated electrical specifications09/16/2011
1.03updated resolution table and electrical specifications10/18/2012
1.04updated part number key11/20/2012
1.05updated spec, updated DIP switch table07/26/2013
1.06updated spec01/03/2014
1.07updated part number key04/14/2014
1.08updated spec07/18/2014
## 1.09
mounting holes increased to Ø3.96 mm through all, Ø5.79 x 1.02 mm
counter-sink
## 09/04/2015
1.10added high operating temperature range option12/18/2017
1.11updated spec10/10/2018
1.12brand update11/21/2019
1.13added motor shaft tolerance details08/04/2021
1.14logo, datasheet style update08/05/2022
1.15CUI Devices rebranded to Same Sky09/12/2024
The revision history provided is for informational purposes only and is believed to be accurate.
## REVISION HISTORY
## Additional Resources:    Product Page