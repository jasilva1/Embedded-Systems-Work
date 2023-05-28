// Josh Silva
// ECE 231 32-bit Lab 3
// 33435438

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/epoll.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

// configuring gpio inputs
void configure_gpio_input(int gpio_number){
    // converting gpio number from integer to string
    char gpio_num[4];
    sprintf(gpio_num, "%d", gpio_number);
    const char* GPIOExport="/sys/class/gpio/export"; 
    // exporting the GPIO to user space
    FILE* fp = fopen(GPIOExport, "w");
    fwrite(gpio_num, sizeof(char), strlen(gpio_num), fp); //segmentation error(didnt have permission to open file was unable to fix)
    fclose(fp);
    // setting gpio direction as input
    char GPIODirection[40];
    sprintf(GPIODirection, "/sys/class/gpio/gpio%d/direction", gpio_number);
    // setting GPIO as input
    fp = fopen(GPIODirection, "w");
    fwrite("in", sizeof(char), 2, fp);
    fclose(fp);
}

// configuring pin
void config_pin(char* pin_number, char* mode){
    // creates environment to execute command
    if(!vfork()){
        // execute command for pin configuration
        int ret = execl("/usr/bin/config-pin", "config-pin", pin_number, mode, NULL);
        if (ret < 0){
            printf("Failed to configure pin in %s mode.\n", mode);
            exit(-1);
        }
    }
}

// set PWM duty cycle
void set_pwm_duty_cycle(char* pwmchip, char* channel, char* duty_cycle){
    // export file path
    char PWMDutyCycle[60];
    sprintf(PWMDutyCycle, "/sys/class/pwm/%s/pwm-7:%s/duty_cycle", pwmchip, channel);
    // configure PWM device
    FILE* fp = fopen(PWMDutyCycle, "w");
    fwrite(duty_cycle, sizeof(char), strlen(duty_cycle), fp);
    fclose(fp);
}

// set PWM period
void set_pwm_period(char* pwmchip, char* channel, char* period){ 
    long duty_cycle_int, period_int;
    // before setting up the period read old duty cycle
    char PWMDutyCycle[60], duty_cycle_str[20];
    sprintf(PWMDutyCycle, "/sys/class/pwm/%s/pwm-7:%s/duty_cycle", pwmchip, channel);
    FILE* fp = fopen(PWMDutyCycle, "r");
    fscanf(fp, "%ld", &duty_cycle_int);
    fclose(fp);
    period_int = atol(period);
    
    if( duty_cycle_int >= period_int){
        duty_cycle_int = period_int/2;
        // converting long to char data type
        sprintf(duty_cycle_str, "%ld", duty_cycle_int);
        // setup dummy duty cycle
        set_pwm_duty_cycle(pwmchip, channel, duty_cycle_str);
    }
    // export file path
    char PWMPeriod[60];
    sprintf(PWMPeriod, "/sys/class/pwm/%s/pwm-7:%s/period", pwmchip, channel);
    fp = fopen(PWMPeriod, "w");
    fwrite(period, sizeof(char), strlen(period), fp);
    fclose(fp);
}

// starts a PWM
void start_pwm(char* pin_number, char* pwmchip, char* channel, char* period, char* duty_cycle){
    /* Input:
    pin_number: pin_number to generate PWM on
    pwmchip: the device folder to generate PWM
    channel: pwm device channel
    perod: pwm period
    duty_cycle: pwm duty cyle
    */
    // configure the pin in PWM mode
    config_pin(pin_number, "pwm");
    // export PWM device
    FILE* fp;
    char PWMExport[40];
    sprintf(PWMExport, "/sys/class/pwm/%s/export", pwmchip);
    fp = fopen(PWMExport, "w");
    fwrite(channel, sizeof(char), sizeof(channel), fp); 
    fclose(fp);
    // configure PWM Period
    set_pwm_period(pwmchip, channel, period);
    // configure PWM Duty Cycle
    set_pwm_duty_cycle(pwmchip, channel, duty_cycle);
    // enable PWM
    char PWMEnable[40];
    sprintf(PWMEnable, "/sys/class/pwm/%s/pwm-7:%s/enable", pwmchip, channel);
    // configure generating PWM
    fp = fopen(PWMEnable, "w");
    fwrite("1", sizeof(char), 1, fp);
    fclose(fp);
}

void stop_pwm(char* pin_number, char* pwmchip, char* channel){
    char PWMDisable[40];
    sprintf(PWMDisable, "/sys/class/pwm/%s/pwm-7:%s/enable", pwmchip, channel);
    // stop generating PWM
    FILE* fp = fopen(PWMDisable, "w");
    fwrite("0", sizeof(char), 1, fp);
    fclose(fp);
}

// button 0 
void event_handler_1(){
    // Handles Event 1
    // will be passed to event_callback
    // for event 1
    stop_pwm("P8_13", "pwmchip7", "1"); // Clearing pwm pin and channel
    start_pwm("P8_13","pwmchip7", "1", "1000000", "500000");
    printf("Button 0 has been pressed.\n");
    printf("Generating PWM of 1kHz (1 millisecond period),\n");
    //30 second delay
    sleep(30); //running for 30 seconds

    stop_pwm("P8_13", "pwmchip7", "1");
}
//button 1
void event_handler_2(){
    // Handles Event 2
    // will be passed to event_callback
    // for event 2
    stop_pwm("P8_13", "pwmchip7", "1"); // clearing pwm pin and channel
    start_pwm("P8_13", "pwmchip7", "1", "100000", "50000");
    printf("Button 1 has been pressed.\n");
    printf("Generating PWM of 10kHz (0.1 millisecond period),\n");
    //30 second delay
    sleep(30); //running for 30 seconds

    stop_pwm("P8_13", "pwmchip7", "1");
}

void event_callback(void handler()){
    /* This function gets invoked every time an Event is detected */
    handler();
}

void* event_thread(void* arg){
    int gpio_number_0 = 67; // GPIO 67 = P8_8 (button 0)
    int gpio_number_1 = 69; // GPIO 68 = P8_9 (button 1)

    configure_gpio_input(gpio_number_0); // configure gpio as input
    configure_gpio_input(gpio_number_1); // configure gpio as input

    char valuePath_0[40];
    char valuePath_1[40];

    sprintf(valuePath_0, "/sys/class/gpio/gpio%d/value", gpio_number_0);
    sprintf(valuePath_1, "/sys/class/gpio/gpio%d/value", gpio_number_1);

    while(1){
        FILE *fp_0 = fopen(valuePath_0, "r");
        FILE *fp_1 = fopen(valuePath_1, "r");

        int state[2];

        fscanf(fp_0, "%d", &(state[0]));
        fscanf(fp_1, "%d", &(state[1]));

        fclose(fp_0);
        fclose(fp_1);

        // event 1, button 0 pressed, trigger callback w/ function pointer, event handler 1
        if(state[0] == 0){
            event_callback(event_handler_1);
        } 

        // event 2, button 1 pressed, trigger callback w/ function pointer, event handler 2
        if(state[1] == 0){
            event_callback(event_handler_2);
        }
        sleep(1); //poll every second
    }
    return NULL;
}

//Celsius to Fahrenheit conversion
double Ctof(double c){
    return (c * 9.0 / 5.0) + 32.0;
}

//convert digital value to celsius ADC to celsius
double temperature(char *string){
    int value = atoi(string);
    //convert digital value to millivolts
    double millivolts = (value / 4096.0) * 1800;
    //convert millivolts to celsius
    double temperature = (millivolts - 500.0) / 10.0;

    return temperature;
}

//temperature thread, reads temp and takes timestamp every second
void* temperature_thread(void* arg){
    const char AIN_DEV[] = "/sys/bus/iio/devices/iio:device0/in_voltage0_raw";
    int fd = open(AIN_DEV, O_RDONLY);
    while (1){
        char buffer[1024];
        // Read the temperature sensor data
        int ret = read(fd, buffer, sizeof(buffer));
        if (ret != -1){
            buffer[ret] = '\0';
            double celsius = temperature(buffer);
            double fahrenheit = Ctof(celsius);
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts); //current time
            long time_in_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000; //conversion to ms
            printf("Time: %ldms Digital value: %s Celsius: %f Fahrenheit: %f\n", time_in_ms, buffer, celsius, fahrenheit);
            lseek(fd, 0, 0);
        }
        sleep(1); //poll every second, takes timestamp/temp every second
    }
    close(fd);
    return NULL;
}
// MAIN
int main(){
    configure_gpio_input(67);
    configure_gpio_input(69);
    config_pin("P8_13", "pwm");

    pthread_t eventThread;
    pthread_t tempThread;
    //creates event thread, with eventThread as the thread id
    if (pthread_create(&eventThread, NULL, event_thread, NULL)){
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    //creates temperature thread, with tempThread as the thread id
    if (pthread_create(&tempThread, NULL, temperature_thread, NULL)){
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    //wait for both threads to finish
    pthread_join(eventThread, NULL);
    pthread_join(tempThread, NULL);

    return 0;
}
