#include "main.h"
#include "qpc.h"
#include "actorA.h"
#include "bsp.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

QEvt * ev;
int cnt;
uint16_t static prev_cnt=0;
uint16_t const varx=0x0002;
uint16_t mtrx_buffA[] ={
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000

//		varx,// a doua linie // index 0
//		varx,// a 3 a linie // index 1
//		varx,// linia a 4 a // index 2
//		0x0000,/// lini a 5 a // index 3
//		varx, // lini a 6 a // index 4
//		varx,// linia 7 //index 5
//		0x0000,// linia 8 in afara// index 6
//		0x0000,// linia 9 in afara // idex 7
//		0x0000,// iinia 10 in afara // index 8
//		varx//linia 1 //index 9

};

uint16_t matrixObject[] ={
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000
	//0x0000, 0x0000
};
uint16_t matrixBird[] ={
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000
	//0x0000, 0x0000
};

uint16_t * mtrx_buff = mtrx_buffA;
uint16_t * bird_mtrx = matrixBird;
uint16_t * object_mtrx = matrixObject;
uint16_t object_speed=120;
uint16_t accelX;

int16_t static current_pitch=0;
int16_t static prev_pitch=0;
int16_t static index_linie_accelerometru=2;

/*..........................................................................*/
typedef struct {     /* the active object */
    QActive super;   /* inherit QActive */

    QTimeEvt timeEvt; /* private time event generator */
    QTimeEvt timeEvt1; /* private time event generator */

    int time_sec;
    int cnt;
} actorA_t;

static actorA_t l_actorA; /* the active object */

QActive * const AO_actorA = &l_actorA.super;

QEvt * ev;
/* hierarchical state machine ... */
static QState actorA_initial(actorA_t * const me, QEvt const * const e);
static QState actorA_S00(actorA_t * const me, QEvt const * const e);

void actorA_ctor(void) {
	actorA_t * const me = &l_actorA;
	QActive_ctor(&me->super, Q_STATE_CAST(&actorA_initial));
	QTimeEvt_ctorX(&me->timeEvt, &me->super, TIMEOUT_SIG, 0U);
	QTimeEvt_ctorX(&me->timeEvt1, &me->super, TIMEOUT_SIG1, 0U);
}

QState actorA_initial(actorA_t * const me, QEvt const * const e) {

    /* arm the time event to expire in half a second and every half second */
    QTimeEvt_armX(&me->timeEvt, BSP_TICKS_PER_SEC/2U, BSP_TICKS_PER_SEC/2U);
    QTimeEvt_armX(&me->timeEvt1, BSP_TICKS_PER_SEC/500U, BSP_TICKS_PER_SEC/500U);
    me->time_sec = 0;
    me->cnt = 32000;
    return Q_TRAN(&actorA_S00);
}


// prima pozitie ( stanga ) ar fi bitul al 4 lea
//mtrx_to_update[0][3]=1;


uint16_t get_index(uint16_t current_index)
{
	uint16_t new_index;
	switch(current_index){
			case 0:
			{
				new_index=9;
				break;
			}
			case 1:
			{
				new_index=0;
				break;
			}

			case 2: {
				new_index=1;
				break;
			}

			case 3: {
				new_index=2;
				break;
			}
			case 4:{
				new_index=3;
				break;
			}
			case 5:{
				new_index=4;
				break;
			}
			case 6:{
				new_index=5;
				break;
			}

			case 7: {
				new_index=6;
				break;
			}
			case 8:{
				new_index=7;
				break;
			}
			case 9:{
				new_index=8;
				break;
			}
		}
	return new_index;
}

uint16_t static pixel_count=0;
uint8_t static game_mode=1;
uint16_t static x = 0;

void object_matrix_hexa(uint16_t *matrix,uint16_t time)
{
	uint16_t const null_value=0x0000;
	uint16_t const current_pixel=0x0001;

	srand(time);

	pixel_count=rand()%3+2;

	uint16_t top_down=rand()%7;

	if(top_down>3)
	{
		for (uint16_t i = 0; i <10; i++)
		{
			uint16_t new_index=get_index(i);
			if(i<pixel_count)
				matrix[new_index]=current_pixel;
			else
				matrix[new_index]=null_value;

		}
	}
	else
	{
		for (uint16_t i = 6; i >=2; i--)
		{
			uint16_t new_index=get_index(i);
			if(i>6-pixel_count)
			{
				matrix[new_index]=current_pixel;
			}
			else{
				matrix[new_index]=null_value;
			}

		}
	}

}

void summ_matrix(uint16_t *obj_matrix, uint16_t* bird_matrix, uint16_t* matrix)
{
	for(uint16_t i=0;i<10;++i)
	{
		matrix[i]=obj_matrix[i]|bird_matrix[i];
	}
}

uint16_t static score=0;
bool coliziune(uint16_t*obj_matrix,uint16_t*bird_matrix)
{
	for(uint16_t i=0;i<10;++i)
	{
		if((obj_matrix[i]!=0) && (bird_matrix[i]!=0) )
		{
			if(obj_matrix[i]==bird_matrix[i])
			{
				score=0;
				return true;
			}
		}
	}
	return false;
}

// sus jos pasare
void bird_matrix_fun(uint16_t*matrix,uint16_t index, uint16_t hexa_value)
{
	uint16_t const null_value=0x0000;
	uint16_t const current_pixel=hexa_value;
	uint16_t new_index=get_index(index);
    for (uint16_t i = 0; i <10; i++)
    {
    	if(new_index==i)
    		matrix[i]=current_pixel;
    	else
    		matrix[i]=null_value;

    }
}

static uint16_t object_index=3;

void move_object(uint16_t* object_matrix,uint16_t time)
{
	if(object_index==16 )
		{
			object_matrix_hexa(object_matrix,time);
			object_index=2;
			score++;
		}
	if(object_index == 7)
	{
		object_index=11;
	}

	for(uint16_t i=0;i<10;++i)
	{
		uint16_t new_index=get_index(i);
		if(object_index == 11)
		{
			object_matrix[new_index]=object_matrix[new_index]<<3;
		}
		else
		{
			object_matrix[new_index]=object_matrix[new_index]<<1;
		}
	}
	object_index++;
}

QState actorA_S00(actorA_t * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
        	Digit_Number(me->cnt);
            status = Q_HANDLED();

            // HAL_GPIO_WritePin(MTRX_CRST_GPIO_Port, MTRX_CRST_Pin, SET);
            // HAL_GPIO_WritePin(MTRX_CRST_GPIO_Port, MTRX_CRST_Pin, RESET);
            // HAL_Delay(1);

            break;
        }
        case TIMEOUT_SIG: {
        	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        	me->time_sec++;
        	// MPU MPU-9250 addr 1101000 = 68 hex
        	//accelX = (rx_data[0] << 8) | rx_data[1];

        	uint8_t tx_data[1];
        	uint8_t rx_data[2];
        	tx_data[0] = '\x3D';

        	//HAL_I2C_Master_Transmit(&hi2c2, 0x27 << 1, tx_data_FS , 1, 10);
        	HAL_I2C_Master_Transmit(&hi2c2, 0x68 << 1, tx_data , 1, 10);
        	HAL_I2C_Master_Receive(&hi2c2, 0x68 << 1, rx_data , 1, 10);
        	current_pitch = (rx_data[0]); //acceleratia pe X

        	//Digit_Number((uint16_t)current_pitch);

            status = Q_HANDLED();
            break;
        }

        case TIMEOUT_SIG1: {
        	uint8_t rot = Rot_Read();

        	static uint8_t mtrx_cntr = 0, mtrx_idx = 0;

        	static uint8_t btn_prev, btn,first_time=1;

        	if(first_time==1)
        	{
        		bird_matrix_fun(bird_mtrx,2,0x1000);
        		object_matrix_hexa(object_mtrx,me->time_sec);
        		first_time=0;

        	}
        	btn = HAL_GPIO_ReadPin(ROTB_GPIO_Port, ROTB_Pin);

        	if (btn ^ btn_prev) {
        	    ev =  Q_NEW(QEvt, ROT_BTN_SIG);
        	    QActive_post_((QActive *)me, ev, QF_NO_MARGIN, NULL);
        	}

        	btn_prev = btn;

        	if (rot == 2) {
        	    ev =  Q_NEW(QEvt, ROT_UP_SIG);
        	    QActive_post_((QActive *)me, ev, QF_NO_MARGIN, NULL);
        	} else if (rot == 3) {
        	    ev =  Q_NEW(QEvt, ROT_DN_SIG);
        	    QActive_post_((QActive *)me, ev, QF_NO_MARGIN, NULL);
        	}

        	if(game_mode==1)
        	{
				if(prev_pitch!=current_pitch)
				{
					if(abs(prev_pitch-current_pitch)>2)
					{
						if(prev_pitch>current_pitch)
						{
							index_linie_accelerometru-=1;
							if(index_linie_accelerometru<0){
								index_linie_accelerometru=6;}

						}
						else
						{
							index_linie_accelerometru+=1;
							if(index_linie_accelerometru>6){
								index_linie_accelerometru=0;}
						}
						bird_matrix_fun(bird_mtrx,(index_linie_accelerometru),0x1000);
					}

				}

				prev_pitch=current_pitch;
        	}
       		Matrix_Update(mtrx_buff[(mtrx_idx)%10]);

        	mtrx_idx ++;
        	if (mtrx_idx == 10) {
        		mtrx_idx = 0;
        	}

        	mtrx_cntr ++;
        	if (mtrx_cntr == object_speed)
        	{

        		mtrx_cntr = 0;
        		mtrx_idx = 0;
        		move_object(object_mtrx,me->time_sec);

        		if(!coliziune(object_mtrx,bird_mtrx))
        		{
					summ_matrix(object_mtrx,bird_mtrx,mtrx_buff);
					Digit_Number(score);
        		}
				else
				{
					playMelody();
					score=0;
				}

        	}

        	status = Q_HANDLED();
            break;
        }

        case ROT_BTN_SIG: {

        	x++;
        	if (x % 2) {
        		game_mode=!game_mode;
        	}
            status = Q_HANDLED();
            break;
        }

        case ROT_UP_SIG: {

				if(game_mode==0)
				{
					me->cnt+=1;
					if(me->cnt!=prev_cnt)
					{
						bird_matrix_fun(bird_mtrx,((me->cnt-1)%7),0x1000);

					}
					prev_cnt=me->cnt;

				}else
				{
				 me->cnt=1;
				}

			status = Q_HANDLED();
			break;
        }

        case ROT_DN_SIG: {
        	if (me->cnt > 0)
        	{

        		if(game_mode==0)
        		{
					me->cnt-=1;
					if(me->cnt!=prev_cnt)
					{
						bird_matrix_fun(bird_mtrx,((me->cnt+1)%7),0x1000);

					}

					prev_cnt=me->cnt;
        		}else
        		{
        			me->cnt=2;
        		}
        	}

            status = Q_HANDLED();
            break;
        }

        default: {
            status = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status;
}