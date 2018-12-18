# Zeldy

Test of ADC read speed, here we are performing a read of the time
elapsed for the read of 10 adc values. The time was obtained using the
`timer_get_counter_time_sec()` function of the esp-idf library. It would
seem to take in account the setup of the timer in order to perform the
time estimation acuratly.


```
I (426) ADC characterize: Init
I (426) ADC characterize: start
I (436) ADC characterize: read 0.000560 # time in seconds ?
```

