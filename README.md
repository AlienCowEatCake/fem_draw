# FEM Draw

## Описание

**Назначение проекта:** рисовалка скалярных и векторных полей.

**Язык программирования:** C++

**Фреймворк:** Qt4 / Qt5 или WinAPI

**Лицензия:** [GNU GPL v3](http://www.gnu.org/copyleft/gpl.html)

Программа предназначена для рисования скалярных и векторных полей. На данный момент поддерживаются только двумерные графики. Описание форматов данных приведено ниже.

Есть две версии программы - первая (обозначим ее Q) использует Qt, вторая (пусть будет W) использует WinAPI. У обеих версий есть свои достоинства и недостатки:

* Q - кроссплатформенная, W - только под Windows;
* W компактная, Q довольно большая (статически слинкованный исполняемый файл занимает несколько мегабайт);
* W умеет сохранять изображения только в форматах png, bmp, jpg, gif и tga, Q работает с тем, с чем умеет работать конкретная версия Qt;
* В Q изолинии и линии веторов рисуются более плавно и красиво, чем в W;
* Q более-менее умеет Unicode, W он только снился.

В остальном же отличия неущественны.

## Формат входных данных
На вход программе подаются данные в формате Tecplot. Допустимыми являются ASCII-данные типа Two-dimensional Ordered Data. Значения должны быть заданы в узлах (nodal data). Поддержка Finite Element Data и cell-centered data планируется в отдаленном будущем.

Подробные примеры можно найти в документации к Tecplot или в каталоге [examples](examples/) проекта. Краткий пример:
```
TITLE = "Example"
VARIABLES = "X", "Y", "Val1", "Val2"
ZONE I=3, J=3, F=POINT
0 0  100 100
0 1   90 100
0 2   80 100
1 0   85  80
1 1   70  85
1 2   90  85
2 0   95  70
2 1   75  75
2 2   70  80
```

## Компиляция
Для компиляции версии с Qt требуется Qt версии 4.4.3 и выше. С более ранними версиями работоспособность возможна, но не проверялась. Внешних зависимостей нет. Компиляция производится стандартным способом (`qmake fem_draw_qt.pro`, затем `make`). Шаблоны сборочных скриптов под Windows можно найти в каталоге [buildscripts](buildscripts/).

Для компиляции версии с WinAPI требуется компилятор C++ (поддерживаются MSVC6 и выше, MinGW 3.4 и выше, OpenWatcom 1.9). Внешних зависимостей (кроме заголовочных файлов WinAPI) нет. Можно также воспользоваться шаблонами скриптов из [buildscripts](buildscripts/) или же использовать проект [fem_draw_winapi.pro](fem_draw_winapi.pro). Если есть необходимость использовать Visual Studio в качестве IDE, можно создать проект в ней, добавив в него необходимые файлы (осторожно, нужно будет либо не добавлять [манифест](src_winapi/manifest.manifest) из каталога с исходными кодами, либо отключить генерацию манифеста в свойствах проекта).

