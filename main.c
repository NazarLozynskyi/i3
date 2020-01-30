#include <stdlib.h>
#include "main.h"
#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t mutexInputsGroup1;
pthread_mutex_t mutexInputsGroup2;
pthread_mutex_t mutexTempSensors;

void *randomizeInputPorts(void *args)
{
    while(1)
    {
        pthread_mutex_lock(&mutexInputsGroup1);
        uint16_t randomNumber = rand()%1023;
        ioState.inputPort1 = randomNumber&(1<<0);
        ioState.inputPort2 = randomNumber&(1<<1);
        ioState.inputPort3 = randomNumber&(1<<2);
        ioState.inputPort4 = randomNumber&(1<<3);
        ioState.inputPort5 = randomNumber&(1<<4);
        ioState.inputPort6 = randomNumber&(1<<5);
        ioState.inputPort7 = randomNumber&(1<<6);
        ioState.inputPort8 = randomNumber&(1<<7);
        pthread_mutex_unlock(&mutexInputsGroup1);

        pthread_mutex_lock(&mutexInputsGroup2);
        ioState.inputPort9 = randomNumber&(1<<8);
        ioState.inputPort10 = randomNumber&(1<<9);
        pthread_mutex_unlock(&mutexInputsGroup2);
        usleep(1000000);
    }
    return NULL;
}

void *changeTempSensorsVal(void *args)
{
    while(1)
    {
        uint8_t i;
        for(i = 0; i < 255; ++i)
        {
            pthread_mutex_lock(&mutexTempSensors);
            ioState.tempSensor1 = i;
            ioState.tempSensor2 = abs(i - 255);
            pthread_mutex_unlock(&mutexTempSensors);
            usleep(50000);
        }
        for(i = 255; i > 0; --i)
        {
            pthread_mutex_lock(&mutexTempSensors);
            ioState.tempSensor1 = i;
            ioState.tempSensor2 = abs(i - 255);
            pthread_mutex_unlock(&mutexTempSensors);
            usleep(50000);
        }
    }
    return NULL;
}

void *printState(void *args)
{
    while(1)
    {
        pthread_mutex_lock(&mutexInputsGroup1);
        printf("[IN01] %d",ioState.inputPort1);
        printf("\t%u",ioState.outputPort1);
        printf(" [OUT01]\n");
        printf("[IN02] %d",ioState.inputPort2);
        printf("\t%u",ioState.outputPort2);
        printf(" [OUT02]\n\n");
        printf("[IN03] %d",ioState.inputPort3);
        printf("\t%u",ioState.outputPort3);
        printf(" [OUT03]\n");
        printf("[IN04] %d",ioState.inputPort4);
        printf("\t%u",ioState.outputPort4);
        printf(" [OUT04]\n");
        printf("[IN05] %d",ioState.inputPort5);
        printf("\t%u",ioState.outputPort5);
        printf(" [OUT05]\n");
        printf("[IN06] %d",ioState.inputPort6);
        printf("\t%u",ioState.outputPort6);
        printf(" [OUT06]\n");
        printf("[IN07] %d",ioState.inputPort7);
        printf("\t%u",ioState.outputPort7);
        printf(" [OUT07]\n");
        printf("[IN08] %d",ioState.inputPort8);
        printf("\t%u",ioState.outputPort8);
        printf(" [OUT08]\n");
        printf("\n");
        pthread_mutex_unlock(&mutexInputsGroup1);

        pthread_mutex_lock(&mutexInputsGroup2);
        pthread_mutex_lock(&mutexTempSensors);
        printf("[IN09] %d",ioState.inputPort9);
        printf("\t%u",ioState.outputPort9);
        printf(" [OUT09]\n");
        printf("[IN10] %d",ioState.inputPort10);
        printf("\t%u",ioState.outputPort10);
        printf(" [OUT10]\n\n");
        printf("[SENS1] %d",ioState.tempSensor1);
        printf("\n[SENS2] %d",ioState.tempSensor2);
        pthread_mutex_unlock(&mutexInputsGroup2);
        pthread_mutex_unlock(&mutexTempSensors);
        printf("\n\n\n\n");
        usleep(500000);
    }
    return NULL;
}

void *toggleOutput(void *port)
{
    bool *localPort = (bool*) port;
    *localPort = !*localPort;
    return NULL;
}

void *setOutput(void *port)
{
    bool *localPort = (bool*) port;
    *localPort = 1;
    return NULL;
}

void *resetOutput(void *port)
{
    bool *localPort = (bool*) port;
    *localPort = 0;
    return NULL;
}

void *controller(void *args)
{
    uint8_t tempRule1 = 0;
    uint8_t tempRule2 = 0;
    uint8_t tempRule3 = 0;
    uint8_t tempRule4 = 0;
    while(1)
    {
        //Rule 1
        pthread_mutex_lock(&mutexInputsGroup1);
        if(ioState.inputPort1 & ioState.inputPort2 & !tempRule1)
        {
            pthread_t thread_id1, thread_id2;
            pthread_create(&thread_id1, NULL, toggleOutput, &(ioState.outputPort1));
            pthread_create(&thread_id2, NULL, toggleOutput, &(ioState.outputPort2));
            tempRule1 = 1;
        }
        if(!ioState.inputPort1 & !ioState.inputPort2 & tempRule1)
        {
            pthread_t thread_id1, thread_id2;
            pthread_create(&thread_id1, NULL, toggleOutput, &(ioState.outputPort1));
            pthread_create(&thread_id2, NULL, toggleOutput, &(ioState.outputPort2));
            tempRule1 = 0;
        }

        //Rule 2
        if((ioState.inputPort3 + ioState.inputPort4 + ioState.inputPort5
            + ioState.inputPort6 + ioState.inputPort7 + ioState.inputPort8) > 3
           & !tempRule2)
        {
            pthread_t thread_id[6];
            pthread_create(&thread_id[0], NULL, setOutput, &(ioState.outputPort3));
            pthread_create(&thread_id[1], NULL, setOutput, &(ioState.outputPort4));
            pthread_create(&thread_id[2], NULL, setOutput, &(ioState.outputPort5));
            pthread_create(&thread_id[3], NULL, setOutput, &(ioState.outputPort6));
            pthread_create(&thread_id[4], NULL, setOutput, &(ioState.outputPort7));
            pthread_create(&thread_id[5], NULL, setOutput, &(ioState.outputPort8));
            tempRule2 = 1;
        }
        if((ioState.inputPort3 + ioState.inputPort4 + ioState.inputPort5
            + ioState.inputPort6 + ioState.inputPort7 + ioState.inputPort8) <= 3
           & tempRule2)
        {
            pthread_t thread_id[6];
            pthread_create(&thread_id[0], NULL, setOutput, &(ioState.outputPort3));
            pthread_create(&thread_id[1], NULL, resetOutput, &(ioState.outputPort4));
            pthread_create(&thread_id[2], NULL, setOutput, &(ioState.outputPort5));
            pthread_create(&thread_id[3], NULL, resetOutput, &(ioState.outputPort6));
            pthread_create(&thread_id[4], NULL, setOutput, &(ioState.outputPort7));
            pthread_create(&thread_id[5], NULL, resetOutput, &(ioState.outputPort8));
            tempRule2 = 0;
        }
        pthread_mutex_unlock(&mutexInputsGroup1);


        //Rule 3
        pthread_mutex_lock(&mutexTempSensors);
        pthread_mutex_lock(&mutexInputsGroup2);
        if(ioState.inputPort9 & ioState.tempSensor1 > 127 & !tempRule3)
        {
            pthread_t thread_id1;
            pthread_create(&thread_id1, NULL, setOutput, &(ioState.outputPort9));
            tempRule3 = 1;
        }
        if(ioState.tempSensor1 <= 127 & tempRule3)
        {
            pthread_t thread_id1;
            pthread_create(&thread_id1, NULL, resetOutput, &(ioState.outputPort9));
            tempRule3 = 0;
        }

        //Rule 4
        if(ioState.inputPort9 & ioState.inputPort10 & ioState.tempSensor2 > 127 & !tempRule4)
        {
            pthread_t thread_id1;
            pthread_create(&thread_id1, NULL, setOutput, &(ioState.outputPort10));
            tempRule4 = 1;
        }
        if(ioState.tempSensor2 <= 127 & tempRule4)
        {
            pthread_t thread_id1;
            pthread_create(&thread_id1, NULL, resetOutput, &(ioState.outputPort10));
            tempRule4 = 0;
        }
        pthread_mutex_unlock(&mutexTempSensors);
        pthread_mutex_unlock(&mutexInputsGroup2);
        usleep(1);
    }
    return NULL;
}

int main()
{
    pthread_mutex_init(&mutexInputsGroup1, NULL);
    pthread_mutex_init(&mutexInputsGroup2, NULL);
    pthread_mutex_init(&mutexTempSensors, NULL);

    pthread_t thread_id[4];
    pthread_create(&thread_id[0], NULL, randomizeInputPorts, NULL);
    pthread_create(&thread_id[1], NULL, changeTempSensorsVal, NULL);
    pthread_create(&thread_id[2], NULL, printState, NULL);
    pthread_create(&thread_id[3], NULL, controller, NULL);
    pthread_join(thread_id[3], NULL);
    exit(0);
}
