#ifndef ADXL345_H_
#define ADXL345_H_

#include "stdint.h"

typedef enum
{
	accl_2g=0,
	accl_4g=1,
	accl_8g=2,
	accl_16g=3
}adxl345Parameters;

typedef struct
{

	float ax;
	float ay;
	float az;

}accleration_values_t;

void adxl345_init(adxl345Parameters param);
void adxl345_update();
void adxl345_get_values(accleration_values_t * values);






#endif /* ADXL345_H_ */
