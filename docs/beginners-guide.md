# Beginner's guide for Home Assistant users

![GitHub actions](https://github.com/syssi/esphome-jbd-bms/actions/workflows/ci.yaml/badge.svg)
![GitHub stars](https://img.shields.io/github/stars/syssi/esphome-jbd-bms)
![GitHub forks](https://img.shields.io/github/forks/syssi/esphome-jbd-bms)
![GitHub watchers](https://img.shields.io/github/watchers/syssi/esphome-jbd-bms)
[!["Buy Me A Coffee"](https://img.shields.io/badge/buy%20me%20a%20coffee-donate-yellow.svg)](https://www.buymeacoffee.com/syssi)

This step by step guide describes how to setup your first ESPHome node, extend the YAML to talk to your JBD-BMS via BLE and add this node as device to your Home Assistant instance.

1) Install the "ESPHome Dashboard" using the Home Assistant Add-on store which can be found at the Supervisor panel
2) I found the easiest way to firstly install ESPHome to my ESP32 by using https://web.esphome.io
   This installation method also allows entering credentials for your Home Assistant instance as well as your Wi-Fi
4) Add your ESPHome device to the ESPHome Dashboard by adding new device
5) Once it is shown here, you can edit the configuration
6) Copy this configuration to your ESPHome device: https://github.com/syssi/esphome-jbd-bms/blob/main/esp32-ble-example.yaml
7) Edit the necessary settings within the configuration. Most important: Update the MAC address of your BMS within the substitutions. This MAC address can be found within the XiaoXiangElectric App. It has only hex numbers (and letters) and has the format [xx:xx:xx:xx:xx:xx]. Also adjust the name if necessary to match that of your ESPHome node. Wi-Fi Settings can be adjusted via Secrets file.
8) Add your Encryption Key AND / OR add credentials for your MQTT server
9) Now install this configuration on your board. There are different options for the installation which are well described. This will take a while
10) Once your board is flashed and rebooted, place the device next to your BMS (bluetooth range)
11) Now you can check if everything went fine by hitting the `Log` button of the ESPHome node and hopefully see lines scrolling containing detailed information about your battery
