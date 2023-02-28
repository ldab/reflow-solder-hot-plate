# relow-solder-hot-plate
 USB-C powered, esp32s3 based, hot plate

[![GitHub version](https://img.shields.io/github/v/release/ldab/relow-solder-hot-plate?include_prereleases)](https://github.com/ldab/relow-solder-hot-plate/releases/latest)
![Build Status](https://github.com/ldab/relow-solder-hot-plate/actions/workflows/workflow.yml/badge.svg)
[![License: WTFPL](http://www.wtfpl.net/wp-content/uploads/2012/12/wtfpl-badge-2.png)](https://github.com/ldab/relow-solder-hot-plate/blob/master/LICENSE)

[![GitHub last commit](https://img.shields.io/github/last-commit/ldab/relow-solder-hot-plate.svg?style=social)](https://github.com/ldab/relow-solder-hot-plate)

<p align="center">
  <img src="extras/relow-solder-hot-plate.png" width="30%">
</p>

### Main CPU

I used the the following board because that is what I had on my drawer:

### Reflow profile

It takes 3x temperature "profiles" as arguments, just to keep it simple

<p align="center">
  <img src="https://www.compuphase.com/electronics/solderprofile-standard.png" width="50%">
</p>

### Duty cycle and Supply current

With a duty cycle of 78% (PWM is inverted), it results in 22% of the total power and at 20V it should draw ~2.16A

<p align="center">
  <img src="extras/pwm_sim.png" width="50%">
</p>

https://tinyurl.com/2pvo2wyf
