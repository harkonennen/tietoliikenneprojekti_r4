#ifndef ADC_H_KJJ
#define ADC_H_KJJ

struct Measurement
{
   uint16_t dir;
   uint16_t x;
   uint16_t y;
   uint16_t z;
};

void sample_adc_for_one_second(struct Measurement *buffer, uint16_t direction);
int initializeADC(void);
struct Measurement readADCValue(uint16_t direction);
void printDebugInfo(void);


#endif



