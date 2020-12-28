## Process
### Priority of thread

0 - 99 RT thread
Nice (-20- 19) Normal thread

Top commadn view: rt - highest, RT(-99 - -1), Noraml (0 ~ 20)
Kernel View: RT (0 - 99), Normal (100 -120)
User: 0 - 99
| USER   |  Kernel   |  top command |
| PR 50  |  99 - 50  | -1 - 50      |
| RT 99  |  99 - 99  | rt           |
| nice 5 |           | 20 + 5       |
| nice -5|           | 20 -5        |

### Pthread Scheduling:
Linux by default, uses the ntpl (Native POSIX Thread Library) implementation which considers a thread as a light-weigth process, so the scheduler schedules threads with other processes.


### ASLR
```cpp
cat /proc/sys/kernel/randomize_va_space
```
0 = Disabled. 
1 = Conservative Randomization  
2 = Full Randomization. 
