#include <stdlib.h>
#include "util.h"
#include "main.h"
#include "stm32f4xx_hal.h"

static void Digit_Update(uint16_t data);

static const uint8_t digits[] = {
		0x7E, 0x50, 0x6D, 0x79, 0x53, 0x3B, 0x3F, 0x70, 0x7F, 0x7B
};

static uint16_t digit_state = 0x00;

uint32_t timer_key2;
uint32_t timer_key3;

unsigned int timer_enc = 0;

void Digit_Init(void) {
	Digit_Update(digit_state);
}

void Digit_Number(uint16_t number) {
	digit_state = ((uint16_t)digits[(number/10)%10]<<8) | digits[number % 10];

	Digit_Update(digit_state);
}

void playNote(char note, int duration) {
    char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'h'};
    int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1073 };

    //tonul corespunzator notei
    for (int i = 0; i < 9; i++) {
        if (names[i] == note) {
        	uint32_t compareValue = HAL_RCC_GetPCLK1Freq() / (tones[i] * 2); // valoare de comparatie
        	//Seteaza valoarea pentru a reda tonul
        	__HAL_TIM_SET_AUTORELOAD(&htim4, compareValue);
        	Buzz_On();
        	HAL_Delay(duration);
        	Buzz_Off();
        	HAL_Delay(50); // Pauza intre note

        }
    }
}

void playMelody() {
    char notes[] = "fefeca ha";
    int beats[] = { 3, 1, 3, 1, 1, 1, 1, 2, 1 };
    int tempo = 120;

    for (int i = 0; i < sizeof(notes); i++) {
        if (notes[i] == ' ') {
            HAL_Delay(beats[i] * tempo); // Pauza
        } else {
            playNote(notes[i], beats[i] * tempo);
        }
    }
}



void Matrix_Update(uint16_t data) {
	uint32_t  j = 16;

	HAL_GPIO_WritePin(MTRX_CCLK_GPIO_Port, MTRX_CCLK_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MTRX_CCLK_GPIO_Port, MTRX_CCLK_Pin, GPIO_PIN_SET);

	// negative reset pulse
	HAL_GPIO_WritePin(MTRX_LRST_GPIO_Port, MTRX_LRST_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MTRX_LRST_GPIO_Port, MTRX_LRST_Pin, GPIO_PIN_SET);

	while (j--) { // while(data) -- BAD!!!!

		  // set data bit
		  if (data & 0x8000) {
			  HAL_GPIO_WritePin(SDI_GPIO_Port, SDI_Pin, GPIO_PIN_SET);
		  } else {
			  HAL_GPIO_WritePin(SDI_GPIO_Port, SDI_Pin, GPIO_PIN_RESET);
		  }

		  // clock pulse
		  HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_SET);

		  HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET);

		  data <<= 1;
	}

	// positive update (write) pulse
	HAL_GPIO_WritePin(MTRX_CS_GPIO_Port, MTRX_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MTRX_CS_GPIO_Port, MTRX_CS_Pin, GPIO_PIN_RESET);


}

void Buzz_On(void) {
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
}

void Buzz_Off(void) {
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
}

void Buzz_SetFreq(uint32_t freq) {
	TIM4->ARR = freq;
	TIM4->CCR3 = freq;
}

uint8_t Rot_Read(void) {

	uint8_t ret = 0;

	timer_enc++;

	static uint8_t key2 = 1, key3 = 1, key2_prev = 1, key3_prev = 1;

	key2 = HAL_GPIO_ReadPin(K2_GPIO_Port, K2_Pin);
	key3 = HAL_GPIO_ReadPin(K3_GPIO_Port, K3_Pin);

	if(key2 ^ key2_prev) {
		timer_key2 = timer_enc;
	} else if(key3 ^ key3_prev) {
		timer_key3 = timer_enc;
	}

	key2_prev = key2;
	key3_prev = key3;

	if (timer_key2 != 0 && abs(timer_key3 - timer_key2) < 11) {
		// e.sig = (timer_key3 > timer_key2) + ROT_UP_SIG;

		if (timer_key3 > timer_key2) {
			// e.sig = ROT_UP_SIG;
			ret = 2;
		} else {
			// e.sig = ROT_DN_SIG;
			ret = 3;
		}

		timer_key2 = 0;
		timer_key3 = 0;
	}

	return ret;

}

void Digit_NumberPos(uint8_t number, uint8_t pos) {
	switch(pos) {
		case 0:
			digit_state &= 0xFF00;
			digit_state |= digits[number%10];
			break;
		case 1:
			digit_state &= 0x00FF;
			digit_state |= (((uint16_t)digits[number%10])<<8);
			break;
	}

	Digit_Update(digit_state);
}

void Digit_SymolPos(uint8_t value, uint8_t pos) {
	switch(pos) {
		case 0:
			digit_state &= 0xFF00;
			digit_state |= value;
			break;
		case 1:
			digit_state &= 0x00FF;
			digit_state |= ((uint16_t)value)<<8;
			break;
	}
	Digit_Update(digit_state);
}

static void Digit_Update(uint16_t data) {
	uint32_t  j = 16;

	// negative reset pulse
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);

	while (j--) { // while(data) -- BAD!!!!

		  // set data bit
		  if (data & 0x1) {
			  HAL_GPIO_WritePin(SDI_GPIO_Port, SDI_Pin, GPIO_PIN_SET);
		  } else {
			  HAL_GPIO_WritePin(SDI_GPIO_Port, SDI_Pin, GPIO_PIN_RESET);
		  }

		  // clock pulse
		  HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_SET);

		  HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET);

		  data >>= 1;
	}

	// positive update (write) pulse
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}