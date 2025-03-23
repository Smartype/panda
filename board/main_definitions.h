#include "main_declarations.h"

// ********************* Globals **********************
uint8_t hw_type = 0;
board *current_board;
uint32_t uptime_cnt = 0;
bool green_led_enabled = false;

// heartbeat state
uint32_t heartbeat_counter = 0;
bool heartbeat_lost = false;
bool heartbeat_disabled = false;            // set over USB

// siren state
bool siren_enabled = false;

// -1: missing heartbeat disabled, 0: disabled, 1: start enabled, 2+: enabled
int8_t charge_on_ignition = 1;
uint32_t started_counter = 0;
uint8_t disable_charging_after = 0;
uint32_t usb_powered_seconds = 0;
bool deepsleep_allowed = false;
bool ignition_seen = false;
