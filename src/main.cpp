/*
 * Project airgrouper
 * Description:
 * Author:
 * Date:
 */

#include <Particle.h>

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

// max number of BLE scan results supported
const size_t SCAN_RESULT_MAX = 30;
// our custom adv data length
const size_t CUSTOM_ADV_DATA_LEN = 11;

// SerialLogHandler logHandler(LOG_LEVEL_INFO);
SerialLogHandler logHandler(115200, LOG_LEVEL_INFO,
    {
      {"app", LOG_LEVEL_INFO},
      // {"gsm0710muxer", LOG_LEVEL_WARN},
    });

static SystemSleepConfiguration sleep_cfg = {};
static BleScanResult scan_results[SCAN_RESULT_MAX];

/// current limit of Particle.publish size
constexpr size_t PUBLISH_CHUNK = 622;
constexpr size_t JSON_BUF_LEN = ((PUBLISH_CHUNK + 8) / 4 ) * 4;
static char json_writer_buf[JSON_BUF_LEN] = {};

// control how long we sleep based on data collection and publication config
// static void sleep_control(uint32_t sleep_ms) {
//   sleep_cfg.mode(SystemSleepMode::ULTRA_LOW_POWER)
//   // keep BLE active
//     .ble()
// 	// Wake on battery fuel gauge event, eg battery unplugged 
//     // .gpio(LOW_BAT_UC, FALLING) 
//     .duration(sleep_ms); //ms
  
//   uint32_t sleep_start = millis();
//   Log.info("sleep %lu ms", sleep_ms);
//   SystemSleepResult sleep_res = System.sleep(sleep_cfg);
//   SystemSleepWakeupReason wake_reason = sleep_res.wakeupReason();
//   uint32_t sleep_actual = millis() - sleep_start;
//   // allow some time for usb serial to wake from sleep
//   Serial.begin();
//   delay(3000);
//   Log.info("sleep_actual: %lu", sleep_actual);

//   switch (wake_reason) {
// 	case SystemSleepWakeupReason::BY_RTC:
// 		Log.info("wakeup on RTC");
// 		break;
//     case SystemSleepWakeupReason::BY_GPIO:
//       Log.info("GPIO wakeup pin: %u", sleep_res.wakeupPin());
//       break;
// 	case SystemSleepWakeupReason::BY_NETWORK:
// 		Log.info("Network wakeup");
// 		break;

//     case SystemSleepWakeupReason::BY_ADC: 
//     default: {
//       Log.info("wakeup: %u", (uint16_t)wake_reason);
//     }
//     break;
//   }
// }


static void scan_for_beacons() {
  uint16_t MAX_SCAN_TIME = 50; // 500 ms in units of 10 ms
	BLE.setScanTimeout(MAX_SCAN_TIME);
	int raw_scan_result = BLE.scan(scan_results, SCAN_RESULT_MAX);
  Log.info("scanned: %d", raw_scan_result);

  if (raw_scan_result <= 0) {
    Log.warn("No BLE scan results: %d", raw_scan_result);
    return;
  }
  uint32_t count = (uint32_t)raw_scan_result;
  // zero the persistent json buffer before reusing
  memset((void*)json_writer_buf, 0, sizeof(json_writer_buf));
  JSONBufferWriter* json_writer = new JSONBufferWriter(json_writer_buf, PUBLISH_CHUNK);
  json_writer->beginObject();

	for (size_t beacon_idx = 0; beacon_idx < count; beacon_idx++) {
		uint8_t buf[BLE_MAX_ADV_DATA_LEN];
		size_t len;

		// When getting a specific AD Type, the length returned does not include the length or AD Type so len will be one less
		// than what we put in the beacon code, because that includes the AD Type.
    BleScanResult result = scan_results[beacon_idx];
		len = result.advertisingData.get(BleAdvertisingDataType::MANUFACTURER_SPECIFIC_DATA, buf, BLE_MAX_ADV_DATA_LEN);
		if (len == CUSTOM_ADV_DATA_LEN) {
			// We have manufacturer-specific advertising data (0xff) a

			// Byte: BLE_SIG_AD_TYPE_MANUFACTURER_SPECIFIC_DATA (0xff)
			// 16-bit: Company ID (0xffff)
			// Byte: Internal packet identifier (0x55)
			// 32-bit: custom data

      Log.warn("len: %u expected: %u", len, CUSTOM_ADV_DATA_LEN);

      // filter on company ID and internal packet identifier
			if (buf[0] == 0xff && buf[1] == 0xff && buf[2] == 0x55) {
        Log.warn("len: %u expected: %u", len, CUSTOM_ADV_DATA_LEN);
				double custom_data = 0;
				memcpy(&custom_data, &buf[3], sizeof(custom_data));

        String addr_str = String::format("%02X:%02X:%02X:%02X:%02X:%02X",
          result.address[0], result.address[1], result.address[2],
          result.address[3], result.address[4], result.address[5]);
				Log.info("device: %s custom_data: %0.3f rssi=%d ",
          addr_str.c_str(), custom_data, result.rssi);

          json_writer->name(addr_str).beginObject();
          json_writer->name("custom").value(custom_data);
          json_writer->name("rssi").value(result.rssi);
          json_writer->endObject();
      }
		}


  }
	
  json_writer->endObject();

  size_t written_size = json_writer->dataSize();
  if (written_size > 4) {
    if (written_size > PUBLISH_CHUNK) {
      Log.warn("json size excessive: %u", written_size);
      written_size = PUBLISH_CHUNK-1;
    }

    const char* pub_buf = json_writer->buffer();
    // Log.info("pre pub: %s", pub_buf);
    if (!Particle.publish("bcnz", pub_buf, PRIVATE | WITH_ACK)) {
      Log.warn("publish failed");
    }

  }

}


// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin();
  delay(3000); //wait for serial usb to init, if connected
  Log.info("=== begin ===");
  // enable BLE radio
  BLE.on();

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {

  // connect to cloud
  // publish collection
  // sleep
  if (!Particle.connected()) {
    Particle.connect();
  }
  scan_for_beacons();
  // sleep_control(3000);
  delay(3000);
}

