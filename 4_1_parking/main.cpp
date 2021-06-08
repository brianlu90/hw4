#include "mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"

Ticker servo_ticker;
PwmOut pin5(D12), pin6(D13);
BufferedSerial xbee(D10, D9);
BufferedSerial pc(USBTX, USBRX);
DigitalInOut ping(D6);
Timer t;
const float slope  = 0.10;

BBCar car(pin5, pin6, servo_ticker);

void goStraight_cm(float length);
void turn_90(float fc);
void park(int d1, int d2, int face);
void RPC_park (Arguments *in, Reply *out);
RPCFunction rpcPark(&RPC_park, "park");


int main(void) {
    parallax_ping  ping1(ping);
    float start, end;
    // please contruct you own calibration table with each servo
   double pwm_table0[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
   double speed_table0[] = {-15.147, -14.49, -11.712, -10.603, -4.863, 0.000, 6.059, 11.400, 14.031, 14.988, 15.368};
   double pwm_table1[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
   double speed_table1[] = {-15.625, -14.988, -14.270, -11.321, -5.501, 0.000, 5.899, 11.480, 14.350, 15.386, 15.785};

   // first and fourth argument : length of table
   car.setCalibTable(11, pwm_table0, speed_table0, 11, pwm_table1, speed_table1);

    char buf[256], outbuf[256];
    FILE *devin = fdopen(&xbee, "r");
    FILE *devout = fdopen(&xbee, "w");
    while (1) {
        memset(buf, 0, 256);
        for( int i = 0; ; i++ ) {
            char recv = fgetc(devin);
            if(recv == '\n') {
                printf("\r\n");
                break;
            }
            buf[i] = fputc(recv, devout);
        }
        RPC::call(buf, outbuf);
    }
    //park(0, 0, 3);
    //goStraight_cm(-50);
    //turn_90(0.1);
    //goStraight_cm(-10);

}

void goStraight_cm(float length) {
    if (length > 0) {
        car.goStraightCalib_normp(10);
    } else {
        car.goStraightCalib_normp(-10);
        length = -length;
    }
    int stop = length * 10 * slope + 8;
    for (int i = 0; i < stop; i++) {
        ThisThread::sleep_for(100ms);
    }
    car.stop_normp();
    ThisThread::sleep_for(200ms);
}

void turn_90(float fc) {
    car.turn_normp(-52, fc);
    if (fc > 0) {
        ThisThread::sleep_for(1s);
        ThisThread::sleep_for(800ms);
    } else {
        ThisThread::sleep_for(2s);
        //ThisThread::sleep_for(100ms);
    }
    car.stop_normp();
    ThisThread::sleep_for(100ms);
}

void park(int d1, int d2, int face)
{
    if (face == 0) {
        turn_90(0.1);
        park(d1 - 2, d2 - 10, 1);
    }
    if (face == 1) {
        goStraight_cm(-8 - d2);
        turn_90(-0.1);
        goStraight_cm(10 - d1 - 16);
    }
    if (face == 2) {
        turn_90(-0.1);
        park(d1 + 7, d2 - 10, 1);
    }
    if (face == 3) {
        goStraight_cm(d2 + 4);
        turn_90(0.1);
        goStraight_cm(10 - d1 - 16);
    }
}

void RPC_park (Arguments *in, Reply *out)   {
    int d1 = in->getArg<int>();
    int d2 = in->getArg<int>();
    int face = in->getArg<int>();
    park(d1, d2, face);
    return;
}