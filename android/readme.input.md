# Android Input Events


## About input devices

- `getevent` can dump input events and providing information about input devices.
- `sendevent` can inject input events.

**Send Event args**

``` bash
# All number in decimal
sendevent /dev/input/eventX type code value

type:
  # see Android NDK:linux/input-event-codes.h

code:
  # see Android NDK:linux/input-event-codes.h

value:
  UP     : 0
  DOWN   : 1
  REPEAT : 2
```

## Examples

``` bash
# Pressing Home
sendevent /dev/input/event0  1 102 1
sendevent /dev/input/event0  0 0 0
sendevent /dev/input/event0  1 102 0
sendevent /dev/input/event0  0 0 0

# Doble tap (unlock)
sendevent /dev/input/event4  1 187 1
sendevent /dev/input/event4  0 0 0
sendevent /dev/input/event4  1 187 0
sendevent /dev/input/event4  0 0 0

```

## More info

```
getevent -pl                                               
add device 1: /dev/input/event0
  name:     "mtk-kpd"
  events:
    KEY (0001): KEY_HOME              KEY_END               KEY_VOLUMEDOWN        KEY_VOLUMEUP         
                KEY_POWER             KEY_MENU              KEY_BACK              KEY_HP               
                KEY_CAMERA            KEY_SEND              00fa                  00fb                 
  input props:
    <none>

add device 2: /dev/input/event4
  name:     "mtk-tpd"
  events:
    KEY (0001): KEY_POWER             KEY_MENU              KEY_BACK              KEY_HOMEPAGE         
                KEY_F13               KEY_F14               KEY_F15               KEY_F16              
                KEY_F17               KEY_SEARCH            BTN_TOUCH            
    ABS (0003): ABS_X                 : value 0, min 0, max 720, fuzz 0, flat 0, resolution 720
                ABS_Y                 : value 0, min 0, max 1280, fuzz 0, flat 0, resolution 1280
                ABS_PRESSURE          : value 0, min 0, max 255, fuzz 0, flat 0, resolution 0
                ABS_MT_TOUCH_MAJOR    : value 0, min 0, max 100, fuzz 0, flat 0, resolution 0
                ABS_MT_TOUCH_MINOR    : value 0, min 0, max 100, fuzz 0, flat 0, resolution 0
                ABS_MT_POSITION_X     : value 0, min 0, max 720, fuzz 0, flat 0, resolution 0
                ABS_MT_POSITION_Y     : value 0, min 0, max 1280, fuzz 0, flat 0, resolution 0
                ABS_MT_TRACKING_ID    : value 0, min 0, max 0, fuzz 0, flat 0, resolution 0
  input props:
    INPUT_PROP_DIRECT
add device 3: /dev/input/event3
  name:     "stk_ges"
  events:
    KEY (0001): KEY_PAGEUP            KEY_PAGEDOWN          KEY_VOLUMEDOWN        KEY_VOLUMEUP         
  input props:
    <none>
add device 4: /dev/input/event2
  name:     "hwmdata"
  events:
    REL (0002): REL_Y                
  input props:
    <none>
add device 5: /dev/input/event1
  name:     "ACCDET"
  events:
    KEY (0001): KEY_VOLUMEDOWN        KEY_VOLUMEUP          KEY_HANGEUL           KEY_NEXTSONG         
                KEY_PLAYPAUSE         KEY_PREVIOUSSONG      KEY_STOPCD            KEY_SEND             
  input props:
    <none>
```


### Double Tap to unlock
```
$ getevent
/dev/input/event4: 0001 00bb 00000001
/dev/input/event4: 0000 0000 00000000
/dev/input/event4: 0001 00bb 00000000
/dev/input/event4: 0000 0000 00000000

$ getevent -l
/dev/input/event4: EV_KEY       KEY_F17              DOWN                
/dev/input/event4: EV_SYN       SYN_REPORT           00000000            
/dev/input/event4: EV_KEY       KEY_F17              UP                  
/dev/input/event4: EV_SYN       SYN_REPORT           00000000

$ cat /dev/input/event4 | xxd                                
00000000: 0b58 fc5e a61f 0200 0100 bb00 0100 0000  .X.^............
00000010: 0b58 fc5e ae1f 0200 0000 0000 0000 0000  .X.^............
00000020: 0b58 fc5e c61f 0200 0100 bb00 0000 0000  .X.^............
00000030: 0b58 fc5e c91f 0200 0000 0000 0000 0000  .X.^............
```

## Refs
- [simulate-touch-events](https://igor.mp/blog/2018/02/23/using-adb-simulate-touch-events.html)
- [emulating-touchscreen](http://ktnr74.blogspot.com/2013/06/emulating-touchscreen-interaction-with.html)
- [pawitp](https://gist.github.com/pawitp/3925413)
- [sendevent.c](https://android.googlesource.com/platform/system/core/+/android-4.4.4_r2.0.1/toolbox/sendevent.c)
- [Android NDK:linux/input-event-codes.h](https://android.googlesource.com/platform/prebuilts/ndk/+/dev/platform/sysroot/usr/include/linux/input-event-codes.h)