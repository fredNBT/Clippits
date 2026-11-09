#ifndef STEPPERMOTOR_H
#define STEPPERMOTOR_H

void setup_stepper(void);
void set_stepper_speed(int frequency);
void MakeLabel();
void NudgeBarrel(bool dir,  int time);
void NudgeClose(bool dir, int time);
void set_close_stepper_speed(int frequency);
#endif 