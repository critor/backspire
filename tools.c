void wait(int timesec) {
    volatile long int i;
    for(i=0;i<timesec*10000;i++) {}
}

void trigger_reset() {
    *(volatile unsigned*)0x900A0008 = 2;
    *(unsigned *) 0x90140020=0x80;
}