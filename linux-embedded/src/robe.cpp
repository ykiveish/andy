/*
 * Author: Yevgeniy Kiveisha <yevgeniy.kiveisha@intel.com>
 * Copyright (c) 2014 Intel Corporation.
 */

#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <string>
#include "json/json.h"
#include <stdlib.h>     /* srand, rand */
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cmath>

#include "mraa.h"
#include "uipc.h"

#define PWM_BASE            3
#define PWM_SHOULDER    5
#define PWM_ELBOW           6
#define PWM_WHRIST      9
#define PWM_GRIPPER     4

#define BASE        0
#define SHOULDER    1
#define ELBOW       2
#define WHRIST      3
#define GRIPPER     4

#define RED_PIN 4
#define GREEN_PIN 4
#define BLUE_PIN 4

#define NO  0
#define YES 1

#define HIGH_PULSE      0
#define LOW_PULSE       1

#define ENABLE          1
#define DISABLE         0

#define PERIOD_WIDTH    19800
#define MIN_PULSE_WIDTH 600
#define MAX_PULSE_WIDTH 2200

#define COORDINATE  1
#define SERVO       2

#define SERVO_SPEED_LOW       0
#define SERVO_SPEED_MIDDLE    1
#define SERVO_SPEED_HIGH      2

#define PI 3.1415926

using namespace std;

typedef struct {
    mraa_pwm_context pwmCtx;
    int              currentAngle;
    int              srcAngle;
    int              dstAngle;
} servo_context_t;

typedef struct {
        mraa_gpio_context r;
        mraa_gpio_context g;
        mraa_gpio_context b;
} led_context_t;
/*
typedef struct {
    double x;
    double y;
    double z;
    int   p;
} coordinate_t;
*/
typedef struct {
    double x;
    double y;
    double z;
    int   closure;
    double pitch;
} coordinate_t;

typedef struct {
    double tn;
    double j1;
    double j2;
    double j3;
} arm_angles_t;

typedef struct {
    double           z_offset;
    double           l1;
    double           l2;
    double           l3;
    coordinate_t    coord;
    arm_angles_t    angles;
    arm_angles_t*   angles_ptr;
} arm_context_t;

arm_context_t robe;
servo_context_t servoCtxList[4];
led_context_t leds;
int running = NO;
pthread_t ipcThread;
pthread_t testThread;
mraa_gpio_context LED;
mraa_gpio_context gripper_ctx;

void
setAngle (servo_context_t& ctx, int angle) {
    double notches = ((double)(MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / 180);
    int16_t width = notches * (double) angle + MIN_PULSE_WIDTH;
    mraa_pwm_pulsewidth_us (ctx.pwmCtx, width);
    //printf("setAngle\n");
}

class Motor {
public:
    Motor ();
    Motor (servo_context_t& data, string name);
    void Move (int angle);
    bool Start ();
    void Stop ();

    string Name;
    servo_context_t Servo;
    bool IsMotorActive;
    bool IsAngleUpdated;
    pthread_t WorkerThread;
};

Motor::Motor (servo_context_t& data, string name) {
    Servo = data;
    Name = name;
    IsAngleUpdated = false;
    IsMotorActive = false;
}

void *
MotorExecutor (void * args) {
    Motor* self = (Motor *)args;
    int direction = 0;
    while (self->IsMotorActive) {
        if (self->IsAngleUpdated) {
            direction = (self->Servo.dstAngle - self->Servo.srcAngle > 0) ? 1:(-1);
            for (int i = 0; i < abs(self->Servo.dstAngle - self->Servo.srcAngle); i++) {
                setAngle (self->Servo, i*(direction) + self->Servo.srcAngle);
                // cout << self->Name << " " << i*(direction) + self->Servo.srcAngle << endl;
                usleep(10000);
            }
            self->IsAngleUpdated = false;
            self->Servo.currentAngle = self->Servo.dstAngle;
        } else {
            usleep (50);
        }
    }
}

void
Motor::Move (int angle) {
    Servo.dstAngle = angle;
    Servo.srcAngle = Servo.currentAngle;
    IsAngleUpdated = true;
}

bool
Motor::Start () {
    IsMotorActive = true;

    int error = pthread_create(&WorkerThread, NULL, MotorExecutor, this);
    if (error) {
        return false;
    }

    return true;
}

void
Motor::Stop () {
    IsMotorActive = false;
    pthread_cancel (WorkerThread);
}

Motor* base;
Motor* shoulder;
Motor* elbow;
Motor* whrist;

uint8_t
calculateAngles (arm_context_t& ctx) {
    double a0, a1, a2, a3, a12, aG;
    double wT, w1, w2, z1, z2, l12;

    double zT = ctx.coord.z;

    if (ctx.coord.y == 0 && ctx.coord.x < 0) {
        a0 = PI;
    } else if (ctx.coord.x == 0) {
        if (ctx.coord.y > 0) {
            a0 = PI/2;
        } else if (ctx.coord.y < 0) {
            return NO;
        } else if (ctx.coord.y == 0) {
            a0 = PI/2;
        }
    } else {
        a0 = atan(ctx.coord.y / ctx.coord.x);
    }

    wT = sqrt(ctx.coord.x*ctx.coord.x + ctx.coord.y*ctx.coord.y);

    aG = -0.7853981634;
    //aG = 0;

    w2 = wT - ctx.l3 * cos(aG);
    z2 = zT - ctx.l3 * sin(aG);

    l12 = sqrt((w2*w2) + (z2*z2));
    if (l12 > ctx.l1 + ctx.l2) {
        printf ("------------------------------> NO VALID\n");
        return NO;
    }

    /*a12=atan2(z[2],w[2]);
  a[1]=acos((sq(l[1])+sq(l12)-sq(l[2]))/(2*l[1]*l12))+a12;
  w[1]=cos(a[1])*l[1];
  z[1]=sin(a[1])*l[1];*/

    a12 = atan2 (z2, w2);
    a1 = acos( ( (ctx.l1*ctx.l1) + (l12*l12) - (ctx.l2*ctx.l2) ) / (2 * ctx.l1 * l12 ) ) + a12;

    w1 = ctx.l1 * cos(a1);
    z1 = ctx.l1 * sin(a1);

    a2 = atan2 ((z2 - z1), (w2 - w1)) - a1;
    a3 = aG - a1 - a2;



    ctx.angles.tn = a0 * 180 / PI;
    if (ctx.angles.tn < 0) {
        ctx.angles.tn = 180 - abs(ctx.angles.tn); // case of tn = -0
    }

    printf ("a1 = %f\n", a1);
    a1 = PI - a1;
    // printf ("%f\n", a1);
    ctx.angles.j1 = a1 * 180 / PI;
    // a2 = abs(a2);
    ctx.angles.j2 = abs (a2 * 180 / PI);
    /*ctx.angles.j3 = a3 * 180 / PI;
    if (ctx.angles.j3 < 0) {
        ctx.angles.j3 = 180 - abs(ctx.angles.j3);
    }*/

    //wrist test for pitch
    //ctx.angles.j3 = 90;
    ctx.angles.j3 = 180 - ctx.coord.pitch;

    return YES;
}

void softPWM (int pulse) {
    for (int i = 0; i < 100; i++) {
        mraa_gpio_write(gripper_ctx, 1);
        usleep (pulse);
        mraa_gpio_write(gripper_ctx, 0);
        usleep (MAX_PULSE_WIDTH - pulse);
    }
}

void 
gripper (int angle) {
    double notches = ((double)(MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / 180);
    int16_t width = notches * (double) angle + MIN_PULSE_WIDTH;
    softPWM (width);
}

void *
ipcHandlerThread (void * args) {
    int  client = -1;
    char msg[256];

    Json::Value  root;
    Json::Reader reader;
    bool         parsingSuccessful = false;

    int          handlerId = 0;
    coordinate_t cord = { 0.0, 0.0, 0.0, 0 };

    int servoId     = 0;
    int servoAngle  = 0;

    WiseIPC *ipcInput = NULL;
    try {
        ipcInput = new WiseIPC ("/tmp/andy/position_request");
        ipcInput->setServer ();

        while (true) {
            client = ipcInput->listenIPC ();
                        ipcInput->setBuffer ((uint8_t *)&msg);
            ipcInput->readMsg (client, sizeof (msg));

                        parsingSuccessful = reader.parse(msg, root);
            if (!parsingSuccessful) {
                                std::cout  << "Failed to parse configuration\n"
                                                   << reader.getFormattedErrorMessages();
                        } else {
                handlerId = root.get("handler", 0).asInt();
                switch (handlerId) {
                    case 1: {
                        robe.coord.x = cord.x = root.get("x", 0).asFloat();
                        robe.coord.y = cord.y = root.get("y", 0).asFloat();
                        robe.coord.z = cord.z = root.get("z", 0).asFloat();
                        robe.coord.closure = cord.closure = root.get("closure",0).asInt();
                        robe.coord.pitch = cord.pitch = root.get("pitch", 0).asFloat();
                        if (calculateAngles (robe)) {
                            /*std::cout << "ANGLE" << "(" << robe.angles.tn << " , "
                                    << robe.angles.j1 << ", " << robe.angles.j2 << ", "
                                    << robe.angles.j3 << ")" << std::endl;
                            setAngle (servoCtxList[BASE],     robe.angles.tn);
                            setAngle (servoCtxList[SHOULDER], robe.angles.j1);
                            setAngle (servoCtxList[ELBOW],    robe.angles.j2);
                            setAngle (servoCtxList[WHRIST],   robe.angles.j3);*/
                            base->Move(robe.angles.tn);
                            shoulder->Move(robe.angles.j1);
                            elbow->Move(robe.angles.j2);
                            whrist->Move(robe.angles.j3);

                            if (robe.coord.closure == 1) {
                                // mraa_gpio_write(LED, 0);
                                gripper (0);
                            } else {
                                //mraa_gpio_write(LED, 1);
                                gripper (180);
                            }

                            std::cout << "COORDINATE" << "(" << cord.x << ", "
                            << cord.y << ", " << cord.z << ", "
                            << cord.closure << ", " << cord.pitch << ")" << std::endl;
                        }
                        // std::cout << "-=*=-" << std::endl;
                    }
                    break;
                    case 2: {
                        servoId = root.get("id", 0).asInt();
                        servoAngle = root.get("angle", 0).asInt();
                        if (servoId < 5) {
                            setAngle (servoCtxList[servoId], servoAngle);
                        }
                    }
                    break;
                }
            }
                        close (client);
        }
    } catch (...) {
        std::cout << "[IPC] Default error ..." << std::endl;
    }

    delete ipcInput;
    return NULL;
}

struct TestCoordinates {
    int x;
    int y;
    int z;
    int p;
};

#define X 7
#define Y 7
#define Z 5
TestCoordinates testCase[12] = {
    {X, 0, 1, 0},
    {X, Y, 1, 0},
    {-X, Y, 1, 0},
    {-X, 0, 1, 0},

    {X, 0, 4, 0},
    {X, Y, 4, 0},
    {-X, Y, 4, 0},
    {-X, 0, 4, 0},

    {X, 0, Z, 0},
    {X, Y, Z, 0},
    {-X, Y, Z, 0},
    {-X, 0, Z, 0},

    /*{-3, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 2, 1, 0},
    {0, 3, 1, 0},

    {0, 4, 1, 0},
    {0, 5, 1, 0},
    {0, 4, 1, 0},
    {0, 5, 1, 0},*/
};

void *
testHandlerThread (void * args) {
    std::cout << "testHandlerThread ENTER" << std::endl;
    while (!running) {
        for (int i = 0; i < 12; i++) {
			
			//std::cout << "TEST" << std::endl;
			/*shoulder->Move(0);
			sleep (2);
			shoulder->Move(90);
			sleep (2);
			shoulder->Move(180);
			sleep (2);*/
			
			
			/*setAngle (servoCtxList[SHOULDER],     0);
			sleep (1);
			setAngle (servoCtxList[SHOULDER],     180);
			sleep (1);
			setAngle (servoCtxList[SHOULDER],     180);
			sleep (1);
			setAngle (servoCtxList[SHOULDER],     0);
			sleep (1);*/
			
			
			/*mraa_pwm_pulsewidth_us (servoCtxList[SHOULDER].pwmCtx, 1500);
			sleep (1);
			mraa_pwm_pulsewidth_us (servoCtxList[SHOULDER].pwmCtx, 1700);
			sleep (1);*/
			
            robe.coord.x = testCase[i].x;
            robe.coord.y = testCase[i].y;
            robe.coord.z = testCase[i].z;
            robe.coord.closure = testCase[i].p;

            if (calculateAngles (robe)) {
                std::cout << i << " ANGLE" << "(" << robe.angles.tn << ", "
                        << robe.angles.j1 << ", " << robe.angles.j2 << ", "
                        << robe.angles.j3 << ")" << std::endl;
                //setAngle (servoCtxList[BASE],     robe.angles.tn);
                //setAngle (servoCtxList[SHOULDER], robe.angles.j1);
                //setAngle (servoCtxList[ELBOW],    robe.angles.j2);
                //setAngle (servoCtxList[WHRIST],   robe.angles.j3);
                base->Move(robe.angles.tn);
                shoulder->Move(robe.angles.j1);
                elbow->Move(robe.angles.j2);
                //whrist->Move(robe.angles.j3);
            }
            std::cout << "-=*=-" << std::endl;
            sleep (5);
        }
        sleep (10);
    }
    std::cout << "testHandlerThread OUT" << std::endl;
}

int
main (int argc, char **argv) {
    mraa_result_t last_error = MRAA_SUCCESS;

    robe.z_offset   = 5;
    robe.l1     = 5.5;
    robe.l2     = 5.5;
    robe.l3     = 4;

    mraa_init();
    fprintf(stdout, "MRAA Version: %s\n", mraa_get_version());

    //LED = mraa_gpio_init (BLUE_PIN);
    //mraa_gpio_dir (LED, MRAA_GPIO_OUT);

    gripper_ctx = mraa_gpio_init (GRIPPER);
    mraa_gpio_dir (gripper_ctx, MRAA_GPIO_OUT);

    servoCtxList[BASE].pwmCtx           = mraa_pwm_init (PWM_BASE);
    servoCtxList[BASE].currentAngle     = 90;
    servoCtxList[SHOULDER].pwmCtx       = mraa_pwm_init (PWM_SHOULDER);
    servoCtxList[SHOULDER].currentAngle = 90;
    servoCtxList[ELBOW].pwmCtx          = mraa_pwm_init (PWM_ELBOW);
    servoCtxList[ELBOW].currentAngle    = 90;
    servoCtxList[WHRIST].pwmCtx         = mraa_pwm_init (PWM_WHRIST);
    servoCtxList[WHRIST].currentAngle   = 90;

    base = new Motor (servoCtxList[BASE], "BASE");
    base->Start();
    shoulder = new Motor (servoCtxList[SHOULDER], "SHOULDER");
    shoulder->Start();
    elbow = new Motor (servoCtxList[ELBOW], "ELBOW");
    elbow->Start();
    whrist = new Motor (servoCtxList[WHRIST], "WHRIST");
    whrist->Start();

    mraa_pwm_period_us (servoCtxList[BASE].pwmCtx,     PERIOD_WIDTH);
    mraa_pwm_period_us (servoCtxList[SHOULDER].pwmCtx, PERIOD_WIDTH);
    mraa_pwm_period_us (servoCtxList[ELBOW].pwmCtx,    PERIOD_WIDTH);
    mraa_pwm_period_us (servoCtxList[WHRIST].pwmCtx,   PERIOD_WIDTH);

    mraa_pwm_enable (servoCtxList[BASE].pwmCtx,     ENABLE);
    mraa_pwm_enable (servoCtxList[SHOULDER].pwmCtx, ENABLE);
    mraa_pwm_enable (servoCtxList[ELBOW].pwmCtx,    ENABLE);
    mraa_pwm_enable (servoCtxList[WHRIST].pwmCtx,   ENABLE);

    printf("Starting the listener... [SUCCESS]\n");

    setAngle (servoCtxList[BASE],      servoCtxList[BASE].currentAngle);
    setAngle (servoCtxList[SHOULDER],  servoCtxList[SHOULDER].currentAngle);
    setAngle (servoCtxList[ELBOW],     servoCtxList[ELBOW].currentAngle);
    setAngle (servoCtxList[WHRIST],    servoCtxList[WHRIST].currentAngle);

    umask(0);
    struct stat st = {0};
    if (stat ("/tmp/andy", &st) == -1) {
        mkdir ("/tmp/andy", 0777);
    }

    int error = pthread_create (&ipcThread, NULL, ipcHandlerThread, NULL);
    if (error) {
        exit(EXIT_FAILURE);
    }

    //int error = pthread_create (&testThread, NULL, testHandlerThread, NULL);
    //if (error) {
    //    exit(EXIT_FAILURE);
    //}

    while (!running) {
        usleep (10);
    }

    mraa_gpio_close (gripper_ctx);
    // mraa_gpio_close (LED);


    exit (EXIT_SUCCESS);
}
