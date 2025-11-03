# pragma once

static const uint8_t JBD_CMD_READ = 0xA5;
static const uint8_t JBD_CMD_WRITE = 0x5A;

static const uint8_t JBD_CMD_HWINFO = 0x03;
static const uint8_t JBD_CMD_CELLINFO = 0x04;
static const uint8_t JBD_CMD_HWVER = 0x05;

static const uint8_t JBD_CMD_ENTER_FACTORY = 0x00;
static const uint8_t JBD_CMD_EXIT_FACTORY = 0x01;
static const uint8_t JBD_CMD_FORCE_SOC_RESET = 0x0A;
static const uint8_t JBD_CMD_ERROR_COUNTS = 0xAA;
static const uint8_t JBD_CMD_CAP_REM = 0xE0;   // Set remaining capacity
static const uint8_t JBD_CMD_MOS = 0xE1;       // Set charging/discharging bitmask
static const uint8_t JBD_CMD_BALANCER = 0xE2;  // Enable/disable balancer

