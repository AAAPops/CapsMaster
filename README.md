#### capsmaster

Simple CLI utility that allows to switch On/Off CapsLock, NumLock, ScrollLock and LEDs to appropriate state 
on all keyboards attached to computer.

No depends on build:

```bash
$ mkdir build && cd build
$ cmake ../
$ make

# ./capsmaster --info
# /capsmaster --numlock on  --scrolllock off
```

There are two condition to successful run:  

1. Keyboard(s) is a device from `/dev/input/event*` 
2. You need super user privilege 

##### Thanks for the author of the best [log system](https://github.com/rxi/log.c)


###### The same in Russian

Простая консольная утилита позволяющая переключать клавиши CapsLock, NumLock и ScrollLock в нужное состояние.

Для работы необходимы два условия:
1. Клавиатура должна присутсвовать в системе в виде устройства `/dev/input/event*`
2. Утилита запускается с привилегиями `root`  

Основная "фишка" данной утилиты в том, что она работает напрямую с клавиатурой,
без посредничества `X сервера` или чего-либо еще.


Основная трудность при разработке была в синхронизации клавиш под `X сервером` 
при подключении к компьютеру нескольких клавиатур.

Было выяснено, что для работы этого функционала необходимо менять состояние всех трех клавиш одновременно, 
даже если состояние двух из трех клавиш менять не нужно.

Благодарности: подсистема логирования цельнотянута из проекта https://github.com/rxi/log.c. Исключительно удобная и полезная вещь!