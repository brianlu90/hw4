#include "mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"

Ticker servo_ticker;
PwmOut pin5(D12), pin6(D13);
BufferedSerial uart(D1,D0); //tx,rx
void LineFollow(Arguments *in, Reply *out);
RPCFunction rpcLine(&LineFollow, "Line");

BBCar car(pin5, pin6, servo_ticker);

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
        }
        //printf("%s\r\n", buf);
        RPC::call(buf, outbuf);
    }
}

void LineFollow(Arguments *in, Reply *out)
{
    int x1, x2, y1, y2;
    int line_mid;

    x1 = in->getArg<int>();
    y1 = in->getArg<int>();
    x2 = in->getArg<int>();
    y2 = in->getArg<int>();
    line_mid = (x1 + x2) / 2;
    float slope = (y1 - y2) / (x1 - x2);

    //line_mid = x1;
    if (line_mid < 10) {
        car.turn_normp(40, -0.2);
    } else if (line_mid < 30) {
        car.turn_normp(40, -0.5);
    } else if (line_mid < 50) {
        car.goStraightCalib_normp(8);
    } else if (line_mid < 70) {
        car.turn_normp(40, 0.5);
    } else if (line_mid <= 80 ) {
        car.turn_normp(40, 0.2);
    }
    ThisThread::sleep_for(20ms);
    printf("x1 = %d\r\n", x1);
    return;
}