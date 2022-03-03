#include <Arduino.h>
#include <util/atomic.h>
//M1 = Sorter motor
#define M1_PIN1 7
#define M1_PIN2 8
#define M1_PWM 9 //~pwm
//M2 = Drum motor
#define M2_PIN1 10
#define M2_PWM 11 //~pwm
#define M2_PIN2 12
//OPTO-SENSORS
#define OPTO_1 A1 //LEFT OPTO
#define OPTO_2 A0 //RIGHT OPTO
//M1
#define ENCA_1 2 //~pwm
#define ENCB_1 4
#define GEAR_1 171.8
//M2
#define ENCA_2 3
#define ENCB_2 5 //~pwm
#define GEAR_2 30
//Speed Modes
//#define HSPEED 2.5 //RPM
//#define LSPEED 0.7 //RPM 0.5 to 1
//Drum mode
#define LEFT 1
#define RIGHT -1
#define SCROLL 1
#define STOPPED 0
#define DEPLOY -1
#define OPTO_FLAG 500
#define LOOP_NUMBER 33
#define PASS 100
#define D_INITIAL 195
#define D_SORTER 10.46
#define D_CABLE 3
#define SAFTEY_FACTOR_MAX 100.0
#define SAFTEY_FACTOR_MIN 3.0

struct Motor {
  int direction;
  int pwm;
  byte PIN_PWM;
  byte PIN_1;
  byte PIN_2;
  boolean type;
} Msorter, Mdrum;

unsigned long startT=0, nowT=0,prevT=0,now_safe=0,start_safe=0, startT_1=0,nowT_1=0,prevT_1=0,nowT_2=0,prevT_2=0,nowT_3=0,prevT_3=0; //For Time delays
unsigned long temp_time=0;//For sending time data
int D_iteration=D_INITIAL; // The current D in [mm]
double v_cable=0.0; //Cable speed in [M/S]
double v_sorter=0.0; // Sorter's speed in [M/S]
double time_fullpass=0.0;
unsigned int current_rev=0; //NOT IN USE
int target_rpm_1,target_rpm_2; // Target RPM
int layer_Num=5;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ The Drum starts as in DEPLOY MODE with all cable on the drum, for now 5 layers, need to CHECK
int speed=0; // User's speed decision
int rpm[2]={0},rpm_1=0,rpm_2=0;
unsigned int sampleT=10;//~~~~~~~~~~~~~~~~~~~~~~~~~ IMPORTANT: the sample time of the PID control. if too small: pulses wont count properly. if too big: bigger error and diff
double intervalT_1=0.0,intervalT_2=0.0;//Time interval for RPM calculation
int state=0, test_mode =0;
unsigned int opto_1_val=0,opto_2_val=0; //Opto-Sensor read
volatile int pulse_1=0, pulse_2=0; //Encoder read
double cable_length=0.0;
char send_data[20],data_input[20];
int bytesRead;
int temp_CheckRPM1=0, temp_CheckRPM2=0; //The temporary value of the motors RPM
boolean state_flag=1,opto_read_1=0, opto_read_2=0; //opto_read_1 & 2 = Flag for the opto-sensors so we wont read the same encounter with the sensor more then once.
boolean scroll_saftey_flag=1,deploy_saftey_flag=1; //the Saftey flag of the Saftey Func
byte prev_state=0;
unsigned int process_time=500; //process time (dt) for cable length calc

int kp=5,ki=5,kd=2,duty_sum=0,I,error=0,prev_error_sorter=0, prev_error_drum=0;
long int P,D,max_sum=0,err_sum_sorter=0,err_sum_drum=0;
char temp_string[50];
int sorter_direction=RIGHT , sorter_start=0;
String speedString; 
double hspeed=2.5, lspeed=0.7,speed_temp=0.0;

void calcRPM(); //Calculate the RPM
void sorterDirection(); //Check Sorter direction and change if needed
void readEncoder_1(); //Encouder of Sorter Motor
void readEncoder_2();//Encoder of Drum Motor
void setMotor(int direction, int pwm_val, Motor myMotor);//Set Motors speed and direction
void motorControl(boolean type,int target_rpm, int current_rpm);//PID Control
int checkRPM(Motor myMotor);//Check the current RPM
void safetyCheck();//Saftey check for cable length
void Transition();//Transitioning between modes

void setup() {
  Serial.begin(9600);
  Serial.println("Arduino Connected");
  pinMode(M1_PIN1, OUTPUT);
  pinMode(M1_PIN2, OUTPUT);
  pinMode(M2_PIN1, OUTPUT);
  pinMode(M2_PIN2, OUTPUT);
  pinMode(OPTO_1, INPUT);
  pinMode(OPTO_2, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA_1),readEncoder_1,RISING);
  attachInterrupt(digitalPinToInterrupt(ENCA_2),readEncoder_2,RISING);
  //pinMode(ENCA_1, INPUT);
  pinMode(ENCB_1, INPUT);
  //pinMode(ENCA_2, INPUT);
  pinMode(ENCB_2, INPUT);

  Msorter.PIN_1 = M1_PIN1;
  Msorter.PIN_2 = M1_PIN2;
  Msorter.PIN_PWM = M1_PWM;
  Msorter.pwm = 0;
  Msorter.type = 1;
  Msorter.direction=RIGHT; //Start direction LEFT/RIGHT
  Mdrum.PIN_1 = M2_PIN1;
  Mdrum.PIN_2 = M2_PIN2;
  Mdrum.PIN_PWM = M2_PWM;
  Mdrum.pwm = 0;
  Mdrum.type = 0;
}

void loop() {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Read Com ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
while(Serial.available()>0)
  {
    bytesRead = Serial.readBytesUntil('\0',data_input,20);
    data_input[bytesRead] = '\0';

    Serial.print(">> ");
    Serial.println(data_input);

    if(data_input[0] == 'H'){
      speedString = data_input+2;
      hspeed=speedString.toDouble();
    }
    if(data_input[0] == 'L'){
      speedString = data_input+2;
      lspeed=speedString.toDouble();
    }

    if(data_input[0] == '3' && state==0)
    {
      sscanf(data_input,"%d,%d",&state,&sorter_start);
    }
    if(data_input[0] == '*' && (state==0 || state==3))
    {
      sscanf(data_input+2,"%d",&sorter_direction);
      if(sorter_direction == 1)
        Msorter.direction=RIGHT;
      if(sorter_direction == 0)
        Msorter.direction=LEFT;
    }
    if (data_input[0] >= '0' && data_input[0] <= '2')
    {
      sscanf(data_input,"%d,%d,%d",&state,&speed,&test_mode);
    }
    if(state!=prev_state){
      state_flag=1;
      //prev_state=state; - Moved to the cases
    }
    if(speed==1)
      v_cable =lspeed; //LSPEED;
    if(speed==2)
      v_cable =hspeed; //HSPEED;
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Calculate RPM and Iterations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  calcRPM();
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Motors Settings ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  switch(state)
  {
    case 0: //Motors Off
      setMotor(STOPPED, STOPPED, Msorter);
      setMotor(STOPPED, STOPPED, Mdrum);
      Mdrum.pwm = 0;                    // NEW 05/07/2021
      Msorter.pwm = 0;                  // NEW 05/07/2021
      if(state_flag){
        Serial.println("Idle Mode");
        Transition();
        prev_state=state;
        state_flag=0;
      }
    break;
    case 1: //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~DEPLOY~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      Mdrum.direction = DEPLOY; //~~~~~~~~~ Deploy direction
      
      if(state_flag){
        Serial.println("Entered DEPLOY Mode");
        Transition();
        prev_state=state;
        state_flag=0;
      }
     if(test_mode==0)
     {
      nowT=millis();
      if(nowT - prevT >= process_time)
     {
        safetyCheck();
        if(deploy_saftey_flag==0){//SAFTEY CHECK EVERY 100ms
          Serial.println("STOPPED - END OF CABLE");
          state_flag=1;
          state=0;
          break;
        }
        prevT = nowT;
      }
     }
      sorterDirection();

      setMotor(Msorter.direction,Msorter.pwm,Msorter);
      setMotor(Mdrum.direction,Mdrum.pwm,Mdrum);

      nowT_2=millis();
      if(nowT_2 - prevT_2 >= sampleT)
      {
        temp_CheckRPM1 = checkRPM(Msorter);
        temp_CheckRPM2 = checkRPM(Mdrum);
        motorControl(Msorter.type,target_rpm_1,temp_CheckRPM1);
        motorControl(Mdrum.type,target_rpm_2,temp_CheckRPM2);
        
        prevT_2=nowT_2;
      }
      //******************************Sending info to CVI**************************************
      startT_1=nowT_1=millis();
      if(nowT_1 - prevT_1 >= 1000)
      {
        temp_time=millis()/1000;
        sprintf(send_data,"3,%d,%lu",temp_CheckRPM2,temp_time);
        Serial.println(send_data);
        sprintf(send_data,"4,%d,%lu",temp_CheckRPM1,temp_time);
        Serial.println(send_data);
        sprintf(send_data,"5,%d",(int)cable_length);
        Serial.println(send_data);
        
        //Serial.print("CHECK HSPEED: ");                    // NEW 05/07/2021
        //Serial.println(hspeed);                               // NEW 05/07/2021
        //Serial.print("CHECK LSPEED: ");                    // NEW 05/07/2021
       // Serial.println(lspeed);                              // NEW 05/07/2021

        prevT_1 = millis();
      }
    break;
    case 2: //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SCROLL~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      Mdrum.direction = SCROLL; //~~~~~~~~~ SCROLL direction
      if(state_flag){
        Serial.println("Entered SCROLL Mode");
        Transition();
        prev_state=state;
        state_flag=0;
      }
     if(test_mode==0)
     {
      nowT=millis();
      if(nowT - prevT >= process_time)
      {
        safetyCheck();
        if(scroll_saftey_flag==0){//SAFTEY CHECK EVERY 100ms
          Serial.println("STOPPED - END OF CABLE");
          state_flag=1;
          state=0;
          break;
        }
        prevT = nowT;
      }
     }
      sorterDirection();
      setMotor(Msorter.direction,Msorter.pwm,Msorter);
      setMotor(Mdrum.direction,Mdrum.pwm,Mdrum);

      nowT_2=millis();
      if(nowT_2 - prevT_2 >= sampleT)
      {
        temp_CheckRPM1 = checkRPM(Msorter);
        temp_CheckRPM2 = checkRPM(Mdrum);
        motorControl(Msorter.type,target_rpm_1,temp_CheckRPM1);
        motorControl(Mdrum.type,target_rpm_2,temp_CheckRPM2);
        
        prevT_2=nowT_2;
      }
      //******************************Sending info to CVI**************************************
      startT_1=nowT_1=millis();
      if(nowT_1 - prevT_1 >= 1000)
      {
        temp_time=millis()/1000;
        sprintf(send_data,"3,%d,%lu",temp_CheckRPM2,temp_time);
        Serial.println(send_data);
        sprintf(send_data,"4,%d,%lu",temp_CheckRPM1,temp_time);
        Serial.println(send_data);
        sprintf(send_data,"5,%d",(int)cable_length);
        Serial.println(send_data);
        
        //Serial.print("CHECK HSPEED: ");                    // NEW 05/07/2021
        //Serial.println(hspeed);                               // NEW 05/07/2021
        //Serial.print("CHECK LSPEED: ");                    // NEW 05/07/2021
        //Serial.println(lspeed);                                // NEW 05/07/2021

        prevT_1 = millis();
      }
    break;
    case 3:
      int p1=0,p2=0;
      if (sorter_start==1)
        setMotor(Msorter.direction,80,Msorter);
      
     if(analogRead(OPTO_1)>=OPTO_FLAG && p1==0){
        Msorter.direction=RIGHT;
        p1=1;
        p2=0;
      }
      if(analogRead(OPTO_2)>=OPTO_FLAG && p2==0){
        Msorter.direction=LEFT;
        p2=1;
        p1=0;
      }
     
    break;
  }
}//END OF LOOP

void sorterDirection()
{
  opto_1_val = analogRead(OPTO_1); //Left (Front view) If Deployed, starting from the right, and the first opto sensor that powered is OPTO_1
  opto_2_val = analogRead(OPTO_2); //Right (Front view)
  
  if (opto_1_val>=OPTO_FLAG && opto_2_val>=OPTO_FLAG){ //!!!!!!!!! ERROR CONDITION !!!!!!!!!
    Msorter.pwm = STOPPED;
    Msorter.direction = STOPPED;
    state=0;
  }
  if (opto_1_val>=OPTO_FLAG && opto_2_val<=OPTO_FLAG && opto_read_1==0){ //Left flag raised
    Msorter.direction = RIGHT; //Sorter goes to the right
    if(Mdrum.direction==DEPLOY){
      layer_Num--;
      if(layer_Num<=0)
        layer_Num=0;
    }
    if(Mdrum.direction==SCROLL)
      layer_Num++;
    opto_read_1 = 1;
    opto_read_2 = 0;
  }
  if (opto_2_val>=OPTO_FLAG && opto_1_val<=OPTO_FLAG && opto_read_2 == 0){ //Right flag raised
    Msorter.direction = LEFT; //Sorter goes to the left
    if(Mdrum.direction==SCROLL)
      layer_Num++;
    if(Mdrum.direction==DEPLOY){
      layer_Num--;
      if(layer_Num<=0)
        layer_Num=0;
    }
    opto_read_1 = 0;
    opto_read_2 = 1;
  }
}
void readEncoder_1()
{
  pulse_1++;
}
void readEncoder_2()
{
  pulse_2++;
}
void setMotor(int direction, int pwm_val, Motor myMotor)
{
  analogWrite(myMotor.PIN_PWM,pwm_val);
  if(direction == 1){
    digitalWrite(myMotor.PIN_1,HIGH);
    digitalWrite(myMotor.PIN_2,LOW);
  }
  else if(direction == -1){
    digitalWrite(myMotor.PIN_1,LOW);
    digitalWrite(myMotor.PIN_2,HIGH);
  }
  else{
    digitalWrite(myMotor.PIN_1,LOW);
    digitalWrite(myMotor.PIN_2,LOW);
  }
}
void motorControl(boolean type, int target_rpm, int current_rpm)
{
   error = target_rpm - current_rpm;

  if(type)//sorter - After assembly - P=3,I=3,D=0 | Try with kd >> 5
    {
      kp=3;
      ki=3;
      kd=0; //Multiplied by 1000
      max_sum = 255/(ki*sampleT*0.001);
      err_sum_sorter += error;
      if(err_sum_sorter>=max_sum)
        err_sum_sorter=max_sum;
      I=ki*err_sum_sorter*(sampleT*0.001);
      D=kd*((error-prev_error_sorter)/sampleT); //-(kd*(prev_error-error)*0.001)/(sampleT*0.001);
    }
  else //drum - After assembly - P=3,I=3,D=0 || P=1,I=3,D=0
  {
    kp=1;
    ki=3;
    kd=0; //Multiplied by 1000
    max_sum = 255/(ki*sampleT*0.001);
    err_sum_drum += error;
    if(err_sum_drum>=max_sum)
      err_sum_drum=max_sum;
    I=ki*err_sum_drum*(sampleT*0.001);
    D=(kd*(error-prev_error_drum))/sampleT; //-(kd*(prev_error-error)*0.001)/(sampleT*0.001);
  }
  P=(long int)error*kp;        
       
  duty_sum=P+I+D;
  if(duty_sum>=255)
    duty_sum=255;
  if(duty_sum<=0)
    duty_sum=0;

  if(type){
    Msorter.pwm=duty_sum;
    prev_error_sorter=error;
  }
  else {
    Mdrum.pwm=duty_sum;
    prev_error_drum=error;
  }
}
int checkRPM(Motor myMotor)
{
    if(myMotor.type==1){ //SORTER
      //if(pulse_1==0)
       // rpm[myMotor.type] = 0;
      //else{
      intervalT_1 =sampleT*(12.0/(float)(pulse_1));
      rpm_1 = (1000*(float)(60.0/intervalT_1)/GEAR_1);
      rpm[myMotor.type] = rpm_1; 
      //}   
      pulse_1 = 0;
    }
    if(myMotor.type==0){ //DRUM
      //if(pulse_2==0)
       // rpm[myMotor.type] = 0;
      //else{
      intervalT_2 =sampleT*(16.0/(float)(pulse_2));
      rpm_2 = (1000*(float)(60.0/intervalT_2)/GEAR_2);
      rpm[myMotor.type] = rpm_2;
      //}
      /*current_rev = current_rev + rpm_2/(6);/////////(rpm/6000*dt)*dt Multiplied by factor of 1000
      if(current_rev > (LOOP_NUMBER*1000)) //Multiplied by factor of 1000
      {
        if(myMotor.direction==SCROLL)
          layer_Num++;
        if(myMotor.direction==DEPLOY)
          layer_Num--;
        current_rev=0;
      }*/
      pulse_2 = 0;
    }
    
  return rpm[myMotor.type];
}
void safetyCheck()
{
    if(Mdrum.direction == DEPLOY)
    {
      cable_length = cable_length + (v_cable*process_time/1000.0);
      if(cable_length >= SAFTEY_FACTOR_MAX){
        deploy_saftey_flag = 0;
      }
      if(cable_length > SAFTEY_FACTOR_MIN)
        scroll_saftey_flag = 1; //Enables Scrolling
    }

    if(Mdrum.direction == SCROLL)
    {
      cable_length = cable_length - (v_cable*process_time/1000.0);
      if(cable_length <= SAFTEY_FACTOR_MIN){
        scroll_saftey_flag = 0;
      }
      if(cable_length < SAFTEY_FACTOR_MAX)
        deploy_saftey_flag = 1; //Enables Deploy
    }
}
void calcRPM()
{
  D_iteration = D_INITIAL+(layer_Num*D_CABLE);
  target_rpm_2 = ((v_cable*60.0*1000.0)/(PI*(double)D_iteration)); //maybe double problems
  time_fullpass = (PI*D_iteration*(1.0/1000.0)*LOOP_NUMBER)/v_cable;
  v_sorter = ((double)PASS*1.0/1000.0)/time_fullpass;
  target_rpm_1 = ((v_sorter*60.0*1000.0)/(PI*D_SORTER)); //maybe change rpm target to double?
}
void Transition()
{
  if ((state == 1 && prev_state == 2) || (state == 2 && prev_state == 1))
  {
    if(Msorter.direction==RIGHT)        // 
      Msorter.direction = LEFT;         //    Change Sorter direction when changing mode between SCROLL and DEPLOY
    else if (Msorter.direction==LEFT)   //
      Msorter.direction = RIGHT;
  }
  temp_CheckRPM1 = 0;
  temp_CheckRPM2 = 0;
  err_sum_drum = 0;               // NEW 05/07/2021
  err_sum_sorter = 0;             // NEW 05/07/2021
  opto_read_2 = 0;
  opto_read_1 = 0;
}