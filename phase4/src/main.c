/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

 //L4_e2
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>
#include <dk_buttons_and_leds.h>
#include "my_lbs.h"
#include "adc.h"

//ADC
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#define USER_BUTTON_1           DK_BTN1_MSK
#define USER_BUTTON_2           DK_BTN2_MSK
#define USER_BUTTON_3           DK_BTN3_MSK
#define USER_BUTTON_4           DK_BTN4_MSK

static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(BT_LE_ADV_OPT_CONN |
	 BT_LE_ADV_OPT_USE_IDENTITY), /* Connectable advertising and use identity address */
	800, /* Min Advertising Interval 500ms (800*0.625ms) */
	801, /* Max Advertising Interval 500.625ms (801*0.625ms) */
	NULL); /* Set to NULL for undirected advertising */

LOG_MODULE_REGISTER(Lesson4_Exercise2, LOG_LEVEL_INF);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_STATUS_LED DK_LED1
#define CON_STATUS_LED DK_LED2
#define USER_LED DK_LED3
#define USER_BUTTON DK_BTN1_MSK

#define STACKSIZE 2048
#define PRIORITY 7

static volatile bool trigger_sampling = false;  // Flag to trigger one batch of samples

#define RUN_LED_BLINK_INTERVAL 1000
/* STEP 17 - Define the interval at which you want to send data at */
#define NOTIFY_INTERVAL 10000
static bool app_button_state;
static struct k_work adv_work;
static uint32_t app_sensor_direction = 0;	//Sensorin suunta


static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),

};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_LBS_VAL),
};

static void adv_work_handler(struct k_work *work)
{
	int err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
}
static void advertising_start(void)
{
	k_work_submit(&adv_work);
}
static void recycled_cb(void)
{
	printk("Connection object available from previous conn. Disconnect is complete!\n");
	advertising_start();
}

static void app_led_cb(bool led_state)
{
	dk_set_led(USER_LED, led_state);
}

static bool app_button_cb(void)
{
	return app_button_state;
}

/* STEP 18.1 - Define the thread function  */
void send_data_thread(void)
{
	while (1) {
		if (trigger_sampling) {
			/* Collect a burst of 100 samples */
			struct Measurement samples[100];

			printk("Starting to collect 100 samples...\n");
			
			for (int i = 0; i < 100; i++) {
				samples[i] = readADCValue(app_sensor_direction);
				/* small delay between ADC reads to avoid hogging CPU/ADC */
				k_sleep(K_MSEC(5));
			}

			printk("Collected 100 samples. Sending notifications...\n");

			/* Send each measurement at a controlled rate and handle back-pressure */
			for (int i = 0; i < 100; i++) {
				int ret;
				int retry = 0;

				printk("Sending sample %d: dir=%u, x=%u, y=%u, z=%u\n", i, samples[i].dir, samples[i].x, samples[i].y, samples[i].z);

				/* Try to send; retry briefly on transient errors like -EAGAIN/-ENOMEM */
				do {
					ret = my_lbs_send_sensor_notify(&samples[i]);
					if (ret == 0) {
						break;
					}

					/* If notifications are not enabled, abort sending remaining samples */
					if (ret == -EACCES) {
						printk("Notifications not enabled, aborting batch send\n");
						break;
					}

					/* For transient errors, wait a bit and retry */
					if (ret == -EAGAIN || ret == -ENOMEM) {
						retry++;
						k_sleep(K_MSEC(10 * retry)); /* exponential-ish backoff */
					} else {
						/* Non-recoverable error; log and skip this sample */
						printk("my_lbs_send_sensor_notify error: %d\n", ret);
						break;
					}
				} while (retry < 5);

				/* Small gap between notifications to avoid flooding the BLE stack */
				k_sleep(K_MSEC(20));
			}

			printk("Finished sending 100 samples. Press Button 2 to collect another batch.\n");
			
			/* Increment direction after sending all samples */
			app_sensor_direction++;
			if (app_sensor_direction >= 6) {
				app_sensor_direction = 0;
			}
			printk("Direction incremented to %u\n", app_sensor_direction);
			
			trigger_sampling = false;  // Reset the trigger
		}

		/* Sleep for a short time before checking trigger again */
		k_sleep(K_MSEC(100));
	}
}

static struct my_lbs_cb app_callbacks = {
	.led_cb = app_led_cb,
	.button_cb = app_button_cb,
};

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	if (has_changed & USER_BUTTON) {
		/*
		uint32_t user_button_state = button_state & USER_BUTTON;
		/* STEP 6 - Send indication on a button press
		my_lbs_send_button_state_indicate(user_button_state);
		app_button_state = user_button_state ? true : false;
		*/
		if (button_state & USER_BUTTON) {
		app_sensor_direction = 0;
		LOG_INF("Button 1 pressed, direction=%d", app_sensor_direction);
		}
	}

	// Handle Button 2 press to trigger one batch of samples
	if (has_changed & USER_BUTTON_2) {
		if (button_state & USER_BUTTON_2) {  // Button 2 pressed
			LOG_INF("Button pressed, 2 direction=%d", app_sensor_direction);
			if (app_sensor_direction >= 6){
				app_sensor_direction = 0;
			}
			trigger_sampling = true;  // Trigger one batch of samples
			app_sensor_direction++;
			printk("Triggered sampling of 100 measurements\n");
		}
	}
}
static void on_connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err %u)\n", err);
		return;
	}

	printk("Connected\n");

	dk_set_led_on(CON_STATUS_LED);
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);

	dk_set_led_off(CON_STATUS_LED);
}

struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
	.recycled = recycled_cb,
};

static int init_button(void)
{
	int err;

	err = dk_buttons_init(button_changed);
	if (err) {
		printk("Cannot init buttons (err: %d)\n", err);
	}

	return err;
}


void print_samples(struct Measurement *samples, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        printk("Sample %d: x = %u, y = %u, z = %u\n", i, samples[i].x, samples[i].y, samples[i].z);
    }
}


int main(void)
{
	int blink_status = 0;
	int err;

	LOG_INF("Starting Lesson 4 - Exercise 2 \n");

	err = dk_leds_init();
	if (err) {
		LOG_ERR("LEDs init failed (err %d)\n", err);
		return -1;
	}

	err = init_button();
	if (err) {
		printk("Button init failed (err %d)\n", err);
		return -1;
	}

	//ADC initialize
	if(initializeADC() != 0)
	{
	printk("ADC initialization failed!");
	return -1;
	}

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return -1;
	}
	bt_conn_cb_register(&connection_callbacks);

	err = my_lbs_init(&app_callbacks);
	if (err) {
		printk("Failed to init LBS (err:%d)\n", err);
		return -1;
	}
	LOG_INF("Bluetooth initialized\n");
	k_work_init(&adv_work, adv_work_handler);
	advertising_start();
	for (;;) {
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}

/* STEP 18.2 - Define and initialize a thread to send data periodically */
K_THREAD_DEFINE(send_data_thread_id, STACKSIZE, send_data_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
