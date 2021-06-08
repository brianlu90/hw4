#include "mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"

Ticker servo_ticker;
PwmOut pin5(D12), pin6(D13);
BufferedSerial uart(D1,D0); //tx,rx
void Calibration(Arguments *in, Reply *out);
void Cali(int mag);
void goStraight_cm(float length);
RPCFunction rpcCali(&Calibration, "Cali");
DigitalInOut ping(D6);

BBCar car(pin5, pin6, servo_ticker);
parallax_ping  ping1(ping);

int main() {
    char buf[256], outbuf[256];

    double pwm_table0[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table0[] = {-15.147, -14.49, -11.712, -10.603, -4.863, 0.000, 6.059, 11.400, 14.031, 14.988, 15.368};
    double pwm_table1[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table1[] = {-15.625, -14.988, -14.270, -11.321, -5.501, 0.000, 5.899, 11.480, 14.350, 15.386, 15.785};

    // first and fourth argument : length of table
    car.setCalibTable(11, pwm_table0, speed_table0, 11, pwm_table1, speed_table1);
    uart.set_baud(9600);
    while (1) {
        memset(buf, 0, 256);
        for (int i = 0; i < 256; i++) {
            char *recv = new char[1];
            uart.read(recv, sizeof(recv));
            if (*recv == '\r' || *recv == '\n') {
                break;
            }
            buf[i] = *recv;
            delete [] recv;
        }
        //printf("%s\r\n", buf);
        RPC::call(buf, outbuf);
        car.stop();
    }
}

void Calibration(Arguments *in, Reply *out)
{
    int y_degree, c_x;

    y_degree = in->getArg<int>();
    c_x = in->getArg<int>();

    //printf("%d %d\r\n", y_degree, c_x);
    if ((10 < y_degree) && (y_degree <= 180)) {
        car.turn_normp(50, 0.1);
        ThisThread::sleep_for(800ms);
        car.goStraight_normp(50);
        ThisThread::sleep_for(1s);
        car.turn_normp(50, -0.1);
        ThisThread::sleep_for(300ms);
    } else if ((180 < y_degree) && (y_degree <= 350)) {
        car.turn_normp(50, -0.1);
        ThisThread::sleep_for(500ms);
        car.goStraight_normp(50);
        ThisThread::sleep_for(1s);
        car.turn_normp(50, 0.1);
        ThisThread::sleep_for(500ms);
    } else {
        if (c_x < 75) {
            car.turn_normp(20, -0.6);
            ThisThread::sleep_for(250ms);
        } else if (c_x > 85) {
            car.turn_normp(20, 0.5);
            ThisThread::sleep_for(250ms);
        } else {
            car.stop();
            printf("%.2f\r\n", (float)ping1);
        }
    }
    return;
}