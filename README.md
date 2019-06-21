# LK mt6580 для Tele2 Midi 1.1           
![Tele2 Midi 1.1](./platform/mt6580/tele2-midi-11-chernyy.jpg)

## Описание

Моя модификация LK включает следующие изменения от оригинала:

  - Загрузочное меню открывается всегда после включения смартфона
  - Загрузочное меню реализовано на картинках, а не на тексте
  - Добавлен пункт в меню для выключения смартфона
  - Изменено управление в меню, а именно смена выбора происходит на клавиши увеличения и уменьшения громкости, а подтверждение выбора - на клавишу включения смартфона

Картинка меню: [Смотреть](./dev/logo/fwvga/fwvga_boot_menu_1.bmp "Смотреть")

## Инструкция по сборке

```sh
$ git clone https://github.com/Ultimate-1/lk_mt6580.git
$ cd lk_mt6580
$ export TOOLCHAIN_PREFIX=/android/alps/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin/arm-linux-androideabi-
$ make hexing6580_weg_l
```

## Инструкция по установке

Прошить файлы:

  - build-hexing6580_weg_l/lk.bin - в раздел lk
  - build-hexing6580_weg_l/logo.bin - в раздел logo
