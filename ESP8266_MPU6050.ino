#define BLYNK_PRINT Serial


#include <Wire.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

long accelX, accelY, accelZ;   //Variables for raw register data
float gForceX, gForceY, gForceZ;  // Variables for gForce
float fpsX, fpsY, fpsZ; // Variables for feet per second

long gyroX, gyroY, gyroZ;  //Variables for raw gyroscope data
float rotX, rotY, rotZ;    // Variables for rotational acceleration in degrees / second

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "820feedb369a45278512a3a1100efb38";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "You Mirin My Gainz???";
char pass[] = "6317378931";

BlynkTimer timer;
int pinValue;
float myTimer = 0.00;        //Declares a Timer to track velocities
float velocityZ = 0.00;      //Sets up variable for calculating velocity
// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.
void sendData()
{
  recordAccelRegisters();     //Obtains values in acceleration registers
  //recordGyroRegisters();       //Obtains values in gyroscope registers 
  //printFps();
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  if(pinValue){
     //filterAccel();
     myTimer +=0.20;
     //Serial.print(myTimer);
     Serial.println(gForceY);
     Blynk.virtualWrite(V1, myTimer);
     Blynk.virtualWrite(V5, fpsZ);
     Blynk.virtualWrite(V6, fpsY);
     Blynk.virtualWrite(V7, fpsX);
  }
  else {
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V2, 0);
    Blynk.virtualWrite(V3, 0);
    myTimer = 0.00;
    delay(1000);
  }
}


  BLYNK_WRITE(V0)
  {
    pinValue = param.asInt();
    
  }

//Setup Functions
void setupMPU(){
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission();  
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4) 
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s 
  Wire.endTransmission(); 
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
  Wire.write(0b00000000); //Setting the accel to +/- 2g
  Wire.endTransmission(); 
}

void recordAccelRegisters() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Accel Registers (3B - 40)
  while(Wire.available() < 6);
  accelX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  accelY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processAccelData();
}
void recordGyroRegisters() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Gyro Registers (43 - 48)
  while(Wire.available() < 6);
  gyroX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processGyroData();                  //Next Function
}

void filterAccel(){                     //Filters all acceleration values for stabalization
  if(fpsZ < 0){
        fpsZ = fpsZ * 2;
        fpsZ = ceil(fpsZ);
        fpsZ = fpsZ / 2;
}
     else if(fpsZ > 0){
        fpsZ = fpsZ * 2;
        fpsZ = floor(fpsZ);
        fpsZ = fpsZ / 2;
     }

     
     if(fpsY < 0){
        fpsY = fpsY * 2;
        fpsY = ceil(fpsY);
        fpsY = fpsY / 2;
}
     else if(fpsY > 0){
        fpsY = fpsY * 2;
        fpsY = floor(fpsY);
        fpsY = fpsY / 2;
     }

     
     if(fpsX < 0){
        fpsX = fpsX * 2;
        fpsX = ceil(fpsX);
        fpsX = fpsX / 2;
}
     else if(fpsX > 0){
        fpsX = fpsX * 2;
        fpsX = floor(fpsX);
        fpsX = fpsX / 2;
     }
     
}


//Processes Gyroscope Data
void processGyroData() {
  rotX = gyroX / 131.0 + 2.5;      //Values in degrees per second and additional values offset calibration
  rotY = gyroY / 131.0 - 1.25;     
  rotZ = gyroZ / 131.0 + .1;
}



void processAccelData(){              //Numbers being divided to obtain G-force numbers being subtracted calibration offset
  gForceX = accelX / 16384.0 -.035;
  gForceY = accelY / 16384.0; 
  gForceZ = accelZ / 16384.0 - .045;
                                      //G-force multiplied by 32.217 to obtain FPS Values
  fpsX = gForceX * 32.217 - 1.0;          
  fpsY = gForceY * 32.217 - 0.5;            
  fpsZ = (gForceZ - 1) * 32.217 + 0.75;
  
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8442);
  Wire.begin();  
  setupMPU();     //Sets up our accelometer
  
  timer.setInterval(200L, sendData);
}
void printGyro(){
  Serial.print(" Gyro (Degrees/s)");
  Serial.print(" X=");
  Serial.print(rotX);
  Serial.print(" Y=");
  Serial.print(rotY);
  Serial.print(" Z=");
  Serial.println(rotZ);
}
void printFps(){
  Serial.print(" Accel (fps)");
  Serial.print(" X=");
  Serial.print(fpsX);
  Serial.print(" Y=");
  Serial.print(fpsY);
  Serial.print(" Z=");
  Serial.println(fpsZ);
}

void loop()
{
  Blynk.run();
  timer.run(); // Initiates BlynkTimer
}

