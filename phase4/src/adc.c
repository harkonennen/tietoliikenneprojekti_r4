#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <hal/nrf_saadc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include "adc.h"

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};

#define SAMPLE_RATE_HZ 100
#define SAMPLE_INTERVAL_MS (1000 / SAMPLE_RATE_HZ)
#define SAMPLE_COUNT 100

void sample_adc_for_one_second(struct Measurement *buffer, uint16_t direction)
{
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        buffer[i] = readADCValue(direction);
        //printk("Sample %d: x=%d, y=%d, z=%d\n", i, buffer[i].x, buffer[i].y, buffer[i].z);
        k_sleep(K_MSEC(SAMPLE_INTERVAL_MS));
    }
}


void printDebugInfo(void)
{
   printk("printing first adc_dt_spec structure contents for all channels\n");
   for(int i = 0;i<3;i++)
   {
   printk("adc_dt_spec structure, channel %d = \n",i);
   printk("Device pointer = %p\n",adc_channels[1].dev);
   printk("Channel Id = %d\n",adc_channels[i].channel_id);
   printk("Voltage reference = %d\n",adc_channels[i].vref_mv);
   printk("Resolution = %d\n",adc_channels[i].resolution);
   printk("Oversampling = %d\n",adc_channels[i].oversampling);
   printk("\n\n");
   }

}

int initializeADC(void)
{

    int err;
	
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!device_is_ready(adc_channels[i].dev)) {
			printk("ADC controller device not ready\n");
			return -1;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			printk("Could not setup channel #%d (%d)\n", i, err);
			return -1;
		}
	}

    return 0;

}

struct Measurement readADCValue(uint16_t direction)
{
	int16_t buf;
    struct Measurement m;
    m.dir = direction;
    struct adc_sequence sequence = {
	  .buffer = &buf,

	  .buffer_size = sizeof(buf),
    };

    //printk("ADC reading:\n");
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) 
    {
		int err;
        int32_t val_mv;
        // For debug use... 
		/*
        printk("- %s, channel %d: ",
		       adc_channels[i].dev->name,
		       adc_channels[i].channel_id);
        */

		(void)adc_sequence_init_dt(&adc_channels[i], &sequence);

		err = adc_read(adc_channels[i].dev, &sequence);
		if (err < 0) {
			printk("Could not read (%d)\n", err);
			continue;
		} else {
            if(i==0)
            {
                m.x = val_mv;
            }
            else if (i==1)
            {
               m.y = val_mv;
            }
            else if (i==2)
            {
                m.z = val_mv;
            }           
			//printk("%"PRId16, buf);
		}

		
		val_mv = buf;
		err = adc_raw_to_millivolts_dt(&adc_channels[i],&val_mv);
		if (err < 0) 
        {
			printk(" (value in mV not available)\n");
		} 
        else 
        {
			if(i==0)
            {
                m.x = val_mv;
            }
            else if (i==1)
            {
               m.y = val_mv;
            }
            else if (i==2)
            {
                m.z = val_mv;
            }           
            //printk(" = %"PRId32" mV\n", val_mv);
		}
	}
    return m;
}
