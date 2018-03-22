#ifndef ADC_HAL_H_
#define ADC_HAL_H_

void initADC();
void startADC();

void initJoyStick();

unsigned sampleconv(unsigned v);
void getSampleJoyStick(unsigned *X, unsigned *Y);

#endif /* ADC_HAL_H_ */
